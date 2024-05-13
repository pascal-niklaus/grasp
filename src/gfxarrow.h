#ifndef GFXARROW_H
#define GFXARROW_H

#include "grasp.h"

#include <QGraphicsItem>
#include <QPen>


// GfxArrow
//
// this arrow has a width of 1 unit
//
// use setPos, setScale, and setRotation to position it
// setPen to set the color

class GfxArrow : public QGraphicsPolygonItem {
  public:
    enum { Type = GFX_ARROW };

  public:
    GfxArrow(QGraphicsItem *parent = nullptr);
    ~GfxArrow();

    virtual int type() const override { return Type; }
};

#endif  // GFXARROW_H
