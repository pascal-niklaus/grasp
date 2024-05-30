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

#ifndef GFXSMOOTH_H
#define GFXSMOOTH_H

#include "grasp.h"
#include "gfxarrowitem.h"
#include "path.h"

#include <QPen>

class GfxUndoSmooth;

// GfxSmooth
//
class GfxSmooth : public GfxArrowItem {
  public:
    enum { Type = GFX_SMOOTH };

  public:
    GfxSmooth(QGraphicsItem* parent,
              const QColor& color,
              double width,
              ArrowType atype,
              double arrowsize,
              double simplify,
              double snap,
              int verbosity);
    ~GfxSmooth();

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
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_ROTATE |
                CAP_DELETE);
    }

    // - build system
    virtual void start(const QPointF& p,
                       Qt::KeyboardModifiers mods = Qt::NoModifier,
                       double pressure = 1.0) override;
    virtual void move(const QPointF& p,
                      Qt::KeyboardModifiers mods = Qt::NoModifier,
                      double pressure = 1.0) override;
    void move_internal(const QPointF& p, bool force);
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
    ArrowType arrow_type() const { return _arrow_type; }
    double arrow_size() const { return _arrow_size; }
    double snap() const { return _snap; }
    double simplify() const { return _simplify; }
    const Path& path() const { return _xy_path; }
    void set_path(const Path& path);

  private:
    // ----- internal helpers
    void repos_arrows(bool snapped = false);
    QPainterPath outline(double dist) const;

  private:
    QPen _pen;
    ArrowType _arrow_type = ARROW_NONE;
    double _arrow_size = 0.0;
    double _snap = 0.0;
    double _simplify = 0.0;
    GfxArrow* _arrow_start = nullptr;
    GfxArrow* _arrow_end = nullptr;

    Path _xy_path;
    Path _xy_path_raw;
    QPainterPath _path;

    Path _stashed_xy_path;
};

// GfxUndoSmooth
//
class GfxUndoSmooth : public GfxUndoItem {
  public:
    explicit GfxUndoSmooth(const GfxSmooth* kpath);
    virtual ~GfxUndoSmooth();

    virtual GfxItem* create_from_undo_item() const override;

    QPen pen;
    ArrowType arrow_type;
    double arrow_size;
    Path xy_path;
    double snap;
};

#endif  // GFXSMOOTH_H
