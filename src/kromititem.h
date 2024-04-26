#ifndef KROMITITEM_H
#define KROMITITEM_H

#include "kromit.h"

#include <QTransform>

class QPointF;

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

    bool select();
    bool deselect();
    bool toggleselect();

    virtual TransformCaps caps() const = 0;
    virtual QRectF selectionBox() const = 0;

    static QString type(int item);
};

#endif
