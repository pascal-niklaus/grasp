#include "kromitpath.h"
#include "xy.h"

#include <qgraphicsitem.h>
#include <qpoint.h>

#include <QDebug>
#include <QPainter>

KromitPath::KromitPath() {
    prepareGeometryChange();
    pen = QPen(Qt::yellow, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}


void KromitPath::start(const QPointF& p) {
    prepareGeometryChange();
    path.moveTo(p);
}

void KromitPath::stop(const QPointF& p) {
    prepareGeometryChange();
    path.lineTo(p);
}

void KromitPath::line_to(double x, double y) {
    if (XY(path.currentPosition()).distance(XY(x, y)) > .5) {
        prepareGeometryChange();
        path.lineTo(x, y);
    }
}

void KromitPath::line_to(const QPointF& p) {
    if (XY(path.currentPosition()).distance(XY(p)) > .5) {
        prepareGeometryChange();
        path.lineTo(p);
    }
}

// TODO: adjust for arrow heads !
QRectF KromitPath::boundingRect() const {
    double delta = pen.width() / 2.0 + 5;
    QRectF rect = path.boundingRect();
    QDebug deb = qDebug();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

void KromitPath::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    painter->save();
    painter->setPen(pen);
    painter->drawPath(path);
    painter->restore();
}
