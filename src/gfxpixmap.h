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

#ifndef GFXPIXMAP_H
#define GFXPIXMAP_H

#include "grasp.h"
#include "gfxarrowitem.h"

#include <QPen>

class GfxUndoPixmap;

// GfxPixmap
//
class GfxPixmap : public GfxItem {
  public:
    enum { Type = GFX_PIXMAP };

  public:
    GfxPixmap(QGraphicsItem* parent,
              const QColor& color,
              double width,
              int verbosity);
    ~GfxPixmap();

    // ----- GfxItem methods

    // - undo system
    virtual GfxUndoItem* create_undo_item() const override;

    // - transform system
    virtual void stash() override;
    virtual void apply(const QTransform& m) override;
    virtual QRectF selectionBox() const override { return _sel_rect; }
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ISOSCALE | CAP_DELETE);
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
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

  public:
    // ----- item accessors
    QPen pen() const { return _pen; }
    const QPixmap& pixmap() const { return _pixmap; }
    const QRectF& sel_rect() const { return _sel_rect; }

    void set_pixmap(const QPixmap& pixmap, const QRectF& place);

  private:
    bool drag_frame;
    QPen _pen;
    QRectF _sel_rect;
    QRectF _stashed_rect;
    QPixmap _pixmap;
};

// GfxUndoPixmap
//
class GfxUndoPixmap : public GfxUndoItem {
  public:
    explicit GfxUndoPixmap(const GfxPixmap* kpath);

    virtual GfxItem* create_from_undo_item() const override;

    QRectF _rect;
    QPixmap _pixmap;
};

#endif  // GFXPIXMAP_H
