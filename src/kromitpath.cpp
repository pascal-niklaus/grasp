#include "kromitpath.h"
#include <qpoint.h>
#include <qtransform.h>

#include <QBrush>
#include <QGraphicsItem>

#include "kromit.h"
#include "utility.h"
#include "tools.h"
#include "arrowitem.h"
#include "kromitarrow.h"
#include "xy.h"
#include "string_format.h"

#include <QDebug>
#include <QPainter>

#include <iostream>
#include <cmath>

// KromitPath::KromitPath
//
KromitPath::KromitPath(QGraphicsItem* parent,
                       const QColor& color,
                       double width,
                       ArrowType arrowtype,
                       double arrowsize) :
    ArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_size = arrowsize;
    _pen = QPen(color, width);
    ArrowItem::setPen(_pen);

    setFlag(QGraphicsItem::ItemIsSelectable);
}

KromitPath::~KromitPath() {}


// KromitPath::create_undo_item
//
KromitUndoItem* KromitPath::create_undo_item() const {
    auto* undo = new KromitUndoPath(this);
    return undo;
}

// KromitPath::stash
//
// store pre-transformation state
//
void KromitPath::stash() {
  _stashed_xy_path = _xy_path;
}

// KromitPath::apply
//
// apply 2D affine transformation
//
void KromitPath::apply(const QTransform& m) {
    prepareGeometryChange();
    _xy_path = _stashed_xy_path;
    _xy_path.apply2D(m);

    std::cerr << _xy_path << "\n";

    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);

    repos_arrows();
}

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
    _xy_path.douglas_peucker(_pen.widthF() * 0.9);

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

// KromitPath::outline
//
// create a path outlining the path at given distance
//
QPainterPath KromitPath::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_xy_path.first());
    for (const auto& pt : _xy_path.path())
        p.lineTo(pt);

    QPainterPathStroker ps;
    ps.setWidth(dist);
    return ps.createStroke(p);
}

// KromitPath::shape
//
// selectable area
//
QPainterPath KromitPath::shape() const {
    return outline(15);
}

// KromitPath::repos_arrows
//
// reposition arrow heads according to path
//
void KromitPath::repos_arrows() {
    // search radius for angle
    const double radius = cap<double>(_pen.widthF() * 10.0, 10.0, 30.0);
    const double arrow_width = _arrow_size * _pen.widthF();
    if (_arrow_type & ARROW_START) {
        double rot = _xy_path.end_angle(radius, true);
        set_arrow_start(_xy_path.first(), arrow_width, rot);
    }
    if (_arrow_type & ARROW_END) {
        double rot = _xy_path.end_angle(radius, false) + M_PI;
        set_arrow_end(_xy_path.last(), arrow_width, rot);
    }
}

// KromitPath::boundingRect
//
// this area is re-painted
//
QRectF KromitPath::boundingRect() const {
    double delta = _pen.width() / 2.0 + 5;
    QRectF rect = _path.boundingRect().normalized();
    QDebug deb = qDebug();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// KromitPath::paint
//
void KromitPath::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::DashLine));
        painter->drawPath(outline(10));
    }
    painter->setPen(_pen);
    painter->drawPath(_path);
}

// KromitPath::set_path
//
// Set the path, recreating PainterPath
//
void KromitPath::set_path(const Path &path) {
    prepareGeometryChange();
    _xy_path = path;
    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);
    repos_arrows();
}

// KromitPath::dump
//
void KromitPath::dump(std::ostream& os) const {
    os << string_format(
        " (%.0f,%.0f)", _xy_path.first().x(), _xy_path.first().y());
    os << _arrow_type;
    os << string_format(
        "(%.0f,%.0f)", _xy_path.last().x(), _xy_path.last().y());
}

// ============================== undo ===============================

// KromitUndoPath::KromitUndoPath
//
KromitUndoPath::KromitUndoPath(const KromitPath* kpath) :
    KromitUndoItem(static_cast<const KromitItem*>(kpath)) {
    pen = kpath->pen();
    arrow_type = kpath->arrow_type();
    arrow_size = kpath->arrow_size();
    xy_path = kpath->path();
}

// KromitUndoPath::create_from_undo_item
//
KromitItem* KromitUndoPath::create_from_undo_item() const {
    KromitPath* kpath = new KromitPath(
        nullptr, pen.color(), pen.widthF(), arrow_type, arrow_size);
    kpath->setData(KromitPath::KeyID, id);
    kpath->set_path(xy_path);
    return kpath;
}
