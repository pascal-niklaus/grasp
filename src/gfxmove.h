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

#ifndef GFXMOVE_H
#define GFXMOVE_H

#include <qgraphicsitem.h>

#include <QDebug>
#include <QGraphicsItem>
#include <QPen>

#include "grasp.h"

// GfxMove
//
// square handle with side length 1.0
//
// use setPos and setScale to position it, and setPen to set the color
//
class GfxMove : public QGraphicsPolygonItem {
  public:
    enum { Type = GFX_MOVEHANDLE };

  private:
    const int len = 15;
    const int width = 5;  // half the width of the arrow shaft
    const int tiplen = 10;
    const int tipside = 5;

  public:
    GfxMove(QGraphicsItem *parent = nullptr);
    ~GfxMove();

    virtual int type() const override { return Type; }
};

#endif  // GFXMOVE_H
