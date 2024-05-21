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

#ifndef GFXHANDLE_H
#define GFXHANDLE_H

#include "grasp.h"

#include <QGraphicsItem>
#include <QPen>

// GfxHandle
//
// square handle with side length 1.0
//
// use setPos and setScale to position it, and setPen to set the color
//
class GfxHandle : public QGraphicsPolygonItem {
  public:
    enum { Type = GFX_HANDLE };

  private:
    const int side = 15;

  public:
    GfxHandle(QGraphicsItem *parent = nullptr);
    ~GfxHandle();

    virtual int type() const override { return Type; }
};

#endif  // GFXHANDLE_H
