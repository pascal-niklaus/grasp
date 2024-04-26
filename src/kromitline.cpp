
#include "kromitline.h"

#include <QBrush>
#include <QDebug>
#include <QLineF>
#include <QPainter>
#include <cmath>
#include <iostream>

#include "arrowitem.h"
#include "kromit.h"
#include "kromititem.h"
#include "string_format.h"
#include "tools.h"
#include "utility.h"

KromitLine::KromitLine(QGraphicsItem* parent,
                       const QColor& color,
                       double width,
                       ArrowType arrowtype,
                       double arrowsize) :
    ArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_size = arrowsize;
    //    _arrow_width = arrowsize * width;
    _pen = QPen(color, width);
    ArrowItem::setPen(_pen);
    _line_item = new QGraphicsLineItem(this);
    _line_item->setPen(_pen);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

KromitLine::~KromitLine() {}

// KromitLine::create_undo_item
//
// create an undo record that allows recreatine the line
//
KromitUndoItem* KromitLine::create_undo_item() const {
    auto* undo = new KromitUndoLine(this);
    return undo;
}

// KromitLine::stash
//
// store pre-transformation state
//
void KromitLine::stash() {
    _stashed_line = _line_item->line();
}

// KromitLine::apply
//
// apply 2D affine transformation
//
void KromitLine::apply(const QTransform& m) {
    prepareGeometryChange();
    _line_item->setLine(m.map(_stashed_line));
    repos_arrows();
}

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

// KromitLine::outline
//
// create a path outlining the path at given distance
//
QPainterPath KromitLine::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_line_item->line().p1());
    p.lineTo(_line_item->line().p2());

    QPainterPathStroker ps;
    ps.setWidth(dist);
    return ps.createStroke(p);
}

// KromitLine::shape
//
// selectable area
//
QPainterPath KromitLine::shape() const {
    return outline(15);
}

// KromitLine::boundingRect
//
// this area is re-painted
//
QRectF KromitLine::boundingRect() const {
    if (isSelected()) {
        return outline(20).boundingRect();
    } else {
        return QRectF(0, 0, 0, 0);
    }
};

// KromitLine::paint
//
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
    const double arrow_width = _pen.widthF() * _arrow_size;
    if (_arrow_type & ARROW_START) { set_arrow_start(p1, arrow_width, rot); }
    if (_arrow_type & ARROW_END) { set_arrow_start(p2, arrow_width, rot); }
}

// KromitLine::dump
//
void KromitLine::dump(std::ostream& os) const {
    os << string_format(" (%.0f,%.0f)",
                        _line_item->line().p1().x(),
                        _line_item->line().p1().y());
    os << _arrow_type;
    os << string_format("(%.0f,%.0f)",
                        _line_item->line().p2().x(),
                        _line_item->line().p2().y());
}

// ============================== undo ===============================

// KromitUndoLine::KromitUndoLine
//
// create undo record for line
//
KromitUndoLine::KromitUndoLine(const KromitLine* kline) :
    KromitUndoItem(static_cast<const KromitItem*>(kline)) {
    pen = kline->pen();
    arrow_type = kline->arrow_type();
    arrow_size = kline->arrow_size();
    line = kline->line();
}

// KromitUndoLine::create_from_undo_item
//
// re-create the KromitLine
//
KromitItem* KromitUndoLine::create_from_undo_item() const {
    KromitLine* kline = new KromitLine(
        nullptr, pen.color(), pen.widthF(), arrow_type, arrow_size);
    kline->setData(KromitLine::KeyID, id);
    kline->start(line.p1());
    kline->stop(line.p2());
    return kline;
}
