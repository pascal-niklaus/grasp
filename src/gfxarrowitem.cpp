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

#include "grasp.h"
#include "gfxitem.h"
#include "gfxarrow.h"
#include "gfxarrowitem.h"

#include "xy.h"

#include <QBrush>
#include <QPainter>


// GfxArrowItem::GfxArrowItem
//
// create item with two arrow heads; the heads are not shown until
// position is set
//
GfxArrowItem::GfxArrowItem(QGraphicsItem* parent) : GfxItem(parent) {
    prepareGeometryChange();
    _arrow_start = new GfxArrow(this);
    _arrow_start->hide();
    _arrow_end = new GfxArrow(this);
    _arrow_end->hide();
}

GfxArrowItem::~GfxArrowItem() {
    // delete _arrow_start;
    // delete _arrow_end;
}

// GfxArrowItem::boundingRect
//
// return nothing since nothing is drawn here
//
QRectF GfxArrowItem::boundingRect() const {
    return QRectF(0, 0, 0, 0);
}

// GfxArrowItem::paint
//
// paint nothing
//
void GfxArrowItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget) {}

// GfxArrowItem::setPen
//
// set the pen (basically the color)
//
void GfxArrowItem::setPen(const QPen &pen) {
    QPen p = pen;
    p.setWidth(0);
    _arrow_start->setPen(p);
    _arrow_start->setBrush(p.color());
    _arrow_end->setPen(p);
    _arrow_end->setBrush(p.color());
}

// GfxArrowItem::set_arrow_<start|end>
//
// display the arrow; a width <= 0 indicates the arrow is hidded
//
void GfxArrowItem::set_arrow_start(const QPointF &pos,
                                double size,
                                double rotation) {
    if (size > 0.0) {
        _arrow_start->setPos(pos);
        _arrow_start->setScale(size);
        _arrow_start->setRotation(rotation * 180 / M_PI);
        _arrow_start->show();
    } else {
        _arrow_start->hide();
    }
}

void GfxArrowItem::set_arrow_end(const QPointF &pos,
                              double size,
                              double rotation) {
    if (size > 0.0) {
        _arrow_end->setPos(pos);
        _arrow_end->setScale(size);
        _arrow_end->setRotation(rotation * 180.0 / M_PI);
        _arrow_end->show();
    } else {
        _arrow_end->hide();
    }
}

std::ostream& operator<<(std::ostream& os, const ArrowType& a) {
    switch (a) {
    case ARROW_START:
        os << "<--";
        break;
    case ARROW_END:
        os << "-->";
        break;
    case ARROW_DOUBLE:
        os << "<->";
        break;
    case ARROW_NONE:
      os << "---";
        break;
    }
    return os;
}

