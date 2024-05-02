#include "kromitrect.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>


// KromitRect::KromitRect
//
KromitRect::KromitRect(QGraphicsItem* parent,
                       const QColor& color,
                       double width,
                       double radius) :
    KromitItem(parent) {
    _radius = radius;
    _pen = QPen(color, width, Qt::SolidLine, Qt::RoundCap);

    setFlag(QGraphicsItem::ItemIsSelectable);
}

KromitRect::~KromitRect() {}


// KromitRect::create_undo_item
//
KromitUndoItem* KromitRect::create_undo_item() const {
    auto* undo = new KromitUndoRect(this);
    return undo;
}

// KromitRect::stash
//
// store pre-transformation state
//
void KromitRect::stash() {
    _stashed_rect = _rect;
}

// KromitRect::apply
//
// apply 2D affine transformation
//
void KromitRect::apply(const QTransform& m) {
    prepareGeometryChange();
    set_rect(m.mapRect(_stashed_rect));
}

// KromitRect::start
//
// start a path
//
void KromitRect::start(const QPointF& p, double pressure) {
    _rect.setTopLeft(p);
    move(p);
}

// KromitRect::stop
//
// stop a path
//
void KromitRect::stop(const QPointF& p, double pressure) {
    move(p);
}

// KromitRect::move
//
// drag rectangle corner
//
void KromitRect::move(const QPointF& p, double pressure) {
    _rect.setBottomRight(p);
    build_path();
}

void KromitRect::build_path() {
    prepareGeometryChange();
    auto nrect = _rect.normalized();
    _path.clear();
    const double r = nrect.width() > 2 * _radius ? _radius : 0.0;
    _path.addRoundedRect(nrect, r, r, Qt::AbsoluteSize);
}

// KromitRect::outline
//
// create a path outlining the path at given distance
//
QPainterPath KromitRect::outline(double dist) const {
    QPainterPathStroker ps;
    ps.setWidth(_pen.widthF() + dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(_path);
}

// KromitRect::shape
//
// selectable area
//
QPainterPath KromitRect::shape() const {
    return outline(20);
}

// KromitRect::boundingRect
//
// this area is re-painted
//
QRectF KromitRect::boundingRect() const {
    double delta = _pen.width() / 2.0 + 25;
    QRectF rect = _rect.normalized();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// KromitRect::paint
//
void KromitRect::paint(QPainter* painter,
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

// KromitRect::set_path
//
// Set the path, recreating PainterPath
//
void KromitRect::set_rect(const QRectF& rect) {
    prepareGeometryChange();
    _rect = rect.normalized();
    build_path();
}

// KromitRect::dump
//
void KromitRect::dump(std::ostream& os) const {
    os << string_format(" (%.0f,%.0f) to (%.0f,%.0f)",
                        _rect.left(),
                        _rect.top(),
                        _rect.right(),
                        _rect.bottom());
}

// ============================== undo ===============================

// KromitUndoRect::KromitUndoRect
//
KromitUndoRect::KromitUndoRect(const KromitRect* krect) :
    KromitUndoItem(static_cast<const KromitItem*>(krect)) {
    pen = krect->pen();
    radius = krect->radius();
    rect = krect->rect();
}

// KromitUndoRect::create_from_undo_item
//
KromitItem* KromitUndoRect::create_from_undo_item() const {
    KromitRect* krect =
        new KromitRect(nullptr, pen.color(), pen.widthF(), radius);
    krect->setData(KromitRect::KeyID, id);
    krect->set_rect(rect);
    return krect;
}
