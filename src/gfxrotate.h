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

#ifndef GFXROTATE_H
#define GFXROTATE_H

#include <qgraphicsitem.h>
#include "grasp.h"

#include <QGraphicsItem>
#include <QPen>

// GfxRotHandle
//
// square handle with side length 1.0
//
// use setPos and setScale to position it, and setPen to set the color
//
class GfxRotHandle : public QGraphicsPathItem {
  public:
    enum { Type = GFX_ROTATEHANDLE };

  private:
    const int r1 = 20;
    const int r2 = 30;
    const int alen = 10;
    const int aside = 5;

  public:
    GfxRotHandle(QGraphicsItem *parent = nullptr);
    ~GfxRotHandle();

    virtual int type() const override { return Type; }
};

#endif  // GFXROTATE_H
