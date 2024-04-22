#include "kromitpath.h"
#include <qpoint.h>

#include <QBrush>
#include <QGraphicsItem>

#include "kromit.h"
#include "utility.h"
#include "arrowitem.h"
#include "kromitarrow.h"
#include "xy.h"

#include <QDebug>
#include <QPainter>

#include <iostream>
#include <cmath>

KromitPath::KromitPath(QGraphicsItem* parent,
                       const QColor& color,
                       double width,
                       ArrowType arrowtype,
                       double arrowsize) :
    ArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_width = arrowsize * width;
    _pen = QPen(color, width);
    ArrowItem::setPen(_pen);
}

KromitPath::~KromitPath() {}

// KromitPath::start
//
// start a path
//
void KromitPath::start(const QPointF& p, double pressure) {
    prepareGeometryChange();
    _path.clear();
    _xy_path.clear();
    _path.moveTo(p);
    _xy_path.append(p);
    repos_arrows();
}

// KromitPath::stop
//
// stop a path
//
void KromitPath::stop(const QPointF& p, double pressure) {
    prepareGeometryChange();
    _path.lineTo(p);
    _xy_path.append(p);

    // simplify the path TODO: make epsilon dependent on width ?
    _xy_path.douglas_peucker(0.9);

    int oldn = _path.elementCount();

    prepareGeometryChange();

    repos_arrows();

    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);

    qDebug() << "path simplified from" << oldn << "to" << _path.elementCount()
             << "points";
}

// KromitPath::line_to
//
// extend the path
//
void KromitPath::move(const QPointF& p, double pressure) {
    if (XY(_path.currentPosition()).distance(XY(p)) > .5) {
        prepareGeometryChange();
        _path.lineTo(p);
        _xy_path.append(p);
        repos_arrows();
    }
}

// KromitPath::repos_arrows
//
// reposition arrow heads according to path
//
void KromitPath::repos_arrows() {
    // search radius for angle
    const double radius = cap<double>(_pen.widthF() * 10.0, 10.0, 30.0);
    if (_arrow_type & ARROW_START) {
        double rot = _xy_path.end_angle(radius, true);
        set_arrow_start(_xy_path.first(), _arrow_width, rot);
    }
    if (_arrow_type & ARROW_END) {
        double rot = _xy_path.end_angle(radius, false) + M_PI;
        set_arrow_end(_xy_path.last(), _arrow_width, rot);
    }
}

// TODO: adjust for arrow heads !
QRectF KromitPath::boundingRect() const {
    double delta = _pen.width() / 2.0 + 5;
    QRectF rect = _path.boundingRect();
    QDebug deb = qDebug();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

void KromitPath::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    painter->setPen(_pen);
    painter->drawPath(_path);
}
