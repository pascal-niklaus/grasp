#ifndef ARROWITEM_H
#define ARROWITEM_H

#include "kromit.h"
#include "kromititem.h"

class KromitArrow;

// ArrowItem
//
class ArrowItem : public KromitItem {
  public:
    enum { Type = GFX_ARROWITEM };

  public:
    ArrowItem(QGraphicsItem* parent = nullptr);
    ~ArrowItem();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

    virtual QRectF selectionBox() const override { return QRectF(); }
    virtual TransformCaps caps() const override { return CAP_NONE; }

    void setPen(const QPen& pen);

    void set_arrow_start(const QPointF& pos, double size, double rotation);
    void set_arrow_end(const QPointF& pos, double size, double rotation);

  private:
    KromitArrow* _arrow_start;
    KromitArrow* _arrow_end;
};

std::ostream& operator<<(std::ostream& os, const ArrowType& a);


#endif  // ARROWITEM_H
