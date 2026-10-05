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

#ifndef PIPEWIRESCREENSHOT_H
#define PIPEWIRESCREENSHOT_H

#include <QImage>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QVariantMap>

class QSettings;
class QDBusMessage;
class QTimer;

struct pw_thread_loop;
struct pw_context;
struct pw_core;
struct pw_stream;
struct spa_hook;

// PipewireScreenshot
//
// grab a full-screen screenshot through XDG Desktop Portal's
// ScreenCast interface (org.freedesktop.portal.ScreenCast) + PipeWire.
//
// PipewireScreenshot exists in addition to PortalScreenshot because
// the latter has no persistent-grant mechanism on GNOME. ScreenCast
// supports persist_mode + restore_token: the first grab shows the
// compositor's source picker, and the resulting restore_token, stored
// in QSettings, lets later grabs skip the picker.
//
// Frames are delivered by a PipeWire stream running on its own
// pw_thread_loop; the one frame needed is copied and marshalled back
// onto the Qt thread before the stream/session are torn down and
// finished()/failed() is emitted.
//
class PipewireScreenshot : public QObject {
    Q_OBJECT

  public:
    explicit PipewireScreenshot(QSettings &settings, QObject *parent = nullptr);
    ~PipewireScreenshot() override;

    // starts async screenshot request and emits finished() or
    // failed().  If force_reselect==true, a saved restore_token is
    // ignored and the compositor's source picker shown
    void grab(bool force_reselect = false);

  signals:
    void finished(const QPixmap &pixmap);
    void failed(const QString &reason);

  private slots:
    // response handler for the three chained ScreenCast portal
    // requests (CreateSession / SelectSources / Start)
    void slot_portal_response(uint code, const QVariantMap &results);

    // queued from PipeWire thread when frame is availableonce a frame
    // has been captured and copied.
    void slot_frame_ready(const QImage &image);

    // queued from PipeWire thread on a stream error.
    void slot_stream_error(const QString &reason);

  private:
    enum class Stage { None, CreatingSession, SelectingSources, Starting };

    void create_session();
    void select_sources();
    void start_session();
    // create async call whose reply is a Request object path; wires up
    // the Response subscription on _request_path and reports failures.
    // Shared by create_session()/select_sources()/start_session().
    void send_request(const QDBusMessage &msg);
    void open_pipewire_remote();
    void start_stream(int pipewire_fd, quint32 node_id);
    void fail(const QString &reason);
    void teardown();  // tears down the pipewire stream and the portal session

    // returns the pw_stream_events table used by start_stream().
    static const struct pw_stream_events &stream_events();

    // called from PipeWire thread stream callbacks. Static because it
    // needs access to _format/_pw_stream
    static void on_param_changed(void *data,
                                 uint32_t id,
                                 const struct spa_pod *param);
    static void on_process(void *data);

  private:
    QSettings& _settings;
    Stage _stage = Stage::None;
    QString _request_path;
    QString _session_handle;
    QString _restore_token;
    bool _retried_without_token = false;
    quint32 _pending_node_id = 0;
    // guards to make sure finished() / failed() is only emitted once
    // per grab()
    bool _done = false;

    // pipewire objects, owned by _pw_loop's thread
    pw_thread_loop *_pw_loop = nullptr;
    pw_context *_pw_context = nullptr;
    pw_core *_pw_core = nullptr;
    pw_stream *_pw_stream = nullptr;
    spa_hook *_stream_listener = nullptr;
    // timer to make sure the grab completes even if the compositor
    // never delivers a frame
    QTimer *_frame_timeout = nullptr;

    struct {
        int width = 0;
        int height = 0;
        QImage::Format qt_format = QImage::Format_Invalid;
    } _format;
};

#endif  // PIPEWIRESCREENSHOT_H
