#ifndef KROMITLINEH_H
#define KROMITLINEH_H

#include <QGraphicsItem>
#include <QPointF>
#include <QPen>
#include <QColor>

#include "kromit.h"
#include "arrowitem.h"
#include "path.h"

// KromitLine
//
class KromitLine : public ArrowItem {
  public:
    enum { Type = static_cast<int>(UserType) + GFX_LINE };

  public:
    KromitLine(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double width = 1,
               ArrowType atype = ARROW_NONE,
               double arrowsize = 0.0);
    ~KromitLine();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const { return QRectF(0, 0, 0, 0); }

    virtual void start(const QPointF& p, double pressure = 1.0) override;
    virtual void move(const QPointF& p, double pressure = 1.0) override;
    virtual void stop(const QPointF& p, double pressure = 1.0) override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr){};

  private:
    void repos_arrows();

  private:
    QPen _pen;
    ArrowType _arrow_type = ARROW_NONE;
    double _arrow_width = 0.0;
    QGraphicsLineItem* _line_item = nullptr;
};

#endif  // KROMITLINEH_H
