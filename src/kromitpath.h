#ifndef KROMITPATH_H
#define KROMITPATH_H

#include <qgraphicsitem.h>
#include <qpainterpath.h>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>

// KromitPath
//
class KromitPath : public QGraphicsItem {
  public:
    enum { Type = UserType + 1 };

  public:
    KromitPath();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

    void start(const QPointF& p);
    void stop(const QPointF& p);
    void line_to(double x, double y);
    void line_to(const QPointF& p);

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

  private:
    QPen pen;
    QPainterPath path;
};

#endif // KROMITPATH_H


/*
 * moveTo : starts a new subpath
 * lineTo:
 * arcTo
 * cubicTo
 * quadTo
 * closeSubpath: line back to start
 *
 *
 * QPainterPath::Element
 *
 * type: CurveTo, LineTo, MoveTo
 */
