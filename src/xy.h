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

#ifndef XY_H
#define XY_H

#include <qpoint.h>
#include <ostream>

#include <QPointF>

class QTransform;

// XY
//
// Coordinate (x,y)
//
class XY : public QPointF {
  public:
    XY();
    explicit XY(const QPointF& p);
    explicit XY(double x, double y);

    ~XY() = default;

    void add(const XY &p);

    void apply2d(const QTransform& m);
    void apply2d(const QTransform&, XY &result) const;
    XY transformed2d(const QTransform& m) const;

    XY midpoint(const XY &p) const;
    XY vector_to(const XY &p) const;
    double vector_length() const;
    double angle_to(const XY &p) const;

    double distance_from_line(const XY &line_start, const XY &line_end) const;
    double distance_from_segment(const XY &line_start, const XY &line_end) const;

    double distance(const XY &p) const;
    static double distance(const XY &p1, const XY &p2);
    static double angle_between(const XY &src1,
                                const XY &src2,
                                const XY &dst1,
                                const XY &dst2);

    friend std::ostream &operator<<(std::ostream &os, const XY &p);
};

// XYW
//
// A coordinate (x,y) with associated stroke width
//
class XYW : public XY {
  public:
    XYW() : XY(){};
    XYW(double x, double y, double w) : XY(x, y), _w(w) {}
    ~XYW() = default;

    double w() const { return _w; }
    void set_w(double w) { _w = w; }

    friend std::ostream &operator<<(std::ostream &os, const XYW &p);

  protected:
    double _w = 1.0;
};

#endif
