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

#include "screenres.h"

#include <QScreen>
#include <QDebug>
#include <QGuiApplication>

ScreenChangeDetector::ScreenChangeDetector(QObject *parent) :
    QObject(parent) {
    setup();
    adjust();
}

void ScreenChangeDetector::setup() {

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        connect(screen,
                &QScreen::geometryChanged,
                this,
                &ScreenChangeDetector::slot_geometry_changed);
        connect(screen,
                &QScreen::physicalSizeChanged,
                this,
                &ScreenChangeDetector::slot_physical_size_changed);
        connect(screen,
                &QScreen::logicalDotsPerInchChanged,
                this,
                &ScreenChangeDetector::slot_dpi_changed);
    }
}

void ScreenChangeDetector::adjust() {
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        _screen_rect_logical = screen->geometry();
        _pixel_ratio = screen->devicePixelRatio();

        _screen_rect_pixels =
            _screen_rect_logical.translated(-_screen_rect_logical.topLeft());

        const int w =
            static_cast<int>(_screen_rect_pixels.width() * _pixel_ratio + 1) &
            0xfffe;
        const int h =
            static_cast<int>(_screen_rect_pixels.height() * _pixel_ratio + 1) &
            0xfffe;
        _screen_rect_pixels.setWidth(w);
        _screen_rect_pixels.setHeight(h);
    }
}

void ScreenChangeDetector::slot_geometry_changed(const QRect &rect) {
    adjust();
    emit screen_pixels_changed(_screen_rect_pixels);
}

void ScreenChangeDetector::slot_physical_size_changed(const QSizeF &size) {
    adjust();
    emit screen_pixels_changed(_screen_rect_pixels);
}

void ScreenChangeDetector::slot_dpi_changed(qreal dpi) {
    adjust();
    emit screen_pixels_changed(_screen_rect_pixels);
}
