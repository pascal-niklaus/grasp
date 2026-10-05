/*
 * Grasp -- a program to GRAb the screen and Sketch on top during
 * Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "pipewirescreenshot.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QMetaObject>
#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>

#include <unistd.h>

#include <pipewire/pipewire.h>
#include <spa/buffer/buffer.h>
#include <spa/param/video/format-utils.h>
#include <spa/utils/hook.h>
#include <spa/utils/result.h>

namespace {
const char *PORTAL_SERVICE = "org.freedesktop.portal.Desktop";
const char *PORTAL_PATH = "/org/freedesktop/portal/desktop";
const char *SCREENCAST_IFACE = "org.freedesktop.portal.ScreenCast";
const char *PORTAL_REQUEST_IFACE = "org.freedesktop.portal.Request";

const char *SETTINGS_RESTORE_TOKEN_KEY = "Wayland/screencast_restore_token";

// org.freedesktop.portal.ScreenCast SourceType bitmask
constexpr quint32 SOURCE_TYPE_MONITOR = 1;

// org.freedesktop.portal.ScreenCast CursorMode bitmask
constexpr quint32 CURSOR_MODE_EMBEDDED = 2;

// org.freedesktop.portal.ScreenCast PersistMode
constexpr quint32 PERSIST_MODE_UNTIL_REVOKED = 2;

QString new_token() {
    return QStringLiteral("grasp%1").arg(
        QRandomGenerator::global()->generate(), 0, 16);
}

// Request object paths are predictable: derived from our own unique bus
// name and a token we choose ourselves (see
// https://flatpak.github.io/xdg-desktop-portal/docs/requests-and-sessions.html).
QString request_path_for(const QString &handle_token) {
    QString sender = QDBusConnection::sessionBus().baseService();
    sender.remove(0, 1);       // drop leading ':'
    sender.replace('.', '_');  // "1.42" -> "1_42"
    return QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
        .arg(sender, handle_token);
}

// free function with signature matching
// pw_stream_events::state_changed
//
// Delivering error goes through same string-based
// QMetaObject::invokeMethod() dispatch because a direct member call
// would cross the thread boundary
void on_state_changed(void *data,
                      enum pw_stream_state /*old_state*/,
                      enum pw_stream_state state,
                      const char *error) {
    if (state != PW_STREAM_STATE_ERROR) return;
    const QString msg = QStringLiteral("pipewire stream error: %1")
                            .arg(error ? error : "(unknown)");
    QMetaObject::invokeMethod(static_cast<QObject *>(data),
                              "slot_stream_error",
                              Qt::QueuedConnection,
                              Q_ARG(QString, msg));
}

}  // namespace

const pw_stream_events &PipewireScreenshot::stream_events() {
    static const pw_stream_events events = [] {
        pw_stream_events ev{};
        ev.version = PW_VERSION_STREAM_EVENTS;
        ev.state_changed = on_state_changed;
        ev.param_changed = on_param_changed;
        ev.process = on_process;
        return ev;
    }();
    return events;
}

PipewireScreenshot::PipewireScreenshot(QSettings &settings, QObject *parent) :
    QObject(parent), _settings(settings) {
    // pw_init() is safe to call multiple times, but only one
    // PipewireScreenshot needs to be constructed. There is no need to
    // call pw_deinit().
    static bool pw_initialized = false;
    if (!pw_initialized) {
        pw_init(nullptr, nullptr);
        pw_initialized = true;
    }
}

PipewireScreenshot::~PipewireScreenshot() {
  teardown();
}

void PipewireScreenshot::grab(bool force_reselect) {
    _done = false;
    _retried_without_token = force_reselect;
    _restore_token = force_reselect
                         ? QString()
                         : _settings.value(SETTINGS_RESTORE_TOKEN_KEY).toString();
    create_session();
}

