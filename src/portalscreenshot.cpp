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

#include "portalscreenshot.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QRandomGenerator>
#include <QUrl>

namespace {
const char *PORTAL_SERVICE = "org.freedesktop.portal.Desktop";
const char *PORTAL_PATH = "/org/freedesktop/portal/desktop";
const char *PORTAL_SCREENSHOT_IFACE = "org.freedesktop.portal.Screenshot";
const char *PORTAL_REQUEST_IFACE = "org.freedesktop.portal.Request";
}  // namespace

PortalScreenshot::PortalScreenshot(QObject *parent) : QObject(parent) {}

// PortalScreenshot::grab
//
// See:
// https://flatpak.github.io/xdg-desktop-portal/docs/requests-and-sessions.html
//
void PortalScreenshot::grab(bool interactive) {
    // request object paths are derived from own unique bus name and a
    // chosen token
    const QString token =
        QStringLiteral("grasp%1")
            .arg(QRandomGenerator::global()->generate(), 0, 16);

    QString sender = QDBusConnection::sessionBus().baseService();
    sender.remove(0, 1);        // drop leading ':'
    sender.replace('.', '_');   // "1.42" -> "1_42"

    _request_path = QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
                         .arg(sender, token);

    // subscribe to Response signal before issuing call, so we can't
    // race a portal implementation that answers very quickly.
    QDBusConnection::sessionBus().connect(
        PORTAL_SERVICE, _request_path, PORTAL_REQUEST_IFACE, "Response", this,
        SLOT(slot_response(uint, QVariantMap)));

    QDBusMessage msg = QDBusMessage::createMethodCall(
        PORTAL_SERVICE, PORTAL_PATH, PORTAL_SCREENSHOT_IFACE, "Screenshot");

    QVariantMap options;
    options["handle_token"] = token;
    options["interactive"] = interactive;
    msg << QString()  // parent_window: none
        << options;

    auto *watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *w) {
                w->deleteLater();
                QDBusPendingReply<QDBusObjectPath> reply = *w;
                if (reply.isError()) {
                    QDBusConnection::sessionBus().disconnect(
                        PORTAL_SERVICE, _request_path, PORTAL_REQUEST_IFACE,
                        "Response", this, SLOT(slot_response(uint, QVariantMap)));
                    emit failed(reply.error().message());
                    return;
                }
                // object path handed back must equal request
                if (reply.value().path() != _request_path) {
                    qWarning() << "PortalScreenshot: unexpected request path"
                               << reply.value().path() << "!=" << _request_path;
                }
            });
}

void PortalScreenshot::slot_response(uint code, const QVariantMap &results) {
    // disconnect slot since only used once
    QDBusConnection::sessionBus().disconnect(
        PORTAL_SERVICE,
        _request_path,
        PORTAL_REQUEST_IFACE,
        "Response",
        this,
        SLOT(slot_response(uint, QVariantMap)));

    if (code != 0) {
        // 1 = cancelled by user, 2 = other error
        emit failed(QStringLiteral("screenshot portal request did not succeed "
                                   "(response code %1)")
                        .arg(code));
        return;
    }

    const QUrl uri(results.value("uri").toString());
    const QString path = uri.isLocalFile() ? uri.toLocalFile() : uri.toString();

    QImage img(path);
    if (img.isNull()) {
        emit failed(QStringLiteral("could not load screenshot from '%1'").arg(path));
        return;
    }
    // delete temporary file
    QFile::remove(path);

    emit finished(QPixmap::fromImage(img));
}
