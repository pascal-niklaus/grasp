#ifndef KROMITPATH_H
#define KROMITPATH_H

#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>

class KromitPath : public QGraphicsPathItem
{
//    Q_OBJECT
public:
    KromitPath();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option=0, QWidget *widget=0);

private:
    QPainterPath original_path;

    QPen pen;
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
