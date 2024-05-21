/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
