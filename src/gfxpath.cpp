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

#include "gfxpath.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>


// GfxPath::GfxPath
//
GfxPath::GfxPath(QGraphicsItem* parent,
                 const QColor& color,
                 double width,
                 ArrowType arrowtype,
                 double arrowsize,
                 int verbosity) :
    GfxArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_size = arrowsize;
    _pen = QPen(color, width, Qt::SolidLine, Qt::RoundCap);
    GfxArrowItem::setPen(_pen);

    setFlag(QGraphicsItem::ItemIsSelectable);

    if (_verbosity >= VerboseAll) { qDebug() << "GfxPath created @" << this; }
}

GfxPath::~GfxPath() {
    if (_verbosity >= VerboseAll) { qDebug() << "GfxPath deleted @" << this; }
}


// GfxPath::create_undo_item
//
GfxUndoItem* GfxPath::create_undo_item() const {
    auto* undo = new GfxUndoPath(this);
    return undo;
}

// GfxPath::stash
//
// store pre-transformation state
//
void GfxPath::stash() {
  _stashed_xy_path = _xy_path;
}

// GfxPath::apply
//
// apply 2D affine transformation
//
void GfxPath::apply(const QTransform& m) {
    prepareGeometryChange();
    _xy_path = _stashed_xy_path;
    _xy_path.apply2D(m);

    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);

    repos_arrows();
}

// GfxPath::start
//
// start a path
//
void GfxPath::start(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    prepareGeometryChange();
    _path.clear();
    _xy_path.clear();
    _path.moveTo(p);
    _xy_path.append(p);
    repos_arrows();
}

// GfxPath::stop
//
// stop a path
//
void GfxPath::stop(const QPointF& p,
                   Qt::KeyboardModifiers mods,
                   double pressure) {
    prepareGeometryChange();
    _path.lineTo(p);
    _xy_path.append(p);

    // simplify the path TODO: make epsilon dependent on width ?
    _xy_path.douglas_peucker(0.9);

    prepareGeometryChange();

    repos_arrows();

    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);
}

// GfxPath::line_to
//
// extend the path
//
void GfxPath::move(const QPointF& p,
                   Qt::KeyboardModifiers mods,
                   double pressure) {
    if (XY(_path.currentPosition()).distance(XY(p)) >
        cap<double>(sqrt(_pen.widthF()), .9, 5.0)) {
        prepareGeometryChange();
        _path.lineTo(p);
        _xy_path.append(p);
        repos_arrows();
    }
}

// GfxPath::outline
//
// create a path outlining the path at given distance
//
QPainterPath GfxPath::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_xy_path.first());
    for (const auto& pt : _xy_path.path())
        p.lineTo(pt);

    QPainterPathStroker ps;
    ps.setWidth(_pen.widthF() + dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(p).simplified();
}

// GfxPath::shape
//
// selectable area
//
QPainterPath GfxPath::shape() const {
    return outline(20);
}

// GfxPath::repos_arrows
//
// reposition arrow heads according to path
//
void GfxPath::repos_arrows() {
    // search radius for angle
    const double radius = cap<double>(_pen.widthF() * 6.0, 10.0, 50.0);
    const double arrow_width = _arrow_size * _pen.widthF();
    if (_arrow_type & ARROW_START) {
        double rot = _xy_path.end_angle(radius, true);
        set_arrow_start(_xy_path.first(), arrow_width, rot);
    }
    if (_arrow_type & ARROW_END) {
        double rot = _xy_path.end_angle(radius, false);
        set_arrow_end(_xy_path.last(), arrow_width, rot);
    }
}

// GfxPath::boundingRect
//
// this area is re-painted
//
QRectF GfxPath::boundingRect() const {
    double delta = _pen.width() / 2.0 + 25;
    QRectF rect = _path.boundingRect().normalized();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// GfxPath::paint
//
void GfxPath::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawPath(outline(15));
    }
    painter->setPen(_pen);
    painter->drawPath(_path);
}

// GfxPath::set_path
//
// Set the path, recreating PainterPath
//
void GfxPath::set_path(const Path &path) {
    prepareGeometryChange();
    _xy_path = path;
    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);
    repos_arrows();
}

// GfxPath::dump
//
void GfxPath::dump(std::ostream& os) const {
    os << string_format(
        " (%.0f,%.0f)", _xy_path.first().x(), _xy_path.first().y());
    os << _arrow_type;
    os << string_format(
        "(%.0f,%.0f)", _xy_path.last().x(), _xy_path.last().y());
}

// ============================== undo ===============================

// GfxUndoPath::GfxUndoPath
//
GfxUndoPath::GfxUndoPath(const GfxPath* kpath) :
    GfxUndoItem(static_cast<const GfxItem*>(kpath)) {
    pen = kpath->pen();
    arrow_type = kpath->arrow_type();
    arrow_size = kpath->arrow_size();
    xy_path = kpath->path();
}

GfxUndoPath::~GfxUndoPath() {}

// GfxUndoPath::create_from_undo_item
//
GfxItem* GfxUndoPath::create_from_undo_item() const {
    GfxPath* kpath = new GfxPath(
        nullptr, pen.color(), pen.widthF(), arrow_type, arrow_size, verbosity);
    kpath->setData(GfxPath::KeyID, id);
    kpath->set_path(xy_path);
    return kpath;
}
