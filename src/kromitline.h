#ifndef KROMITLINEH_H
#define KROMITLINEH_H

#include "arrowitem.h"
#include "kromit.h"
#include "path.h"

#include <QLine>
#include <QPen>

// KromitLine
//
class KromitLine : public ArrowItem {
  public:
    enum { Type = GFX_LINE };

  public:
    KromitLine(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double width = 1,
               ArrowType atype = ARROW_NONE,
               double arrowsize = 0.0);
    ~KromitLine();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const;

    virtual QRectF selectionBox() const override {
        return QRectF(_line_item->line().p1(), _line_item->line().p2());
    }
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_ROTATE |
                CAP_DELETE);
    }

    virtual void stash() override;
    virtual void apply(const QTransform& m) override;
    virtual void start(const QPointF& p, double pressure = 1.0) override;
    virtual void move(const QPointF& p, double pressure = 1.0) override;
    virtual void stop(const QPointF& p, double pressure = 1.0) override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr);

    QPainterPath outline(double dist) const;
    virtual QPainterPath shape() const override;

  private:
    void repos_arrows();

  private:
    QPen _pen;
    ArrowType _arrow_type = ARROW_NONE;
    double _arrow_width = 0.0;
    QGraphicsLineItem* _line_item = nullptr;
    QLineF _stashed_line;
};

#endif  // KROMITLINEH_H
