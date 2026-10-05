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

#ifndef PORTALSCREENSHOT_H
#define PORTALSCREENSHOT_H

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QVariantMap>

// PortalScreenshot
//
// Used for screen-grabbing under Wayland, using XDG Desktop Portal
// (org.freedesktop.portal.Screenshot). Under Wayland,
// QScreen::grabWindow() cannot read the compositor-protected screen
// content.
//
class PortalScreenshot : public QObject {
    Q_OBJECT

  public:
    explicit PortalScreenshot(QObject *parent = nullptr);

    // when grab is called, a permission dialog is shown. Under KDE,
    // this occurs only once when the user choses to remember the
    // permission.  Under GNOME, remembering is not possible, and
    // permission is therefore asked each time.
    //
    // grab starts an async screenshot request and emits finished() or
    // failed()
    void grab(bool interactive = false);

  signals:
    void finished(const QPixmap &pixmap);
    void failed(const QString &reason);

  private slots:
    void slot_response(uint code, const QVariantMap &results);

  private:
    QString _request_path;
};

#endif  // PORTALSCREENSHOT_H
