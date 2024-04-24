#include "kromithandle.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>
#include <iostream>

KromitHandle::KromitHandle(QGraphicsItem* parent) :
    QGraphicsPolygonItem(parent) {
    QPolygonF poly;
    poly << QPointF(+0.5 * side, +0.5 * side)
         << QPointF(+0.5 * side, -0.5 * side)
         << QPointF(-0.5 * side, -0.5 * side)
         << QPointF(-0.5 * side, +0.5 * side);

    prepareGeometryChange();
    setPolygon(poly);
}

KromitHandle::~KromitHandle() {}
