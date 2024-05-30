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

#include "gfxpixmap.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>


// GfxPixmap::GfxPixmap
//
GfxPixmap::GfxPixmap(QGraphicsItem* parent,
                     const QColor& color,
                     double width,
                     int verbosity) :
    GfxItem(parent) {
    _pen = QPen(color, width, Qt::DashLine, Qt::RoundCap);
    setFlag(QGraphicsItem::ItemIsSelectable);
    set_verbosity(verbosity);
    drag_frame = false;
    setZValue(-10.0);
    if (_verbosity >= VerboseAll) { qDebug() << "GfxPixmap created @" << this; }
}

GfxPixmap::~GfxPixmap() {
    if (_verbosity >= VerboseAll) { qDebug() << "GfxPixmap deleted @" << this; }
}

// GfxPixmap::create_undo_item
//
GfxUndoItem* GfxPixmap::create_undo_item() const {
    auto* undo = new GfxUndoPixmap(this);
    return undo;
}

// GfxPixmap::stash
//
// store pre-transformation state
//
void GfxPixmap::stash() {
    _stashed_rect = _sel_rect;
}

// GfxPixmap::apply
//
// apply 2D affine transformation
//
void GfxPixmap::apply(const QTransform& m) {
    prepareGeometryChange();
    _sel_rect = m.mapRect(_stashed_rect);
}

// GfxPixmap::start
//
// start a path
//
void GfxPixmap::start(const QPointF& p,
                      Qt::KeyboardModifiers mods,
                      double pressure) {
    _sel_rect.setTopLeft(p);
    drag_frame = true;
    move(p);
}

// GfxPixmap::stop
//
// stop a path
//
void GfxPixmap::stop(const QPointF& p,
                     Qt::KeyboardModifiers mods,
                     double pressure) {
    drag_frame = false;
    move(p);
}

// GfxPixmap::move
//
// drag rectangle corner
//
void GfxPixmap::move(const QPointF& p,
                     Qt::KeyboardModifiers mods,
                     double pressure) {
    prepareGeometryChange();
    _sel_rect.setBottomRight(p);
}

// GfxPixmap::boundingRect
//
// this area is re-painted
//
QRectF GfxPixmap::boundingRect() const {
    double delta = _pen.width() / 2.0 + 15;
    QRectF rect = _sel_rect.normalized();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// GfxPixmap::paint
//
void GfxPixmap::paint(QPainter* painter,
                         const QStyleOptionGraphicsItem* option,
                         QWidget* widget) {
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawRect(_sel_rect.adjusted(-5, -5, 5, 5));
    }
    if (drag_frame) {
        painter->setPen(_pen);
        painter->drawRect(_sel_rect);
    }
    if (!_pixmap.isNull()) {
        painter->drawPixmap(_sel_rect, _pixmap, _pixmap.rect());
    }
}

// GfxPixmap::set_pixmap
//
// Set the pixmap and it's place
//
void GfxPixmap::set_pixmap(const QPixmap& image, const QRectF& place) {
    prepareGeometryChange();
    _sel_rect = place;
    _pixmap = image;
}

// GfxPixmap::dump
//
void GfxPixmap::dump(std::ostream& os) const {
    os << string_format(" (%.0f,%.0f) to (%.0f,%.0f)",
                        _sel_rect.left(),
                        _sel_rect.top(),
                        _sel_rect.right(),
                        _sel_rect.bottom());
}

// ============================== undo ===============================

// GfxUndoPixmap::GfxUndoPixmap
//
GfxUndoPixmap::GfxUndoPixmap(const GfxPixmap* kpixmap) :
    GfxUndoItem(static_cast<const GfxItem*>(kpixmap)) {
    _rect = kpixmap->sel_rect();
    _pixmap = kpixmap->pixmap();
}

// GfxUndoPixmap::create_from_undo_item
//
GfxItem* GfxUndoPixmap::create_from_undo_item() const {
    GfxPixmap* krect = new GfxPixmap(nullptr, Qt::yellow, 1, verbosity);
    krect->setData(GfxPixmap::KeyID, id);
    krect->set_pixmap(_pixmap, _rect);
    krect->setZValue(-10.0);
    return krect;
}
