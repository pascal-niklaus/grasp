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

#include "gfxline.h"

#include "string_format.h"
#include "tools.h"
#include "utility.h"

#include <QPainter>

GfxLine::GfxLine(QGraphicsItem* parent,
                       const QColor& color,
                       double width,
                       ArrowType arrowtype,
                       double arrowsize) :
    GfxArrowItem(parent) {
    prepareGeometryChange();
    _arrow_type = arrowtype;
    _arrow_size = arrowsize;
    _pen = QPen(color, width, Qt::SolidLine, Qt::RoundCap);
    GfxArrowItem::setPen(_pen);
    _line_item = new QGraphicsLineItem(this);
    _line_item->setPen(_pen);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

GfxLine::~GfxLine() {}

// GfxLine::create_undo_item
//
// create an undo record that allows recreatine the line
//
GfxUndoItem* GfxLine::create_undo_item() const {
    auto* undo = new GfxUndoLine(this);
    return undo;
}

// GfxLine::stash
//
// store pre-transformation state
//
void GfxLine::stash() {
    _stashed_line = _line_item->line();
}

// GfxLine::apply
//
// apply 2D affine transformation
//
void GfxLine::apply(const QTransform& m) {
    prepareGeometryChange();
    _line_item->setLine(m.map(_stashed_line));
    repos_arrows();
}

// GfxLine::start
//
// start a path
//
void GfxLine::start(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    prepareGeometryChange();
    _line_item->setLine(QLineF(p, p));
    repos_arrows();
}

// GfxLine::stop
//
// stop a path
//
void GfxLine::stop(const QPointF& p,
                   Qt::KeyboardModifiers mods,
                   double pressure) {
    prepareGeometryChange();
    QLineF line = _line_item->line();
    line.setP2(p);
    // line must have length > 0, otherwise selecting it will not work
    if (line.length() < 0.01)
        line.setP2(QPointF(line.p1().x() + 0.01, line.p1().y() + 0.01));
    _line_item->setLine(line);
    repos_arrows();
}

// GfxLine::move
//
// move end of path
//
void GfxLine::move(const QPointF& p,
                   Qt::KeyboardModifiers mods,
                   double pressure) {
    stop(p);
}

// GfxLine::outline
//
// create a path outlining the path at given distance
//
QPainterPath GfxLine::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_line_item->line().p1());
    p.lineTo(_line_item->line().p2());

    QPainterPathStroker ps;
    ps.setWidth(dist);
    return ps.createStroke(p);
}

// GfxLine::shape
//
// selectable area
//
QPainterPath GfxLine::shape() const {
    return outline(15);
}

// GfxLine::boundingRect
//
// this area is re-painted
//
// it seems the boundingRect must not change without a call to
// prepareGeometryChange; therefore, the same rect is returned
// regardless of whether it is selected or not
QRectF GfxLine::boundingRect() const {
    // if (isSelected()) {
    return outline(20).boundingRect();
    // } else {
    return QRectF(0, 0, 0, 0);
    // }
};

// GfxLine::paint
//
void GfxLine::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::DashLine));
        painter->drawPath(outline(10));
    }
}

// GfxLine::repos_arrows
//
// reposition arrow heads according to path
//
void GfxLine::repos_arrows() {
    // search radius for angle
    const QPointF& p1 = _line_item->line().p1();
    const QPointF& p2 = _line_item->line().p2();
    const double rot = atan2(p2.y() - p1.y(), p2.x() - p1.x());
    const double arrow_width = _pen.widthF() * _arrow_size;
    if (_arrow_type & ARROW_START) {
        set_arrow_start(p1, arrow_width, rot + M_PI);
    }
    if (_arrow_type & ARROW_END) { set_arrow_end(p2, arrow_width, rot); }
}

// GfxLine::dump
//
void GfxLine::dump(std::ostream& os) const {
    os << string_format(" (%.0f,%.0f)",
                        _line_item->line().p1().x(),
                        _line_item->line().p1().y());
    os << _arrow_type;
    os << string_format("(%.0f,%.0f)",
                        _line_item->line().p2().x(),
                        _line_item->line().p2().y());
}

// ============================== undo ===============================

// GfxUndoLine::GfxUndoLine
//
// create undo record for line
//
GfxUndoLine::GfxUndoLine(const GfxLine* kline) :
    GfxUndoItem(static_cast<const GfxItem*>(kline)) {
    pen = kline->pen();
    arrow_type = kline->arrow_type();
    arrow_size = kline->arrow_size();
    line = kline->line();
}

GfxUndoLine::~GfxUndoLine() {
}

// GfxUndoLine::create_from_undo_item
//
// re-create the GfxLine
//
GfxItem* GfxUndoLine::create_from_undo_item() const {
    GfxLine* kline = new GfxLine(
        nullptr, pen.color(), pen.widthF(), arrow_type, arrow_size);
    kline->setData(GfxLine::KeyID, id);
    kline->start(line.p1());
    kline->stop(line.p2());
    return kline;
}
