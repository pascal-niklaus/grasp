
#include <qpoint.h>
#include <QBrush>
#include <QGraphicsItem>
#include <QLineF>
#include <QDebug>
#include <QPainter>

#include "kromit.h"
#include "kromitline.h"
#include "utility.h"
#include "arrowitem.h"
#include "kromitarrow.h"
#include "xy.h"

#include <iostream>
#include <cmath>

KromitLine::KromitLine(QGraphicsItem* parent,
                       const QColor& color,
                       double width,
                       ArrowType arrowtype,
                       double arrowsize) :
    ArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_width = arrowsize * width;
    _pen = QPen(color, width);
    ArrowItem::setPen(_pen);
    _line_item = new QGraphicsLineItem(this);
    _line_item->setPen(_pen);
}

KromitLine::~KromitLine() {}

// KromitLine::start
//
// start a path
//
void KromitLine::start(const QPointF& p, double pressure) {
    prepareGeometryChange();
    _line_item->setLine(QLineF(p, p));
    repos_arrows();
}

// KromitLine::stop
//
// stop a path
//
void KromitLine::stop(const QPointF& p, double pressure) {
    prepareGeometryChange();
    QLineF line = _line_item->line();
    line.setP2(p);
    _line_item->setLine(line);
    repos_arrows();
}

// KromitLine::move
//
// move end of path
//
void KromitLine::move(const QPointF& p, double pressure) {
    stop(p);
}

// KromitLine::repos_arrows
//
// reposition arrow heads according to path
//
void KromitLine::repos_arrows() {
    // search radius for angle
  const QPointF& p1 = _line_item->line().p1();
  const QPointF& p2 = _line_item->line().p2();
  const double rot = atan2(p2.y() - p1.y(), p2.x() - p1.x());

  qDebug() << "rot =" << rot;
  if (_arrow_type & ARROW_START) { set_arrow_start(p1, _arrow_width, rot); }
  if (_arrow_type & ARROW_END) { set_arrow_start(p2, _arrow_width, rot); }
}

