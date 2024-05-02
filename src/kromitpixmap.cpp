#include "kromitpixmap.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>


// KromitPixmap::KromitPixmap
//
KromitPixmap::KromitPixmap(QGraphicsItem* parent,
                           const QColor& color,
                           double width) :
    KromitItem(parent) {
    _pen = QPen(color, width, Qt::DashLine, Qt::RoundCap);
    setFlag(QGraphicsItem::ItemIsSelectable);
    drag_frame = false;
    setZValue(-10.0);
}

KromitPixmap::~KromitPixmap() {}

// KromitPixmap::create_undo_item
//
KromitUndoItem* KromitPixmap::create_undo_item() const {
    auto* undo = new KromitUndoPixmap(this);
    return undo;
}

// KromitPixmap::stash
//
// store pre-transformation state
//
void KromitPixmap::stash() {
    _stashed_rect = _sel_rect;
}

// KromitPixmap::apply
//
// apply 2D affine transformation
//
void KromitPixmap::apply(const QTransform& m) {
    prepareGeometryChange();
    _sel_rect = m.mapRect(_stashed_rect);
}

// KromitPixmap::start
//
// start a path
//
void KromitPixmap::start(const QPointF& p, double pressure) {
    _sel_rect.setTopLeft(p);
    drag_frame = true;
    move(p);
}

// KromitPixmap::stop
//
// stop a path
//
void KromitPixmap::stop(const QPointF& p, double pressure) {
    drag_frame = false;
    move(p);
}

// KromitPixmap::move
//
// drag rectangle corner
//
void KromitPixmap::move(const QPointF& p, double pressure) {
    prepareGeometryChange();
    _sel_rect.setBottomRight(p);
}

// KromitPixmap::boundingRect
//
// this area is re-painted
//
QRectF KromitPixmap::boundingRect() const {
    double delta = _pen.width() / 2.0 + 15;
    QRectF rect = _sel_rect.normalized();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// KromitPixmap::paint
//
void KromitPixmap::paint(QPainter* painter,
                         const QStyleOptionGraphicsItem* option,
                         QWidget* widget) {
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawRect(_sel_rect.adjusted(-5, -5, 5, 5));
    }
    if (drag_frame) {
        painter->setPen(_pen);
        painter->drawRect(_sel_rect);
    }
    if (!_pixmap.isNull()) {
        painter->drawPixmap(_sel_rect, _pixmap, _pixmap.rect());
    }
}

// KromitPixmap::set_pixmap
//
// Set the pixmap and it's place
//
void KromitPixmap::set_pixmap(const QPixmap& image, const QRectF& place) {
    prepareGeometryChange();
    _sel_rect = place;
    _pixmap = image;
}

// KromitPixmap::dump
//
void KromitPixmap::dump(std::ostream& os) const {
    os << string_format(" (%.0f,%.0f) to (%.0f,%.0f)",
                        _sel_rect.left(),
                        _sel_rect.top(),
                        _sel_rect.right(),
                        _sel_rect.bottom());
}

// ============================== undo ===============================

// KromitUndoPixmap::KromitUndoPixmap
//
KromitUndoPixmap::KromitUndoPixmap(const KromitPixmap* kpixmap) :
    KromitUndoItem(static_cast<const KromitItem*>(kpixmap)) {
    _rect = kpixmap->sel_rect();
    _pixmap = kpixmap->pixmap();
}

// KromitUndoPixmap::create_from_undo_item
//
KromitItem* KromitUndoPixmap::create_from_undo_item() const {
    KromitPixmap* krect = new KromitPixmap(nullptr);
    krect->setData(KromitPixmap::KeyID, id);
    krect->set_pixmap(_pixmap, _rect);
    krect->setZValue(-10.0);
    return krect;
}
