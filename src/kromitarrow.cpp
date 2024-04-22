#include "kromitarrow.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>
#include <iostream>

KromitArrow::KromitArrow(QGraphicsItem* parent) : QGraphicsPolygonItem(parent) {
    QPolygonF poly;
    poly << QPointF(.6, 0)
         << QPointF(-.6, 0.5)
         << QPointF(-0.3, 0)
         << QPointF(-.6, -0.5);

    prepareGeometryChange();
    setPolygon(poly);
}

KromitArrow::~KromitArrow() {}

