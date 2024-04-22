#ifndef KROMITARROW_H
#define KROMITARROW_H

#include "kromit.h"

#include <QGraphicsItem>
#include <QPen>


// KromitArrow
//
// this arrow has a width of 1 unit
//
// use setPos, setScale, and setRotation to position it
// setPen to set the color

class KromitArrow : public QGraphicsPolygonItem {
  public:
    enum { Type = static_cast<int>(UserType) + GFX_ARROW };

  public:
    KromitArrow(QGraphicsItem *parent = nullptr);
    ~KromitArrow();

    virtual int type() const override { return Type; }
};

#endif  // KROMITARROW_H
