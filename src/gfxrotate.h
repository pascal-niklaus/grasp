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
