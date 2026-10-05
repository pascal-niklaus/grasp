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

#include "globalshortcutportal.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QList>
#include <QRandomGenerator>
#include <QSettings>
#include <QVariantMap>

namespace {
const char *PORTAL_SERVICE = "org.freedesktop.portal.Desktop";
const char *PORTAL_PATH = "/org/freedesktop/portal/desktop";
const char *GLOBAL_SHORTCUTS_IFACE = "org.freedesktop.portal.GlobalShortcuts";
const char *SESSION_IFACE = "org.freedesktop.portal.Session";
const char *PORTAL_REQUEST_IFACE = "org.freedesktop.portal.Request";

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

// One entry of BindShortcuts' `a(sa{sv})` argument: a shortcut id paired
// with its property dict (just "description" for our use). Marshaled
// via the documented Qt D-Bus custom-type pattern (struct + registered
// streaming operators) rather than building the QDBusArgument container
// by hand.
struct PortalShortcut {
    QString id;
    QVariantMap properties;
};

QDBusArgument &operator<<(QDBusArgument &arg, const PortalShortcut &s) {
    arg.beginStructure();
    arg << s.id << s.properties;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, PortalShortcut &s) {
    arg.beginStructure();
    arg >> s.id >> s.properties;
    arg.endStructure();
    return arg;
}

}  // namespace

Q_DECLARE_METATYPE(PortalShortcut)

GlobalShortcutPortal::GlobalShortcutPortal(QSettings &settings, QObject *parent) :
    QObject(parent), _settings(settings) {
    static bool metatype_registered = false;
    if (!metatype_registered) {
        qDBusRegisterMetaType<PortalShortcut>();
        qDBusRegisterMetaType<QList<PortalShortcut>>();
        metatype_registered = true;
    }

    QDBusConnection::sessionBus().connect(
        PORTAL_SERVICE, PORTAL_PATH, GLOBAL_SHORTCUTS_IFACE, "Activated", this,
        SLOT(slot_activated(QDBusObjectPath, QString, qulonglong, QVariantMap)));
}

GlobalShortcutPortal::~GlobalShortcutPortal() {
    if (!_session_handle.isEmpty()) {
        QDBusMessage msg = QDBusMessage::createMethodCall(
            PORTAL_SERVICE, _session_handle, SESSION_IFACE, "Close");
        QDBusConnection::sessionBus().asyncCall(msg);  // fire-and-forget, app is exiting
    }
}

void GlobalShortcutPortal::bind(const QList<ShortcutDescriptor> &shortcuts) {
    if (_bind_in_progress || _bound) return;
    _bind_in_progress = true;
    _shortcuts = shortcuts;
    create_session();
}

void GlobalShortcutPortal::send_request(const QDBusMessage &msg) {
    auto *watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *w) {
                w->deleteLater();
                QDBusPendingReply<QDBusObjectPath> reply = *w;
                if (reply.isError()) {
                    QDBusConnection::sessionBus().disconnect(
                        PORTAL_SERVICE, _request_path, PORTAL_REQUEST_IFACE,
                        "Response", this, SLOT(slot_portal_response(uint, QVariantMap)));
                    fail(reply.error().message());
                    return;
                }
                if (reply.value().path() != _request_path) {
                    qWarning() << "GlobalShortcutPortal: unexpected request path"
                               << reply.value().path() << "!=" << _request_path;
                }
            });
}

void GlobalShortcutPortal::create_session() {
    _stage = Stage::CreatingSession;

    const QString handle_token = new_token();
    _request_path = request_path_for(handle_token);

    QDBusConnection::sessionBus().connect(
        PORTAL_SERVICE, _request_path, PORTAL_REQUEST_IFACE, "Response", this,
        SLOT(slot_portal_response(uint, QVariantMap)));

    QVariantMap options;
    options["handle_token"] = handle_token;
    options["session_handle_token"] = new_token();

    QDBusMessage msg = QDBusMessage::createMethodCall(
        PORTAL_SERVICE, PORTAL_PATH, GLOBAL_SHORTCUTS_IFACE, "CreateSession");
    msg << options;
    send_request(msg);
}

void GlobalShortcutPortal::bind_shortcuts() {
    _stage = Stage::BindingShortcuts;

    const QString handle_token = new_token();
    _request_path = request_path_for(handle_token);

    QDBusConnection::sessionBus().connect(
        PORTAL_SERVICE, _request_path, PORTAL_REQUEST_IFACE, "Response", this,
        SLOT(slot_portal_response(uint, QVariantMap)));

    QList<PortalShortcut> shortcuts;
    for (const auto &s : _shortcuts) {
        QVariantMap props;
        props["description"] = s.description;
        shortcuts.append({s.id, props});
    }

    QVariantMap options;
    options["handle_token"] = handle_token;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        PORTAL_SERVICE, PORTAL_PATH, GLOBAL_SHORTCUTS_IFACE, "BindShortcuts");
    msg << QVariant::fromValue(QDBusObjectPath(_session_handle))
        << QVariant::fromValue(shortcuts) << QString()  // parent_window: none
        << options;
    send_request(msg);
}

void GlobalShortcutPortal::slot_portal_response(uint code, const QVariantMap &results) {
    QDBusConnection::sessionBus().disconnect(
        PORTAL_SERVICE, _request_path, PORTAL_REQUEST_IFACE, "Response", this,
        SLOT(slot_portal_response(uint, QVariantMap)));

    if (code != 0) {
        // 1 = cancelled by user, 2 = other error. Unlike
        // PipewireScreenshot's restore_token, there is nothing here to
        // clear and retry with -- just report the failure.
        fail(QStringLiteral("global shortcuts portal request did not succeed "
                             "(stage %1, response code %2)")
                 .arg(static_cast<int>(_stage))
                 .arg(code));
        return;
    }

    switch (_stage) {
        case Stage::CreatingSession:
            _session_handle = results.value("session_handle").toString();
            if (_session_handle.isEmpty()) {
                fail(QStringLiteral(
                    "global shortcuts portal: CreateSession returned no session_handle"));
                return;
            }
            bind_shortcuts();
            break;

        case Stage::BindingShortcuts: {
            _bound = true;
            _bind_in_progress = false;
            const auto bound = qdbus_cast<QList<PortalShortcut>>(results.value("shortcuts"));
            for (const auto &s : bound) {
                qInfo() << "GlobalShortcutPortal: bound" << s.id << "->"
                        << s.properties.value("trigger_description").toString();
            }
            break;
        }

        case Stage::None:
            break;
    }
}

void GlobalShortcutPortal::slot_activated(const QDBusObjectPath &session_handle,
                                           const QString &shortcut_id,
                                           qulonglong /*timestamp*/,
                                           const QVariantMap & /*options*/) {
    if (session_handle.path() != _session_handle) return;
    emit activated(shortcut_id);
}

void GlobalShortcutPortal::fail(const QString &reason) {
    _bind_in_progress = false;
    qWarning() << "GlobalShortcutPortal:" << reason;
    emit bind_failed(reason);
}
