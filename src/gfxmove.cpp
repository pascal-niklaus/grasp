#include <qpoint.h>
#include "gfxmove.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>
#include <iostream>

GfxMove::GfxMove(QGraphicsItem* parent) : QGraphicsPolygonItem(parent) {
    QPolygonF poly;
    poly << QPointF(-width, len)             // bottom tip
         << QPointF(-width - tipside, len)   // bottom tip
         << QPointF(0, len + tiplen)         // bottom tip
         << QPointF(width + tipside, len)    //
         << QPointF(width, len)              //

         << QPointF(width, width)            //

         << QPointF(len, width)              //
         << QPointF(len, width + tipside)    //
         << QPointF(len + tiplen, 0)         // right tip
         << QPointF(len, -width - tipside)   //
         << QPointF(len, -width)             //

         << QPointF(width, -width)           //

         << QPointF(width, -len)             // top tip
         << QPointF(width + tipside, -len)   // top tip
         << QPointF(0, -len - tiplen)        // top tip
         << QPointF(-width - tipside, -len)  // top tip
         << QPointF(-width, -len)            // top tip

         << QPointF(-width, -width)          //

         << QPointF(-len, -width)            //
         << QPointF(-len, -width - tipside)  //
         << QPointF(-len - tiplen, 0)        // left tip
         << QPointF(-len, width + tipside)   //
         << QPointF(-len, width)             //

         << QPointF(-width, width)           //
        ;

    prepareGeometryChange();
    setPolygon(poly);
}

GfxMove::~GfxMove() {}
