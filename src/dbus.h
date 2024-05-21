/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
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

#ifndef _DBUS_H
#define _DBUS_H

#include <qstringconverter.h>
#include <qtmetamacros.h>

#include <QObject>
#include <iostream>

class QRectF;

class Message : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.pascalniklaus.grasp")

  public:
    Message(QObject* parent);
    ~Message();

    void set_verbose(bool v) { _verbose = v; }
    bool verbose() const { return _verbose; }
    void log(const std::string& msg) const;

    static QString service_name() { return "org.pascalniklaus.grasp"; }
    static QString service_path() { return "/exec"; }

  signals:
    void quit_received();

    void toggle_received();
    void show_received();
    void hide_received();

    void grab_received();
    void grabnew_received();

    void color_received(int index);
    void arrow_received(const QString&);
    void tool_received(const QString&);
    void width_received(int);

    void rect_received(const QRectF&, int);

  public Q_SLOTS:
    Q_SCRIPTABLE void quit();

    Q_SCRIPTABLE void toggle();
    Q_SCRIPTABLE void show();
    Q_SCRIPTABLE void hide();

    Q_SCRIPTABLE void grab();
    Q_SCRIPTABLE void grabnew();

    Q_SCRIPTABLE void tool(const QString& tool);
    Q_SCRIPTABLE void arrow(const QString& arrow);
    Q_SCRIPTABLE void color(int index);
    Q_SCRIPTABLE void width(int width);

    Q_SCRIPTABLE void rect(int x0, int y0, int x1, int y1, int r = 0);

  private:
    bool _verbose = false;
};

#endif
