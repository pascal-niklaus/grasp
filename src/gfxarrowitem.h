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

#ifndef GFXARROWITEM_H
#define GFXARROWITEM_H

#include "gfxitem.h"

class GfxArrow;

// GfxArrowItem
//
class GfxArrowItem : public GfxItem {
  public:
    enum { Type = GFX_ARROWITEM };

  public:
    GfxArrowItem(QGraphicsItem* parent = nullptr);
    ~GfxArrowItem();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

    virtual QRectF selectionBox() const override { return QRectF(); }
    virtual TransformCaps caps() const override { return CAP_NONE; }

    void setPen(const QPen& pen);

    void set_arrow_start(const QPointF& pos, double size, double rotation);
    void set_arrow_end(const QPointF& pos, double size, double rotation);

  private:
    GfxArrow* _arrow_start;
    GfxArrow* _arrow_end;
};

std::ostream& operator<<(std::ostream& os, const ArrowType& a);


#endif  // GFXARROWITEM_H
