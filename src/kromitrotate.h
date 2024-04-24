#ifndef KROMITROTATE_H
#define KROMITROTATE_H

#include <qgraphicsitem.h>
#include "kromit.h"

#include <QGraphicsItem>
#include <QPen>

// KromitRotHandle
//
// square handle with side length 1.0
//
// use setPos and setScale to position it, and setPen to set the color
//
class KromitRotHandle : public QGraphicsPathItem {
  public:
    enum { Type = GFX_ROTATEHANDLE };
private:
  const int r1 = 20;
  const int r2 = 30;
  const int alen = 10;
  const int aside = 5;

  public:
    KromitRotHandle(QGraphicsItem *parent = nullptr);
    ~KromitRotHandle();

    virtual int type() const override { return Type; }
};

#endif  // KROMITROTATE_H