void PipewireScreenshot::send_request(const QDBusMessage &msg) {
    auto *watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(msg), this);
    connect(watcher,
            &QDBusPendingCallWatcher::finished,
            this,
            [this](QDBusPendingCallWatcher *w) {
                w->deleteLater();
                QDBusPendingReply<QDBusObjectPath> reply = *w;
                if (reply.isError()) {
                    QDBusConnection::sessionBus().disconnect(
                        PORTAL_SERVICE,
                        _request_path,
                        PORTAL_REQUEST_IFACE,
                        "Response",
                        this,
                        SLOT(slot_portal_response(uint, QVariantMap)));
                    fail(reply.error().message());
                    return;
                }
                if (reply.value().path() != _request_path) {
                    qWarning() << "PipewireScreenshot: unexpected request path"
                               << reply.value().path() << "!=" << _request_path;
                }
            });
}

void PipewireScreenshot::create_session() {
    _stage = Stage::CreatingSession;

    const QString handle_token = new_token();
    _request_path = request_path_for(handle_token);

    QDBusConnection::sessionBus().connect(
        PORTAL_SERVICE,
        _request_path,
        PORTAL_REQUEST_IFACE,
        "Response",
        this,
        SLOT(slot_portal_response(uint, QVariantMap)));

    QVariantMap options;
    options["handle_token"] = handle_token;
    options["session_handle_token"] = new_token();

    QDBusMessage msg = QDBusMessage::createMethodCall(
        PORTAL_SERVICE, PORTAL_PATH, SCREENCAST_IFACE, "CreateSession");
    msg << options;
    send_request(msg);
}

void PipewireScreenshot::select_sources() {
    _stage = Stage::SelectingSources;

    const QString handle_token = new_token();
    _request_path = request_path_for(handle_token);

    QDBusConnection::sessionBus().connect(
        PORTAL_SERVICE,
        _request_path,
        PORTAL_REQUEST_IFACE,
        "Response",
        this,
        SLOT(slot_portal_response(uint, QVariantMap)));

    QVariantMap options;
    options["handle_token"] = handle_token;
    options["types"] = SOURCE_TYPE_MONITOR;
    options["multiple"] = false;
    options["cursor_mode"] = CURSOR_MODE_EMBEDDED;
    options["persist_mode"] = PERSIST_MODE_UNTIL_REVOKED;
    if (!_restore_token.isEmpty()) options["restore_token"] = _restore_token;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        PORTAL_SERVICE, PORTAL_PATH, SCREENCAST_IFACE, "SelectSources");
    msg << QVariant::fromValue(QDBusObjectPath(_session_handle)) << options;
    send_request(msg);
}

void PipewireScreenshot::start_session() {
    _stage = Stage::Starting;

    const QString handle_token = new_token();
    _request_path = request_path_for(handle_token);

    QDBusConnection::sessionBus().connect(
        PORTAL_SERVICE,
        _request_path,
        PORTAL_REQUEST_IFACE,
        "Response",
        this,
        SLOT(slot_portal_response(uint, QVariantMap)));

    QVariantMap options;
    options["handle_token"] = handle_token;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        PORTAL_SERVICE, PORTAL_PATH, SCREENCAST_IFACE, "Start");
    msg << QVariant::fromValue(QDBusObjectPath(_session_handle)) << QString()
        << options;
    send_request(msg);
}

