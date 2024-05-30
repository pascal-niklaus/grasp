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

#ifndef GFXLINE_H
#define GFXLINE_H

#include "gfxarrowitem.h"
#include "path.h"

#include <QPen>

class GfxUndoLine;

// GfxLine
//
class GfxLine : public GfxArrowItem {
  public:
    enum { Type = GFX_LINE };

  public:
    GfxLine(QGraphicsItem* parent,
            const QColor& color,
            double width,
            ArrowType atype,
            double arrowsize,
            int verbosity);
    ~GfxLine();

    virtual GfxUndoItem* create_undo_item() const override;

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

    virtual QRectF selectionBox() const override {
        return QRectF(_line_item->line().p1(), _line_item->line().p2());
    }
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_ROTATE |
                CAP_DELETE);
    }

    virtual void stash() override;
    virtual void apply(const QTransform& m) override;

    virtual void start(const QPointF& p,
                       Qt::KeyboardModifiers mods = Qt::NoModifier,
                       double pressure = 1.0) override;
    virtual void move(const QPointF& p,
                      Qt::KeyboardModifiers mods = Qt::NoModifier,
                      double pressure = 1.0) override;
    virtual void stop(const QPointF& p,
                      Qt::KeyboardModifiers mods = Qt::NoModifier,
                      double pressure = 1.0) override;

    virtual void dump(std::ostream& os) const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr);

    QPainterPath outline(double dist) const;
    virtual QPainterPath shape() const override;

    QPen pen() const { return _pen; }
    ArrowType arrow_type() const { return _arrow_type; }
    double arrow_size() const { return _arrow_size; }
    QLineF line() const { return _line_item->line(); }

  private:
    void repos_arrows();

  private:
    QPen _pen;
    ArrowType _arrow_type = ARROW_NONE;
    double _arrow_size = 0.0;
    QGraphicsLineItem* _line_item = nullptr;
    QLineF _stashed_line;
};

// GfxUndoLine
//
class GfxUndoLine : public GfxUndoItem {
  public:
    explicit GfxUndoLine(const GfxLine* kline);
    virtual ~GfxUndoLine();
    virtual GfxItem* create_from_undo_item() const override;

    QPen pen;
    ArrowType arrow_type;
    double arrow_size;
    QLineF line;
};

#endif  // GFXLINE_H
