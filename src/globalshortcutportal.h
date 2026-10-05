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

#ifndef GLOBALSHORTCUTPORTAL_H
#define GLOBALSHORTCUTPORTAL_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

class QSettings;
class QDBusMessage;
class QDBusObjectPath;

// (id, description) pair to register via bind(). id is a string the
// caller gets back from activated(); description is shown to the user
// in the shortcut-assignment UI.
struct ShortcutDescriptor {
    QString id;
    QString description;
};

// GlobalShortcutPortal
//
// Registers a set of named actions as compositor-owned global keyboard
// shortcuts via org.freedesktop.portal.GlobalShortcuts.
class GlobalShortcutPortal : public QObject {
    Q_OBJECT

  public:
    explicit GlobalShortcutPortal(QSettings &settings, QObject *parent = nullptr);
    ~GlobalShortcutPortal() override;

    void bind(const QList<ShortcutDescriptor> &shortcuts);

  signals:
    void activated(const QString &shortcut_id);
    void bind_failed(const QString &reason);

  private slots:
    void slot_portal_response(uint code, const QVariantMap &results);
    void slot_activated(const QDBusObjectPath &session_handle,
                         const QString &shortcut_id,
                         qulonglong timestamp,
                         const QVariantMap &options);

  private:
    enum class Stage { None, CreatingSession, BindingShortcuts };

    void create_session();
    void bind_shortcuts();
    void send_request(const QDBusMessage &msg);
    void fail(const QString &reason);

  private:
    QSettings &_settings;
    Stage _stage = Stage::None;
    QString _request_path;
    QString _session_handle;
    QList<ShortcutDescriptor> _shortcuts;
    bool _bind_in_progress = false;
    bool _bound = false;
};

#endif  // GLOBALSHORTCUTPORTAL_H
