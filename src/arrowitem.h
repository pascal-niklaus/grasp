#ifndef ARROWITEM_H
#define ARROWITEM_H

#include <QGraphicsItem>
#include <QPointF>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>

#include "kromit.h"
#include "kromitarrow.h"

// ArrowItem
//
class ArrowItem : public QGraphicsItem {
  public:
    enum { Type = static_cast<int>(UserType) + GFX_ARROWITEM };

  public:
    ArrowItem(QGraphicsItem* parent = nullptr);
    ~ArrowItem();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

    void setPen(const QPen& pen);

    void set_arrow_start(const QPointF& pos, double size, double rotation);
    void set_arrow_end(const QPointF& pos, double size, double rotation);

  private:
    //  void repos_arrows();

  private:
    KromitArrow* _arrow_start;
    KromitArrow* _arrow_end;
};

#endif  // ARROWITEM_H