void PipewireScreenshot::slot_portal_response(uint code,
                                              const QVariantMap &results) {
    QDBusConnection::sessionBus().disconnect(
        PORTAL_SERVICE,
        _request_path,
        PORTAL_REQUEST_IFACE,
        "Response",
        this,
        SLOT(slot_portal_response(uint, QVariantMap)));

    if (code != 0) {
        // 1 = cancelled by user, 2 = other error.
        if (!_restore_token.isEmpty() && !_retried_without_token) {
            _settings.remove(SETTINGS_RESTORE_TOKEN_KEY);
            _restore_token.clear();
            _retried_without_token = true;
            create_session();
            return;
        }
        fail(QStringLiteral("screencast portal request did not succeed "
                            "(stage %1, response code %2)")
                 .arg(static_cast<int>(_stage))
                 .arg(code));
        return;
    }

    switch (_stage) {
        case Stage::CreatingSession:
            _session_handle = results.value("session_handle").toString();
            if (_session_handle.isEmpty()) {
                fail(
                    QStringLiteral("screencast portal: CreateSession returned "
                                   "no session_handle"));
                return;
            }
            select_sources();
            break;

        case Stage::SelectingSources:
            start_session();
            break;

        case Stage::Starting: {
            if (results.contains("restore_token"))
                _settings.setValue(SETTINGS_RESTORE_TOKEN_KEY,
                                   results.value("restore_token").toString());

            const QDBusArgument streams_arg =
                results.value("streams").value<QDBusArgument>();
            quint32 node_id = 0;
            bool have_node = false;
            streams_arg.beginArray();
            while (!streams_arg.atEnd()) {
                streams_arg.beginStructure();
                quint32 id;
                QVariantMap props;
                streams_arg >> id >> props;
                streams_arg.endStructure();
                if (!have_node) {
                    node_id = id;
                    have_node = true;
                }
            }
            streams_arg.endArray();

            if (!have_node) {
                fail(QStringLiteral(
                    "screencast portal: Start returned no streams"));
                return;
            }
            _pending_node_id = node_id;
            open_pipewire_remote();
            break;
        }

        case Stage::None:
            break;
    }
}

void PipewireScreenshot::open_pipewire_remote() {
    QDBusMessage msg = QDBusMessage::createMethodCall(
        PORTAL_SERVICE, PORTAL_PATH, SCREENCAST_IFACE, "OpenPipeWireRemote");
    msg << QVariant::fromValue(QDBusObjectPath(_session_handle))
        << QVariantMap();

    auto *watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(msg), this);
    connect(
        watcher,
        &QDBusPendingCallWatcher::finished,
        this,
        [this](QDBusPendingCallWatcher *w) {
            w->deleteLater();
            QDBusPendingReply<QDBusUnixFileDescriptor> reply = *w;
            if (reply.isError()) {
                fail(QStringLiteral("OpenPipeWireRemote failed: %1")
                         .arg(reply.error().message()));
                return;
            }
            // pw_context_connect_fd() takes ownership of the fd and
            // closes it itself; dup() so the QDBusUnixFileDescriptor
            // closing its own copy on scope exit doesn't race it.
            int fd = ::dup(reply.value().fileDescriptor());
            if (fd < 0) {
                fail(QStringLiteral("could not dup() the PipeWire remote fd"));
                return;
            }
            start_stream(fd, _pending_node_id);
        });
}

