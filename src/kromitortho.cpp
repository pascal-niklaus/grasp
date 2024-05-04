#include "kromitortho.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>

// KromitOrtho::KromitOrtho
//
KromitOrtho::KromitOrtho(QGraphicsItem* parent,
                         const QColor& color,
                         double width,
                         ArrowType arrowtype,
                         double arrowsize,
                         double simplify,
                         double snap,
                         double radius,
                         double minlen,
                         double maxangle) :
    ArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_size = arrowsize;
    _pen = QPen(color, width, Qt::SolidLine, Qt::RoundCap);
    ArrowItem::setPen(_pen);
    _snap = snap;
    _simplify = simplify;
    _radius = radius;
    _minlen = minlen;
    _maxangle = maxangle;

    setFlag(QGraphicsItem::ItemIsSelectable);
}

KromitOrtho::~KromitOrtho() {}


// KromitOrtho::create_undo_item
//
KromitUndoItem* KromitOrtho::create_undo_item() const {
    auto* undo = new KromitUndoOrtho(this);
    return undo;
}

// KromitOrtho::stash
//
// store pre-transformation state
//
void KromitOrtho::stash() {
  _stashed_xy_path = _xy_path;
}

// KromitOrtho::apply
//
// apply 2D affine transformation
//
void KromitOrtho::apply(const QTransform& m) {
    prepareGeometryChange();
    _xy_path = _stashed_xy_path;
    _xy_path.apply2D(m);

    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);

    repos_arrows();
}

// KromitOrtho::start
//
// start a path
//
void KromitOrtho::start(const QPointF& p, double pressure) {
    _xy_path_raw.clear();
    move_internal(p, true);
}

// KromitOrtho::stop
//
// stop a path
//
void KromitOrtho::stop(const QPointF& p, double pressure) {
    move_internal(p, true);
    _xy_path_raw.clear();
    _raw_path.clear();
}

// KromitOrtho::line_to
//
// extend the path
//
void KromitOrtho::move(const QPointF& p, double pressure) {
    move_internal(p, false);
}

void KromitOrtho::move_internal(const QPointF& p, bool force) {
    if (force || XY(_path.currentPosition()).distance(XY(p)) >
                     cap<double>(sqrt(_pen.widthF()), .9, 5.0)) {
        _xy_path_raw.append(p);
        _xy_path_raw.backtrack(15, 10, 5);

        _xy_path = _xy_path_raw;

        if (_simplify > 0.0) {
            _xy_path.douglas_peucker(_simplify);
            bool snapped = false;
            if (_snap > 0.1) snapped = _xy_path.snap_ends(_snap);
            _xy_path.orthogonalize(_maxangle, _minlen);
            _xy_path.round_corners(_radius, 6, snapped);
        }

        prepareGeometryChange();
        _path.clear();
        _path.moveTo(_xy_path.first());
        for (int i = 1; i < _xy_path.size(); i++) _path.lineTo(_xy_path[i]);
        repos_arrows();

        _raw_path.clear();
        _raw_path.moveTo(_xy_path_raw.first());
        for (int i = 1; i < _xy_path_raw.size(); i++) _raw_path.lineTo(_xy_path_raw[i]);

    }
}

// KromitOrtho::outline
//
// create a path outlining the path at given distance
//
QPainterPath KromitOrtho::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_xy_path.first());
    for (const auto& pt : _xy_path.path()) p.lineTo(pt);

    QPainterPathStroker ps;
    ps.setWidth(_pen.widthF() + dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(p).simplified();
}

// KromitOrtho::shape
//
// selectable area
//
QPainterPath KromitOrtho::shape() const {
    return outline(20);
}

// KromitOrtho::repos_arrows
//
// reposition arrow heads according to path
//
void KromitOrtho::repos_arrows() {
    // search radius for angle
    const double radius = cap<double>(_pen.widthF() * 6.0, 10.0, 50.0);
    const double arrow_width = _arrow_size * _pen.widthF();
    if (_arrow_type & ARROW_START) {
        double rot = _xy_path.end_angle(radius, true);
        set_arrow_start(_xy_path.first(), arrow_width, rot);
    }
    if (_arrow_type & ARROW_END) {
        double rot = _xy_path.end_angle(radius, false);
        set_arrow_end(_xy_path.last(), arrow_width, rot);
    }
}

// KromitOrtho::boundingRect
//
// this area is re-painted
//
QRectF KromitOrtho::boundingRect() const {
    double delta = _pen.width() / 2.0 + 25;
    QRectF rect = _path.boundingRect().united(_raw_path.boundingRect());
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// KromitOrtho::paint
//
void KromitOrtho::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawPath(outline(15));
    }
    painter->setPen(_pen);
    painter->drawPath(_path);

    painter->setPen(QPen(_pen.color(),1, Qt::DashLine));
    painter->drawPath(_raw_path);

}

// KromitOrtho::set_smooth
//
// Set the path, recreating PainterSmooth
//
void KromitOrtho::set_path(const Path& path) {
    prepareGeometryChange();
    _xy_path = path;
    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);
    repos_arrows();
}

// KromitOrtho::dump
//
void KromitOrtho::dump(std::ostream& os) const {
    os << string_format(
        " (%.0f,%.0f)", _xy_path.first().x(), _xy_path.first().y());
    os << _arrow_type;
    os << string_format(
        "(%.0f,%.0f)", _xy_path.last().x(), _xy_path.last().y());
}

// ============================== undo ===============================

// KromitUndoOrtho::KromitUndoOrtho
//
KromitUndoOrtho::KromitUndoOrtho(const KromitOrtho* kortho) :
    KromitUndoItem(static_cast<const KromitItem*>(kortho)) {
    pen = kortho->pen();
    arrow_type = kortho->arrow_type();
    arrow_size = kortho->arrow_size();
    xy_path = kortho->path();
}

// KromitUndoOrtho::create_from_undo_item
//
KromitItem* KromitUndoOrtho::create_from_undo_item() const {
    KromitOrtho* kortho = new KromitOrtho(nullptr,
                                          pen.color(),
                                          pen.widthF(),
                                          arrow_type,
                                          arrow_size,
                                          10,
                                          0,
                                          10,
                                          10,
                                          10);
    kortho->setData(KromitOrtho::KeyID, id);
    kortho->set_path(xy_path);
    return kortho;
}
