#ifndef KROMITHANDLE_H
#define KROMITHANDLE_H

#include "kromit.h"

#include <QGraphicsItem>
#include <QPen>

// KromitHandle
//
// square handle with side length 1.0
//
// use setPos and setScale to position it, and setPen to set the color
//
class KromitHandle : public QGraphicsPolygonItem {
  public:
    enum { Type = GFX_HANDLE };

  private:
    const int side = 15;

  public:
    KromitHandle(QGraphicsItem *parent = nullptr);
    ~KromitHandle();

    virtual int type() const override { return Type; }
};

#endif  // KROMITHANDLE_H
