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

#include "gfxfader.h"
#include "string_format.h"
#include "utility.h"

#include <chrono>

#include <QDebug>
#include <QPainter>


// GfxFader::GfxFader
//
GfxFader::GfxFader(QGraphicsItem* parent,
                   const QColor& color,
                   double width,
                   ArrowType arrowtype,
                   double arrowsize,
                   double sustain,
                   double fade,
                   int verbosity) :
  GfxPath(parent, color, width, arrowtype, arrowsize,verbosity) {
    _sustain_secs = sustain;
    _fading_secs = fade;
    update_time();
    if (_verbosity >= VerboseAll) {
        qDebug() << "GfxFader created @" << this;
    }
}

GfxFader::~GfxFader() {
    if (_verbosity >= VerboseAll) { qDebug() << "GfxFader deleted @" << this; }
}

// GfxFader::create_undo_item
//
GfxUndoItem* GfxFader::create_undo_item() const {
    auto* undo = new GfxUndoFader(this);
    return undo;
}

// GfxFader::start
//
// start a path
//
void GfxFader::start(const QPointF& p,
                     Qt::KeyboardModifiers mods,
                     double pressure) {
    GfxPath::start(p, mods, pressure);
    update_time();
}

// GfxFader::stop
//
// stop a path
//
void GfxFader::stop(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    GfxPath::stop(p, mods, pressure);
    update_time();
}

// GfxFader::move
//
// extend the path
//
void GfxFader::move(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    GfxPath::move(p, mods, pressure);
    update_time();
}

void GfxFader::update_time() {
    std::chrono::milliseconds sust(static_cast<int>(_sustain_secs * 1000));
    std::chrono::milliseconds fade(static_cast<int>(_fading_secs * 1000));
    _sustained = std::chrono::steady_clock::now() + sust;
    _faded = std::chrono::steady_clock::now() + sust + fade;
}

GfxItem::DynResult GfxFader::dynamic() {
    const auto now = std::chrono::steady_clock::now();
    if (now > _faded) return GfxItem::DYN_DELITEM;
    if (now < _sustained) return GfxItem::DYN_CONTINUE;

    std::chrono::duration<double, std::milli> elapsed = now - _sustained;
    std::chrono::duration<double, std::milli> delta = _faded - _sustained;

    const double alpha =
        cap<double>(1.0 - elapsed.count() / delta.count(), 0.0, 1.0);
    setOpacity(alpha);
    return GfxItem::DYN_CONTINUE;
}

// GfxFader::dump
//
void GfxFader::dump(std::ostream& os) const {
    GfxPath::dump(os);
}

// ============================== undo ===============================

// GfxUndoFader::GfxUndoFader
//
GfxUndoFader::GfxUndoFader(const GfxFader* kpath) :
    GfxUndoItem(static_cast<const GfxItem*>(kpath)) {
    pen = kpath->pen();
    arrow_type = kpath->arrow_type();
    arrow_size = kpath->arrow_size();
    xy_path = kpath->path();
    faded = kpath->faded();
    sustained = kpath->sustained();
}

// GfxUndoFader::create_from_undo_item
//
GfxItem* GfxUndoFader::create_from_undo_item() const {
    GfxFader* kfader = new GfxFader(nullptr,
                                    pen.color(),
                                    pen.widthF(),
                                    arrow_type,
                                    arrow_size,
                                    1,
                                    1,
                                    verbosity);
    kfader->setData(GfxFader::KeyID, id);
    kfader->set_path(xy_path);
    kfader->set_faded(faded);
    kfader->set_sustained(sustained);
    return kfader;
}
