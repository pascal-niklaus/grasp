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

#ifndef GFXFADER_H
#define GFXFADER_H

#include "grasp.h"
#include "gfxpath.h"
#include "path.h"

#include <QPen>

class GfxUndoFader;

// GfxFader
//
class GfxFader : public GfxPath {
  public:
    enum { Type = GFX_FADER };

  public:
    GfxFader(QGraphicsItem* parent,
             const QColor& color,
             double width,
             ArrowType atype,
             double arrowsize,
             double sustain,
             double fade);
    ~GfxFader();

    // ----- GfxItem methods

    // - undo system
    virtual GfxUndoItem* create_undo_item() const override;

    // - transform system
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_ROTATE |
                CAP_DYNAMIC | CAP_DELETE);
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

    virtual DynResult dynamic() override;

  public:
    // ----- item accessors
    std::chrono::steady_clock::time_point sustained() const {
        return _sustained;
    }
    std::chrono::steady_clock::time_point faded() const {
        return _faded;
    }
    void set_sustained(std::chrono::steady_clock::time_point val) {
        _sustained = val;
    }
    void set_faded(std::chrono::steady_clock::time_point val) { _faded = val; }

  private:
    // ----- internal helpers
    void update_time();

  private:
    double _sustain_secs;
    double _fading_secs;
    std::chrono::steady_clock::time_point _sustained;
    std::chrono::steady_clock::time_point _faded;
};

// GfxUndoFader
//
class GfxUndoFader : public GfxUndoItem {
  public:
    explicit GfxUndoFader(const GfxFader* kpath);

    virtual GfxItem* create_from_undo_item() const override;

    QPen pen;
    ArrowType arrow_type;
    double arrow_size;
    Path xy_path;
    std::chrono::steady_clock::time_point sustained;
    std::chrono::steady_clock::time_point faded;
};

#endif  // GFXFADER_H