void PipewireScreenshot::start_stream(int pipewire_fd, quint32 node_id) {
    _pw_loop = pw_thread_loop_new("grasp-screencast", nullptr);
    if (!_pw_loop) {
        fail(QStringLiteral("pw_thread_loop_new() failed"));
        return;
    }

    pw_thread_loop_lock(_pw_loop);

    _pw_context = pw_context_new(pw_thread_loop_get_loop(_pw_loop), nullptr, 0);
    if (!_pw_context) {
        pw_thread_loop_unlock(_pw_loop);
        fail(QStringLiteral("pw_context_new() failed"));
        return;
    }

    _pw_core = pw_context_connect_fd(_pw_context, pipewire_fd, nullptr, 0);
    if (!_pw_core) {
        pw_thread_loop_unlock(_pw_loop);
        fail(QStringLiteral("pw_context_connect_fd() failed"));
        return;
    }

    _pw_stream = pw_stream_new(_pw_core, "grasp-capture", nullptr);
    if (!_pw_stream) {
        pw_thread_loop_unlock(_pw_loop);
        fail(QStringLiteral("pw_stream_new() failed"));
        return;
    }

    _stream_listener = new spa_hook{};
    pw_stream_add_listener(
        _pw_stream, _stream_listener, &stream_events(), this);

    uint8_t buf[1024];
    spa_pod_builder b = SPA_POD_BUILDER_INIT(buf, sizeof(buf));
    // Deliberately no SPA_FORMAT_VIDEO_modifier property: omitting it
    // steers most compositors toward plain memfd/shm buffers instead of
    // DMA-BUF, so on_process() below can read pixels with a straight
    // mmap and this file never needs to bring in EGL/GBM. Needs
    // verification on a real GNOME session -- if the compositor insists
    // on DMA-BUF regardless, buffers won't be CPU-mappable here and a
    // GL/EGL import-and-readback path would need to be added.
    //
    // SPA_RECTANGLE()/SPA_FRACTION() expand to compound-literal-style
    // temporaries; C is fine taking their address inline, C++ isn't, so
    // they need named locals here.
    spa_rectangle default_size = SPA_RECTANGLE(1920, 1080);
    spa_rectangle min_size = SPA_RECTANGLE(1, 1);
    spa_rectangle max_size = SPA_RECTANGLE(8192, 8192);
    spa_fraction default_framerate = SPA_FRACTION(0, 1);
    spa_fraction min_framerate = SPA_FRACTION(0, 1);
    spa_fraction max_framerate = SPA_FRACTION(1000, 1);

    const spa_pod *params[1];
    params[0] = static_cast<const spa_pod *>(spa_pod_builder_add_object(
        &b,
        SPA_TYPE_OBJECT_Format,
        SPA_PARAM_EnumFormat,
        SPA_FORMAT_mediaType,
        SPA_POD_Id(SPA_MEDIA_TYPE_video),
        SPA_FORMAT_mediaSubtype,
        SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
        SPA_FORMAT_VIDEO_format,
        SPA_POD_CHOICE_ENUM_Id(3,
                               SPA_VIDEO_FORMAT_BGRx,
                               SPA_VIDEO_FORMAT_BGRx,
                               SPA_VIDEO_FORMAT_RGBx),
        SPA_FORMAT_VIDEO_size,
        SPA_POD_CHOICE_RANGE_Rectangle(&default_size, &min_size, &max_size),
        SPA_FORMAT_VIDEO_framerate,
        SPA_POD_CHOICE_RANGE_Fraction(
            &default_framerate, &min_framerate, &max_framerate)));

    pw_stream_connect(_pw_stream,
                      PW_DIRECTION_INPUT,
                      node_id,
                      static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
                                                   PW_STREAM_FLAG_MAP_BUFFERS),
                      params,
                      1);

    pw_thread_loop_unlock(_pw_loop);
    pw_thread_loop_start(_pw_loop);

    _frame_timeout = new QTimer(this);
    _frame_timeout->setSingleShot(true);
    connect(_frame_timeout, &QTimer::timeout, this, [this]() {
        fail(QStringLiteral(
            "timed out waiting for a frame from the compositor"));
    });
    _frame_timeout->start(8000);
}

void PipewireScreenshot::slot_stream_error(const QString &reason) {
    fail(reason);
}

void PipewireScreenshot::on_param_changed(void *data,
                                          uint32_t id,
                                          const struct spa_pod *param) {
    auto *self = static_cast<PipewireScreenshot *>(data);
    if (!param || id != SPA_PARAM_Format) return;

    spa_video_info_raw info{};
    if (spa_format_video_raw_parse(param, &info) < 0) return;

    self->_format.width = static_cast<int>(info.size.width);
    self->_format.height = static_cast<int>(info.size.height);
    switch (info.format) {
        case SPA_VIDEO_FORMAT_BGRx:
            self->_format.qt_format = QImage::Format_RGB32;
            break;
        case SPA_VIDEO_FORMAT_RGBx:
            self->_format.qt_format = QImage::Format_RGBX8888;
            break;
        default:
            self->_format.qt_format = QImage::Format_Invalid;
            break;
    }
}

