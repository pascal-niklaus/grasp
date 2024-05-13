#ifndef GFXARROWITEM_H
#define GFXARROWITEM_H

#include "gfxitem.h"

class GfxArrow;

// GfxArrowItem
//
class GfxArrowItem : public GfxItem {
  public:
    enum { Type = GFX_ARROWITEM };

  public:
    GfxArrowItem(QGraphicsItem* parent = nullptr);
    ~GfxArrowItem();

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
    GfxArrow* _arrow_start;
    GfxArrow* _arrow_end;
};

std::ostream& operator<<(std::ostream& os, const ArrowType& a);


#endif  // GFXARROWITEM_H
