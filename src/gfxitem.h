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

#ifndef GFXITEM_H
#define GFXITEM_H

#include <QTransform>

#include "grasp.h"
#include "tools.h"

class QPointF;
class GfxUndoItem;

// GfxItem
//
// abstract base class for customized QGraphcisItem that "builds"
// itself when receiving mouse events
//
class GfxItem : public QGraphicsItem {
  public:
    // data QVariant members
    enum { KeyID = 1, KeyHandle = 2, KeyDash = 3, KeyDashAux = 4 };

    enum KeyResult {
        KEY_IGNORED,    // this particular keystroke was not consumed
        KEY_PROCESSED,  // this keystroke was consumed
        KEY_START,      // indicates that key events should be fed to item
        KEY_STOP        // indicate that key feeding should stop
    };
    enum DynResult {
        DYN_DELITEM,   // delete the item !
        DYN_STOP,      // no further calls needed
        DYN_CONTINUE,  // continue calling
    };

    GfxItem(QGraphicsItem* parent) : QGraphicsItem(parent) {}
    virtual ~GfxItem() = 0;

    // - undo system
    virtual GfxUndoItem* create_undo_item() const = 0;

    // - transform system
    virtual void stash() = 0;
    virtual void apply(const QTransform& m) = 0;
    virtual QRectF selectionBox() const = 0;
    virtual TransformCaps caps() const = 0;

    // - build system
    virtual void start(const QPointF& p,
                       Qt::KeyboardModifiers mods = Qt::NoModifier,
                       double pressure = 1.0) = 0;
    virtual void move(const QPointF& p,
                      Qt::KeyboardModifiers mods = Qt::NoModifier,
                      double pressure = 1.0) = 0;
    virtual void stop(const QPointF& p,
                      Qt::KeyboardModifiers mods = Qt::NoModifier,
                      double pressure = 1.0) = 0;
    virtual KeyResult keystroke(QKeyCombination key, const QString& text) {
        return KEY_IGNORED;
    }
    virtual DynResult dynamic() { return DYN_STOP; }

    // - debugging
    virtual void dump(std::ostream& os) const {};

  public:
    bool select();
    bool deselect();
    bool toggleselect();

    void pgc() { prepareGeometryChange(); }

    void set_new_ID() { setData(GfxItem::KeyID, Tools::generate_ID()); }

    static QString type_txt(int item);

    friend std::ostream& operator<<(std::ostream& os, const GfxItem& item);
    friend std::ostream& operator<<(std::ostream& os, const GfxItem* item);
};

// GfxUndoItem
//
// abstract base class for parallel "undo" analogs of GfxItems
//
class GfxUndoItem {
  public:
    explicit GfxUndoItem(const GfxItem* item);
    virtual ~GfxUndoItem() = 0;

    virtual GfxItem* create_from_undo_item() const = 0;
    virtual void dump(std::ostream& os) const {};

    friend std::ostream& operator<<(std::ostream& os,
                                    const GfxUndoItem& item);
  public:
    int type;
    int id;
};

#endif
