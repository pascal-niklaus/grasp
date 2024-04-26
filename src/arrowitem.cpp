#include "kromit.h"
#include "kromititem.h"
#include "kromitarrow.h"
#include "arrowitem.h"

#include "xy.h"

#include <QBrush>
#include <QPainter>


// ArrowItem::ArrowItem
//
// create item with two arrow heads; the heads are not shown until
// position is set
//
ArrowItem::ArrowItem(QGraphicsItem* parent) : KromitItem(parent) {
    prepareGeometryChange();
    _arrow_start = new KromitArrow(this);
    _arrow_start->hide();
    _arrow_end = new KromitArrow(this);
    _arrow_end->hide();
}

ArrowItem::~ArrowItem() {
    delete _arrow_start;
    delete _arrow_end;
}

// ArrowItem::boundingRect
//
// return nothing since nothing is drawn here
//
QRectF ArrowItem::boundingRect() const {
    return QRectF(0, 0, 0, 0);
}

// ArrowItem::paint
//
// paint nothing
//
void ArrowItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget) {}

// ArrowItem::setPen
//
// set the pen (basically the color)
//
void ArrowItem::setPen(const QPen &pen) {
    QPen p = pen;
    p.setWidth(0);
    _arrow_start->setPen(p);
    _arrow_start->setBrush(p.color());
    _arrow_end->setPen(p);
    _arrow_end->setBrush(p.color());
}

// ArrowItem::set_arrow_<start|end>
//
// display the arrow; a width <= 0 indicates the arrow is hidded
//
void ArrowItem::set_arrow_start(const QPointF &pos,
                                double size,
                                double rotation) {
    if (size > 0.0) {
        _arrow_start->setPos(pos);
        _arrow_start->setScale(size);
        _arrow_start->setRotation(rotation * 180 / M_PI);
        _arrow_start->show();
    } else {
        _arrow_start->hide();
    }
}

void ArrowItem::set_arrow_end(const QPointF &pos,
                              double size,
                              double rotation) {
    if (size > 0.0) {
        _arrow_end->setPos(pos);
        _arrow_end->setScale(size);
        _arrow_end->setRotation(rotation * 180.0 / M_PI);
        _arrow_end->show();
    } else {
        _arrow_end->hide();
    }
}

std::ostream& operator<<(std::ostream& os, const ArrowType& a) {
    switch (a) {
    case ARROW_START:
        os << "<--";
        break;
    case ARROW_END:
        os << "-->";
        break;
    case ARROW_DOUBLE:
        os << "<->";
        break;
    case ARROW_NONE:
      os << "---";
        break;
    }
    return os;
}

