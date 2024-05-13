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
