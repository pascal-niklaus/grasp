#include "kromitsmooth.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>

// KromitSmooth::KromitSmooth
//
KromitSmooth::KromitSmooth(QGraphicsItem* parent,
                           const QColor& color,
                           double width,
                           ArrowType arrowtype,
                           double arrowsize,
                           double simplify,
                           double snap) :
    ArrowItem(parent) {
    _arrow_type = arrowtype;
    _arrow_size = arrowsize;
    _pen = QPen(color, width, Qt::SolidLine, Qt::RoundCap);
    ArrowItem::setPen(_pen);
    _snap = snap;
    _simplify = simplify;

    setFlag(QGraphicsItem::ItemIsSelectable);
}

KromitSmooth::~KromitSmooth() {}


// KromitSmooth::create_undo_item
//
KromitUndoItem* KromitSmooth::create_undo_item() const {
    auto* undo = new KromitUndoSmooth(this);
    return undo;
}

// KromitSmooth::stash
//
// store pre-transformation state
//
void KromitSmooth::stash() {
  _stashed_xy_path = _xy_path;
}

// KromitSmooth::apply
//
// apply 2D affine transformation
//
void KromitSmooth::apply(const QTransform& m) {
    prepareGeometryChange();
    _xy_path = _stashed_xy_path;
    _xy_path.apply2D(m);

    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);

    repos_arrows();
}

// KromitSmooth::start
//
// start a path
//
void KromitSmooth::start(const QPointF& p, double pressure) {
    _xy_path_raw.clear();
    move_internal(p, true);
}

// KromitSmooth::stop
//
// stop a path
//
void KromitSmooth::stop(const QPointF& p, double pressure) {
    move_internal(p, true);
}

// KromitSmooth::line_to
//
// extend the path
//
void KromitSmooth::move(const QPointF& p, double pressure) {
    move_internal(p, false);
}

void KromitSmooth::move_internal(const QPointF& p, bool force) {
    if (force || XY(_path.currentPosition()).distance(XY(p)) >
                     cap<double>(sqrt(_pen.widthF()), .9, 5.0)) {
        _xy_path_raw.append(p);

        _xy_path = _xy_path_raw;
        if (_simplify > 0.0) {
            _xy_path.douglas_peucker(_simplify);
            bool snapped = false;
            if (_snap > 0.0) snapped = _xy_path.snap_ends(_snap);
            _xy_path.catmull_rom(6, snapped);
        }

        prepareGeometryChange();
        _path.clear();
        _path.moveTo(_xy_path.first());
        for (int i = 1; i < _xy_path.size(); i++) _path.lineTo(_xy_path[i]);
        repos_arrows();
    }
}

// KromitSmooth::outline
//
// create a path outlining the path at given distance
//
QPainterPath KromitSmooth::outline(double dist) const {
    QPainterPath p;
    p.moveTo(_xy_path.first());
    for (const auto& pt : _xy_path.path()) p.lineTo(pt);

    QPainterPathStroker ps;
    ps.setWidth(_pen.widthF() + dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(p).simplified();
}

// KromitSmooth::shape
//
// selectable area
//
QPainterPath KromitSmooth::shape() const {
    return outline(20);
}

// KromitSmooth::repos_arrows
//
// reposition arrow heads according to path
//
void KromitSmooth::repos_arrows() {
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

// KromitSmooth::boundingRect
//
// this area is re-painted
//
QRectF KromitSmooth::boundingRect() const {
    double delta = _pen.width() / 2.0 + 25;
    QRectF rect = _path.boundingRect().normalized();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// KromitSmooth::paint
//
void KromitSmooth::paint(QPainter* painter,
                         const QStyleOptionGraphicsItem* option,
                         QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawPath(outline(15));
    }
    painter->setPen(_pen);
    painter->drawPath(_path);
}

// KromitSmooth::set_smooth
//
// Set the path, recreating PainterSmooth
//
void KromitSmooth::set_path(const Path& path) {
    prepareGeometryChange();
    _xy_path = path;
    _path.clear();
    _path.moveTo(_xy_path[0]);
    const auto size = _xy_path.size();
    for (int i = 0; i < size; i++) _path.lineTo(_xy_path[i]);
    repos_arrows();
}

// KromitSmooth::dump
//
void KromitSmooth::dump(std::ostream& os) const {
    os << string_format(
        " (%.0f,%.0f)", _xy_path.first().x(), _xy_path.first().y());
    os << _arrow_type;
    os << string_format(
        "(%.0f,%.0f)", _xy_path.last().x(), _xy_path.last().y());
}

// ============================== undo ===============================

// KromitUndoSmooth::KromitUndoSmooth
//
KromitUndoSmooth::KromitUndoSmooth(const KromitSmooth* ksmooth) :
    KromitUndoItem(static_cast<const KromitItem*>(ksmooth)) {
    pen = ksmooth->pen();
    arrow_type = ksmooth->arrow_type();
    arrow_size = ksmooth->arrow_size();
    xy_path = ksmooth->path();
    snap = ksmooth->snap();
    simplify = ksmooth->simplify();
}

// KromitUndoSmooth::create_from_undo_item
//
KromitItem* KromitUndoSmooth::create_from_undo_item() const {
    KromitSmooth* ksmooth = new KromitSmooth(nullptr,
                                             pen.color(),
                                             pen.widthF(),
                                             arrow_type,
                                             arrow_size,
                                             simplify,
                                             snap);
    ksmooth->setData(KromitSmooth::KeyID, id);
    ksmooth->set_path(xy_path);
    return ksmooth;
}
