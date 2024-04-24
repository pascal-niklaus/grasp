#ifndef KROMITMOVEE_H
#define KROMITMOVEE_H

#include "kromit.h"

#include <QGraphicsItem>
#include <QPen>

// KromitMove
//
// square handle with side length 1.0
//
// use setPos and setScale to position it, and setPen to set the color
//
class KromitMove : public QGraphicsPolygonItem {
  public:
    enum { Type = GFX_MOVEHANDLE };

  private:
    const int len = 15;
    const int width = 5;  // half the width of the arrow shaft
    const int tiplen = 10;
    const int tipside = 5;

  public:
    KromitMove(QGraphicsItem *parent = nullptr);
    ~KromitMove();

    virtual int type() const override { return Type; }
};

#endif  // KROMITMOVEE_H
