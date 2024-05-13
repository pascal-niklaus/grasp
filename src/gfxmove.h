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
