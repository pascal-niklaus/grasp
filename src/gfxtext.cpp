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

#include "gfxtext.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>
#include <QFontDatabase>
#include <QFontMetrics>

// GfxText::GfxText
//
GfxText::GfxText(QGraphicsItem* parent,
                       const QColor& color,
                 QFont font, int verbosity) :
    GfxItem(parent) {
    _size = font.pointSizeF();
    _color = color;
    _crsr_pos = 0;
    _font = font;
    set_verbosity(verbosity);
    setFlag(QGraphicsItem::ItemIsSelectable);
    if (verbosity >= VerboseAll) { qDebug() << "GfxText created @" << this; }
}

GfxText::~GfxText() {
    if (_verbosity >= VerboseAll) { qDebug() << "GfxText deleted @" << this; }
}


// GfxText::create_undo_item
//
GfxUndoItem* GfxText::create_undo_item() const {
    auto* undo = new GfxUndoText(this);
    return undo;
}

// GfxText::stash
//
// store pre-transformation state
//
void GfxText::stash() {
    _stashed_path = _path;
}

// GfxText::apply
//
// apply 2D affine transformation
//
void GfxText::apply(const QTransform& m) {
    prepareGeometryChange();

    set_path(m.map(_stashed_path));
}

// GfxText::start
//
void GfxText::start(const QPointF& p,
                    Qt::KeyboardModifiers mods,
                    double pressure) {
    _location = p;
    build_path(true);
}

// GfxText::stop
//
void GfxText::stop(const QPointF& p,
                   Qt::KeyboardModifiers mods,
                   double pressure) {
    _location = p;
    build_path(true);
}

// GfxText::move
//
void GfxText::move(const QPointF& p,
                   Qt::KeyboardModifiers mods,
                   double pressure) {
    _location = p;
    build_path(true);
}

GfxItem::KeyResult GfxText::keystroke(QKeyCombination key,
                                            const QString& text) {
    if (key.key() == Qt::Key_Escape) {
        _text.clear();
        build_path(false);
        return KEY_STOP;
    }

    if (key == QKeyCombination(Qt::NoModifier | Qt::Key_Return)) {
        build_path(false);
        return KEY_STOP;
    }

    if (key.key() == Qt::Key_Plus &&
        key.keyboardModifiers() & Qt::ControlModifier) {
        _font.setPointSizeF(_font.pointSizeF() * 1.333);
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_Minus &&
        key.keyboardModifiers() & Qt::ControlModifier) {
        _font.setPointSizeF(_font.pointSizeF() / 1.333);
        build_path(true);
        return KEY_PROCESSED;
    }

    if (key.key() == Qt::Key_Backspace) {
        if (_crsr_pos > 0) {
            _text.remove(_crsr_pos - 1, 1);
            _crsr_pos--;
        }
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_Home) {
        _crsr_pos = 0;
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_End) {
        _crsr_pos = _text.length();
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_Delete) {
        if (_crsr_pos < static_cast<int>(_text.length())) {
            _text.remove(_crsr_pos, 1);
        }
        build_path(true);
        return KEY_PROCESSED;
    }

    if (key.key() == Qt::Key_Left) {
        // move cursor left
        _crsr_pos = cap<int>(_crsr_pos - 1, 0, _text.length());
        build_path(true);
    };
    if (key.key() == Qt::Key_Right) {
        // move cursor right
        _crsr_pos = cap<int>(_crsr_pos + 1, 0, _text.length());
        build_path(true);
    };

    if (text.length() == 1) {
        const auto ch = text.front();
        if (ch > 0 && ch < ' ' && ch != QChar(13)) return KEY_IGNORED;

        _text.insert(_crsr_pos, text);
        _crsr_pos += text.length();

        build_path(true);
        return KEY_PROCESSED;
    }

    return KEY_IGNORED;
}

void GfxText::build_path(bool show_cursor) {
    prepareGeometryChange();

    QStringList lines = _text.split(QChar(13));

    _path.clear();
    QFontMetrics fm(_font);

    int y = _location.y();
    int crsr_in_line = _crsr_pos;
    for (const auto& line : lines) {
        _path.addText(
            _location.x(), y, QFont(_font), line);

        if (show_cursor && crsr_in_line >= 0 && crsr_in_line <= static_cast<int>(line.length())) {
            const int crsrx =
                _location.x() +
              fm.horizontalAdvance(line, crsr_in_line);
            const int crsry0 = y + fm.descent();
            const int crsry1 = y - fm.ascent();

            const int dx = 4;
            const int dy = 2;
            _path.moveTo(crsrx, crsry0);
            _path.lineTo(crsrx + dx, crsry0 + dy);
            _path.lineTo(crsrx, crsry0);
            _path.lineTo(crsrx - dx, crsry0 + dy);
            _path.lineTo(crsrx, crsry0);
            _path.lineTo(crsrx, crsry1);
            _path.lineTo(crsrx - dx, crsry1 - dy);
            _path.lineTo(crsrx, crsry1);
            _path.lineTo(crsrx + dx, crsry1 - dy);
            _path.lineTo(crsrx, crsry1);
        }

        y += fm.height();
        crsr_in_line -= (1 + line.length());
    }

}

// GfxText::outline
//
// create a path outlining the path at given distance
//
QPainterPath GfxText::outline(double dist) const {
    QPainterPathStroker ps;
    ps.setWidth(dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(_path).simplified();
}

// GfxText::shape
//
// selectable area
//
QPainterPath GfxText::shape() const {
    return outline(20);
}

// GfxText::boundingRect
//
// this area is re-painted
//
QRectF GfxText::boundingRect() const {
    const double delta = 35;
    QRectF rect = _path.boundingRect();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// GfxText::paint
//
void GfxText::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawPath(outline(15));
    }
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(_color, 0));
    painter->setBrush(_color);
    painter->drawPath(_path);
}

// GfxText::set_path
//
// Set the path, recreating PainterPath
//
void GfxText::set_path(const QPainterPath& path) {
    prepareGeometryChange();
    _path = path;
}

// GfxText::dump
//
void GfxText::dump(std::ostream& os) const {
}

// ============================== undo ===============================

// GfxUndoText::GfxUndoText
//
GfxUndoText::GfxUndoText(const GfxText* ktext) :
    GfxUndoItem(static_cast<const GfxItem*>(ktext)) {
    color = ktext->color();
    font = ktext->font();
    path = ktext->path();
}

GfxUndoText::~GfxUndoText() {}

// GfxUndoText::create_from_undo_item
//
GfxItem* GfxUndoText::create_from_undo_item() const {
    GfxText* ktext = new GfxText(nullptr, color, font, verbosity);
    ktext->setData(GfxText::KeyID, id);
    ktext->set_path(path);
    return ktext;
}
