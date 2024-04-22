#ifndef KROMITITEM_H
#define KROMITITEM_H

#include <QGraphicsItem>
#include <QPointF>

// KromitItem
//
// abstract base class for customized QGraphcisItem that "builds"
// itself when receiving mouse events
//
class KromitItem : public QGraphicsItem {
  public:
    KromitItem(QGraphicsItem* parent) : QGraphicsItem(parent) {}
    ~KromitItem() {}
    virtual void start(const QPointF& p, double pressure = 1.0) = 0;
    virtual void move(const QPointF& p, double pressure = 1.0) = 0;
    virtual void stop(const QPointF& p, double pressure = 1.0) = 0;
};

#endif
