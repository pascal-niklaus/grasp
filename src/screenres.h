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

#ifndef SCREENRES_H
#define SCREENRES_H

#include <QObject>
#include <QRect>

// ScreenChangeDetector
//
// emits signals when screen resolution changed
//
class ScreenChangeDetector : public QObject {
    Q_OBJECT
  public:
    explicit ScreenChangeDetector(QObject *parent = nullptr);

    QRect rect_pixels() const { return _screen_rect_pixels; }
    QRect rect_logical() const { return _screen_rect_logical; }

 signals:
    void screen_pixels_changed(QRect);

  private:
    void setup();
    void adjust();
  private slots:
    void slot_geometry_changed(const QRect &newGeometry);
    void slot_physical_size_changed(const QSizeF &newPhysicalSize);
    void slot_dpi_changed(qreal dpi);

  private:
    QRect _screen_rect_logical;
    QRect _screen_rect_pixels;
    double _pixel_ratio;
};

#endif