void PipewireScreenshot::on_process(void *data) {
    auto *self = static_cast<PipewireScreenshot *>(data);

    pw_buffer *b = pw_stream_dequeue_buffer(self->_pw_stream);
    if (!b) return;

    const spa_data &d = b->buffer->datas[0];

    // Format is negotiated once (on_param_changed, before the first
    // buffer) and applies to every buffer of this stream instance, so
    // if it's unusable here it will stay unusable -- fail immediately
    // via the same queued/string dispatch as on_state_changed rather
    // than silently skipping buffers forever (grab() would otherwise
    // hang with neither finished() nor failed() ever emitted). This is
    // also why it must go through invokeMethod() rather than calling
    // self->fail() directly: fail() -> teardown() stops _pw_loop, and
    // stopping a pw_thread_loop from a callback running on that same
    // loop's thread is not safe.
    if (self->_format.qt_format == QImage::Format_Invalid) {
        pw_stream_queue_buffer(self->_pw_stream, b);
        QMetaObject::invokeMethod(
            self,
            "slot_stream_error",
            Qt::QueuedConnection,
            Q_ARG(QString,
                  QStringLiteral(
                      "compositor negotiated an unsupported pixel format")));
        return;
    }
    if (d.type == SPA_DATA_DmaBuf || !d.data) {
        pw_stream_queue_buffer(self->_pw_stream, b);
        QMetaObject::invokeMethod(
            self,
            "slot_stream_error",
            Qt::QueuedConnection,
            Q_ARG(QString,
                  QStringLiteral(
                      "compositor offered a DMA-BUF/unmapped buffer; "
                      "DMA-BUF (EGL/GBM) readback is not implemented")));
        return;
    }

    QImage frame(static_cast<uchar *>(d.data),
                 self->_format.width,
                 self->_format.height,
                 d.chunk->stride,
                 self->_format.qt_format);
    // Deep copy: the mapped buffer memory becomes invalid the moment we
    // queue it back below, and this callback runs on the PipeWire
    // thread, not the Qt one.
    QImage copy = frame.copy();
    QMetaObject::invokeMethod(
        self, "slot_frame_ready", Qt::QueuedConnection, Q_ARG(QImage, copy));

    pw_stream_queue_buffer(self->_pw_stream, b);
}

void PipewireScreenshot::slot_frame_ready(const QImage &image) {
    if (_done) return;
    if (image.isNull()) {
        fail(QStringLiteral("captured an empty/unsupported-format frame"));
        return;
    }
    _done = true;
    QPixmap pixmap = QPixmap::fromImage(image);
    teardown();
    emit finished(pixmap);
}

void PipewireScreenshot::fail(const QString &reason) {
    if (_done) return;
    _done = true;
    teardown();
    emit failed(reason);
}

void PipewireScreenshot::teardown() {
    if (_frame_timeout) {
        _frame_timeout->stop();
        _frame_timeout->deleteLater();
        _frame_timeout = nullptr;
    }

    if (_pw_loop) {
        pw_thread_loop_lock(_pw_loop);
        if (_pw_stream) {
            pw_stream_destroy(_pw_stream);
            _pw_stream = nullptr;
        }
        if (_pw_core) {
            pw_core_disconnect(_pw_core);
            _pw_core = nullptr;
        }
        pw_thread_loop_unlock(_pw_loop);

        pw_thread_loop_stop(_pw_loop);
        if (_pw_context) {
            pw_context_destroy(_pw_context);
            _pw_context = nullptr;
        }
        pw_thread_loop_destroy(_pw_loop);
        _pw_loop = nullptr;
    }
    delete _stream_listener;
    _stream_listener = nullptr;

    if (!_session_handle.isEmpty()) {
        QDBusMessage msg =
            QDBusMessage::createMethodCall(PORTAL_SERVICE,
                                           _session_handle,
                                           "org.freedesktop.portal.Session",
                                           "Close");
        QDBusConnection::sessionBus().asyncCall(msg);
        _session_handle.clear();
    }

    _stage = Stage::None;
}
