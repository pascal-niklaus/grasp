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

#ifndef GFXRECT_H
#define GFXRECT_H

#include "grasp.h"
#include "gfxarrowitem.h"

#include <QPen>

class GfxUndoRect;

// GfxRect
//
class GfxRect : public GfxItem {
  public:
    enum { Type = GFX_RECT };

  public:
    GfxRect(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double width = 1,
               double radius = 0);
    ~GfxRect();

    // ----- GfxItem methods

    // - undo system
    virtual GfxUndoItem* create_undo_item() const override;

    // - transform system
    virtual void stash() override;
    virtual void apply(const QTransform& m) override;
    virtual QRectF selectionBox() const override {
        return QRectF(_path.boundingRect());
    }
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_DELETE);
    }

    // - build system
    virtual void start(const QPointF& p,
                       Qt::KeyboardModifiers mods = Qt::NoModifier,
                       double pressure = 1.0) override;
    virtual void move(const QPointF& p,
                      Qt::KeyboardModifiers mods = Qt::NoModifier,
                      double pressure = 1.0) override;
    virtual void stop(const QPointF& p,
                      Qt::KeyboardModifiers mods = Qt::NoModifier,
                      double pressure = 1.0) override;

    // - debugging
    virtual void dump(std::ostream& os) const override;

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual QPainterPath shape() const override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

  public:
    // ----- item accessors
    QPen pen() const { return _pen; }
    double radius() const { return _radius; }
    const QRectF& rect() const { return _rect; }
    void set_rect(const QRectF& rect);

  private:
    // ----- internal helpers
    QPainterPath outline(double dist) const;
    void build_path(Qt::KeyboardModifiers mods);

  private:
    QPen _pen;
    double _radius;
    QRectF _rect;

    QPainterPath _path;

    QRectF _stashed_rect;
};

// GfxUndoRect
//
class GfxUndoRect : public GfxUndoItem {
  public:
    explicit GfxUndoRect(const GfxRect* kpath);
    virtual ~GfxUndoRect();

    virtual GfxItem* create_from_undo_item() const override;

    QPen pen;
    double radius;
    QRectF rect;
};

#endif  // GFXRECT_H
