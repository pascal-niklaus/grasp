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

#include "dbus.h"

#include <QDBusConnection>
#include <QRectF>

#include "string_format.h"

Message::Message(QObject* parent) : QObject(parent) {
    QDBusConnection::sessionBus().registerService(service_name());
    QDBusConnection::sessionBus().registerObject(
        service_path(), this, QDBusConnection::ExportScriptableContents);
}

Message::~Message() {}

void Message::quit() {
    emit quit_received();
}

void Message::log(const std::string& msg) const {
    std::cerr << "DBus:" << msg << "\n";
}

// ---------------------------- show/hide ----------------------------

void Message::show() {
    log("'show' received");
    emit show_received();
}

void Message::hide() {
    log("'hide' received");
    emit hide_received();
}

void Message::toggle() {
    log("'toggle' received");
    emit toggle_received();
}

// ---------------------------- grabbing -----------------------------

void Message::grab() {
    log("'grab' received");
    emit grab_received();
}

void Message::grabnew() {
    log("'grabnew' received");
    emit grabnew_received();
}

// ---------------------------- parameter ----------------------------

void Message::color(int index) {
    // human color index starts at 1, internally we start at 0
    log("'color' received: index=" + std::to_string(index));
    emit color_received(index - 1);
}

void Message::arrow(const QString& arrow) {
    log("'arrow' received: " + arrow.toStdString());
    emit arrow_received(arrow);
}

void Message::tool(const QString& tool) {
    log("'tool' received: " + tool.toStdString());
    emit tool_received(tool);
}

void Message::width(int w) {
    log("'width' received: " + std::to_string(w));
    emit width_received(w);
}

// --------------------------- primitives ----------------------------

void Message::rect(int x0, int y0, int x1, int y1, int r) {
    log(string_format("'rect' received: (%d,%d) - (%d,%d)", x0, y0, x1, y1));
    emit rect_received(QRectF(QPointF(x0, y0), QPointF(x1, y1)), r);
}
