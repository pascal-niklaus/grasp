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

#include "gfxrotate.h"
#include <qnamespace.h>
#include <qpainterpath.h>
#include <qpen.h>

#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>

#include <iostream>

// GfxRotHandle::GfxRotHandle
//
// the handle is top right, and origin is the corner
//
GfxRotHandle::GfxRotHandle(QGraphicsItem* parent) :
    QGraphicsPathItem(parent) {
    QPainterPath path;
    path.moveTo(r1, 0);
    path.arcTo(-r1, -r1, 2 * r1, 2 * r1, 0, 90);
    path.lineTo(0, -r1 + aside);
    path.lineTo(-alen, -(r1 + r2) / 2);
    path.lineTo(0, -r2 - aside);
    path.arcTo(-r2, -r2, 2*r2, 2*r2, 90, -90);
    path.lineTo(r2+aside, 0);
    path.lineTo((r1 + r2)/2, alen);
    path.lineTo(r1-aside, 0);
    path.closeSubpath();

    prepareGeometryChange();
    setPath(path);
}

GfxRotHandle::~GfxRotHandle() {}
