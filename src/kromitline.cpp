
#include "kromitline.h"

#include <QBrush>
#include <QDebug>
#include <QLineF>
#include <QPainter>

#include <cmath>
#include <iostream>

#include "kromit.h"
#include "utility.h"

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

    setFlag(QGraphicsItem::ItemIsSelectable);
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
    // line must have length > 0, otherwise selecting it will not work
    if (line.length() < 0.01)
        line.setP2(QPointF(line.p1().x() + 0.01, line.p1().y() + 0.01));
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

QPainterPath KromitLine::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_line_item->line().p1());
    p.lineTo(_line_item->line().p2());

    QPainterPathStroker ps;
    ps.setWidth(dist);
    return ps.createStroke(p);
}

QPainterPath KromitLine::shape() const {
    return outline(15);
}

QRectF KromitLine::boundingRect() const {
    if (isSelected()) {
        return outline(20).boundingRect();
    } else {
        return QRectF(0, 0, 0, 0);
    }
};

void KromitLine::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::DashLine));
        painter->drawPath(outline(10));
    }
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

  if (_arrow_type & ARROW_START) { set_arrow_start(p1, _arrow_width, rot); }
  if (_arrow_type & ARROW_END) { set_arrow_start(p2, _arrow_width, rot); }
}

