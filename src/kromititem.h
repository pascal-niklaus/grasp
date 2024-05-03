#ifndef KROMITITEM_H
#define KROMITITEM_H

#include "kromit.h"
#include "tools.h"

#include <QTransform>

class QPointF;
class KromitUndoItem;

// KromitItem
//
// abstract base class for customized QGraphcisItem that "builds"
// itself when receiving mouse events
//
class KromitItem : public QGraphicsItem {
  public:
    enum { KeyID = 1, KeyHandle = 1 };
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

    KromitItem(QGraphicsItem* parent) : QGraphicsItem(parent) {}
    ~KromitItem() {}

    // - undo system
    virtual KromitUndoItem* create_undo_item() const = 0;

    // - transform system
    virtual void stash() = 0;
    virtual void apply(const QTransform& m) = 0;
    virtual QRectF selectionBox() const = 0;
    virtual TransformCaps caps() const = 0;

    // - build system
    virtual void start(const QPointF& p, double pressure = 1.0) = 0;
    virtual void move(const QPointF& p, double pressure = 1.0) = 0;
    virtual void stop(const QPointF& p, double pressure = 1.0) = 0;
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

    void set_new_ID() { setData(KromitItem::KeyID, Tools::generate_ID()); }

    static QString type_txt(int item);

    friend std::ostream& operator<<(std::ostream& os, const KromitItem& item);
    friend std::ostream& operator<<(std::ostream& os, const KromitItem* item);
};

// KromitUndoItem
//
// abstract base class for parallel "undo" analogs of KromitItems
//
class KromitUndoItem {
  public:
    explicit KromitUndoItem(const KromitItem * item);
    ~KromitUndoItem() = default;

    virtual KromitItem* create_from_undo_item() const = 0;
    virtual void dump(std::ostream& os) const {};

    friend std::ostream& operator<<(std::ostream& os,
                                    const KromitUndoItem& item);
  public:
    int type;
    int id;
};

#endif
