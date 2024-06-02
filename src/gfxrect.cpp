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

#include "gfxrect.h"
#include "string_format.h"
#include "utility.h"
#include "verbosity.h"

#include <QDebug>
#include <QPainter>


// GfxRect::GfxRect
//
GfxRect::GfxRect(QGraphicsItem* parent,
                 const QColor& color,
                 double width,
                 double radius) :
    GfxItem(parent) {
    _radius = radius;
    _pen = QPen(color, width, Qt::SolidLine, Qt::RoundCap);

    setFlag(QGraphicsItem::ItemIsSelectable);
    if (Verbosity::get() >= VerboseAll) {
        qDebug() << "GfxRect created @" << this;
    }
}

GfxRect::~GfxRect() {
    if (Verbosity::get() >= VerboseAll) {
        qDebug() << "GfxRect deleted @" << this;
    }
}

// GfxRect::create_undo_item
//
GfxUndoItem* GfxRect::create_undo_item() const {
    auto* undo = new GfxUndoRect(this);
    return undo;
}

// GfxRect::stash
//
// store pre-transformation state
//
void GfxRect::stash() {
    _stashed_rect = _rect;
}

// GfxRect::apply
//
// apply 2D affine transformation
//
void GfxRect::apply(const QTransform& m) {
    prepareGeometryChange();
    set_rect(m.mapRect(_stashed_rect));
}

// GfxRect::start
//
// start a path
//
void GfxRect::start(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    _rect.setTopLeft(p);
    move(p, mods);
}

// GfxRect::stop
//
// stop a path
//
void GfxRect::stop(const QPointF& p, Qt::KeyboardModifiers mods, double pressure) {
  move(p, mods);
  if (mods != Qt::NoModifier) _radius = 0.0;
}

// GfxRect::move
//
// drag rectangle corner
//
void GfxRect::move(const QPointF& p, Qt::KeyboardModifiers mods, double pressure) {
    if (mods & Qt::ShiftModifier) {
        const auto x0 = _rect.left();
        const auto y0 = _rect.top();
        auto w = p.x() - x0;
        auto h = p.y() - y0;
        if (fabs(w) > fabs(h))
            h = sign(h) * fabs(w);
        else w = sign(w)*fabs(h);
        _rect.setBottomRight(QPointF(x0 + w, y0 + h));
    } else {
        _rect.setBottomRight(p);
    }
    build_path(mods);
}

void GfxRect::build_path(Qt::KeyboardModifiers mods) {
    prepareGeometryChange();
    auto nrect = _rect.normalized();
    _path.clear();
    double r = nrect.width() > 2 * _radius ? _radius : 0.0;
    if (mods & Qt::ControlModifier) r = 0.0;
    _path.addRoundedRect(nrect, r, r, Qt::AbsoluteSize);
}

// GfxRect::outline
//
// create a path outlining the path at given distance
//
QPainterPath GfxRect::outline(double dist) const {
    QPainterPathStroker ps;
    ps.setWidth(_pen.widthF() + dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(_path);
}

// GfxRect::shape
//
// selectable area
//
QPainterPath GfxRect::shape() const {
    return outline(20);
}

// GfxRect::boundingRect
//
// this area is re-painted
//
QRectF GfxRect::boundingRect() const {
    double delta = _pen.width() / 2.0 + 25;
    QRectF rect = _rect.normalized();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// GfxRect::paint
//
void GfxRect::paint(QPainter* painter,
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

// GfxRect::set_path
//
// Set the path, recreating PainterPath
//
void GfxRect::set_rect(const QRectF& rect) {
    prepareGeometryChange();
    _rect = rect.normalized();
    build_path(Qt::NoModifier);
}

// GfxRect::dump
//
void GfxRect::dump(std::ostream& os) const {
    os << string_format(" (%.0f,%.0f) to (%.0f,%.0f)",
                        _rect.left(),
                        _rect.top(),
                        _rect.right(),
                        _rect.bottom());
}

// ============================== undo ===============================

// GfxUndoRect::GfxUndoRect
//
GfxUndoRect::GfxUndoRect(const GfxRect* krect) :
    GfxUndoItem(static_cast<const GfxItem*>(krect)) {
    pen = krect->pen();
    radius = krect->radius();
    rect = krect->rect();
}

GfxUndoRect::~GfxUndoRect() {}

// GfxUndoRect::create_from_undo_item
//
GfxItem* GfxUndoRect::create_from_undo_item() const {
    GfxRect* krect =
        new GfxRect(nullptr, pen.color(), pen.widthF(), radius);
    krect->setData(GfxRect::KeyID, id);
    krect->set_rect(rect);
    return krect;
}
