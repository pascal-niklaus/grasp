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

#include "gfxortho.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>

// GfxOrtho::GfxOrtho
//
GfxOrtho::GfxOrtho(QGraphicsItem* parent,
                         const QColor& color,
                         double width,
                         ArrowType arrowtype,
                         double arrowsize,
                         double simplify,
                         double snap,
                         double radius,
                         double minlen,
                         double maxangle) :
    GfxArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_size = arrowsize;
    _pen = QPen(color, width, Qt::SolidLine, Qt::RoundCap);
    GfxArrowItem::setPen(_pen);
    _snap = snap;
    _simplify = simplify;
    _radius = radius;
    _minlen = minlen;
    _maxangle = maxangle;

    setFlag(QGraphicsItem::ItemIsSelectable);
}

GfxOrtho::~GfxOrtho() {}


// GfxOrtho::create_undo_item
//
GfxUndoItem* GfxOrtho::create_undo_item() const {
    auto* undo = new GfxUndoOrtho(this);
    return undo;
}

// GfxOrtho::stash
//
// store pre-transformation state
//
void GfxOrtho::stash() {
  _stashed_xy_path = _xy_path;
}

// GfxOrtho::apply
//
// apply 2D affine transformation
//
void GfxOrtho::apply(const QTransform& m) {
    prepareGeometryChange();
    _xy_path = _stashed_xy_path;
    _xy_path.apply2D(m);

    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);

    repos_arrows();
}

// GfxOrtho::start
//
// start a path
//
void GfxOrtho::start(const QPointF& p,
                     Qt::KeyboardModifiers mods,
                     double pressure) {
    _xy_path_raw.clear();
    move_internal(p, true);
}

// GfxOrtho::stop
//
// stop a path
//
void GfxOrtho::stop(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    move_internal(p, true);
    _xy_path_raw.clear();
    _raw_path.clear();
}

// GfxOrtho::line_to
//
// extend the path
//
void GfxOrtho::move(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    move_internal(p, false);
}

void GfxOrtho::move_internal(const QPointF& p, bool force) {
    if (force || XY(_path.currentPosition()).distance(XY(p)) >
                     cap<double>(sqrt(_pen.widthF()), .9, 5.0)) {
        _xy_path_raw.append(p);
        _xy_path_raw.backtrack(15, 10, 5);

        _xy_path = _xy_path_raw;

        bool snapped = false;
        if (_simplify > 0.0) {
            _xy_path.douglas_peucker(_simplify);
            if (_snap > 0.1) snapped = _xy_path.snap_ends(_snap);

            _xy_path.orthogonalize(_maxangle, _minlen);
            _xy_path.round_corners(_radius, 6, snapped);
        }

        prepareGeometryChange();
        _path.clear();
        _path.moveTo(_xy_path.first());
        for (int i = 1; i < _xy_path.size(); i++) _path.lineTo(_xy_path[i]);
        repos_arrows(snapped);

        _raw_path.clear();
        _raw_path.moveTo(_xy_path_raw.first());
        for (int i = 1; i < _xy_path_raw.size(); i++) _raw_path.lineTo(_xy_path_raw[i]);
    }
}

// GfxOrtho::outline
//
// create a path outlining the path at given distance
//
QPainterPath GfxOrtho::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_xy_path.first());
    for (const auto& pt : _xy_path.path()) p.lineTo(pt);

    QPainterPathStroker ps;
    ps.setWidth(_pen.widthF() + dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(p).simplified();
}

// GfxOrtho::shape
//
// selectable area
//
QPainterPath GfxOrtho::shape() const {
    return outline(20);
}

// GfxOrtho::repos_arrows
//
// reposition arrow heads according to path
//
void GfxOrtho::repos_arrows(bool snapped) {
    // search radius for angle
    const double radius = cap<double>(_pen.widthF() * 6.0, 10.0, 50.0);
    const double arrow_width = snapped ? 0.0 : _arrow_size * _pen.widthF();
    if (_arrow_type & ARROW_START) {
        double rot = _xy_path.end_angle(radius, true);
        set_arrow_start(_xy_path.first(), arrow_width, rot);
    }
    if (_arrow_type & ARROW_END) {
        double rot = _xy_path.end_angle(radius, false);
        set_arrow_end(_xy_path.last(), arrow_width, rot);
    }
}

// GfxOrtho::boundingRect
//
// this area is re-painted
//
QRectF GfxOrtho::boundingRect() const {
    double delta = _pen.width() / 2.0 + 25;
    QRectF rect = _path.boundingRect().united(_raw_path.boundingRect());
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// GfxOrtho::paint
//
void GfxOrtho::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawPath(outline(15));
    }
    painter->setPen(_pen);
    painter->drawPath(_path);

    painter->setPen(QPen(_pen.color(),1, Qt::DashLine));
    painter->drawPath(_raw_path);
}

// GfxOrtho::set_path
//
// Set the path, recreating PainterPath
//
void GfxOrtho::set_path(const Path& path) {
    prepareGeometryChange();
    _xy_path = path;
    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);
    repos_arrows();
}

// GfxOrtho::dump
//
void GfxOrtho::dump(std::ostream& os) const {
    os << string_format(
        " (%.0f,%.0f)", _xy_path.first().x(), _xy_path.first().y());
    os << _arrow_type;
    os << string_format(
        "(%.0f,%.0f)", _xy_path.last().x(), _xy_path.last().y());
}

// ============================== undo ===============================

// GfxUndoOrtho::GfxUndoOrtho
//
GfxUndoOrtho::GfxUndoOrtho(const GfxOrtho* kortho) :
    GfxUndoItem(static_cast<const GfxItem*>(kortho)) {
    pen = kortho->pen();
    arrow_type = kortho->arrow_type();
    arrow_size = kortho->arrow_size();
    xy_path = kortho->path();
    snap = kortho->snap();
}

GfxUndoOrtho::~GfxUndoOrtho() {}

// GfxUndoOrtho::create_from_undo_item
//
GfxItem* GfxUndoOrtho::create_from_undo_item() const {
    GfxOrtho* kortho =
        new GfxOrtho(nullptr,
                        pen.color(),
                        pen.widthF(),
                        xy_path.end_distance() < snap ? ARROW_NONE : arrow_type,
                        arrow_size,
                        10,
                        0,
                        10,
                        10,
                        10);
    kortho->setData(GfxOrtho::KeyID, id);
    kortho->set_path(xy_path);
    return kortho;
}
