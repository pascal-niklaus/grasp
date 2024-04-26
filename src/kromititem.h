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
    KromitItem(QGraphicsItem* parent) : QGraphicsItem(parent) {}
    ~KromitItem() {}

    virtual void stash() = 0;
    virtual void apply(const QTransform& m) = 0;

    virtual void start(const QPointF& p, double pressure = 1.0) = 0;
    virtual void move(const QPointF& p, double pressure = 1.0) = 0;
    virtual void stop(const QPointF& p, double pressure = 1.0) = 0;

    virtual void dump(std::ostream& os) const {};

    virtual KromitUndoItem* create_undo_item() const = 0;

    bool select();
    bool deselect();
    bool toggleselect();

    void set_new_ID() { setData(KromitItem::KeyID, Tools::generate_ID()); }

    virtual TransformCaps caps() const = 0;
    virtual QRectF selectionBox() const = 0;

    static QString type_txt(int item);

    friend std::ostream& operator<<(std::ostream& os, const KromitItem& item);
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
