#include "kromithighlight.h"
#include "utility.h"

#include <chrono>

#include <QDebug>
#include <QPainter>


// KromitHighlight::KromitHighlight
//
KromitHighlight::KromitHighlight(QGraphicsItem* parent,
                         const QColor& color,
                         double width,
                         ArrowType arrowtype,
                         double arrowsize,
                         double sustain,
                         double fade) :
  KromitPath(parent, color, width * 4, ARROW_NONE, 0.0) {
    _sustain_secs = sustain;
    _fading_secs = fade;
    update_time();
    setOpacity(0.5);
}

KromitHighlight::~KromitHighlight() {}

// KromitHighlight::create_undo_item
//
KromitUndoItem* KromitHighlight::create_undo_item() const {
    auto* undo = new KromitUndoHighlight(this);
    return undo;
}

// KromitHighlight::start
//
// start a path
//
void KromitHighlight::start(const QPointF& p, double pressure) {
    KromitPath::start(p, pressure);
    update_time();
}

// KromitHighlight::stop
//
// stop a path
//
void KromitHighlight::stop(const QPointF& p, double pressure) {
    KromitPath::stop(p, pressure);
    update_time();
}

// KromitHighlight::move
//
// extend the path
//
void KromitHighlight::move(const QPointF& p, double pressure) {
    KromitPath::move(p, pressure);
    update_time();
}

void KromitHighlight::update_time() {
    std::chrono::milliseconds sust(static_cast<int>(_sustain_secs * 1000));
    std::chrono::milliseconds fade(static_cast<int>(_fading_secs * 1000));
    _sustained = std::chrono::steady_clock::now() + sust;
    _faded = std::chrono::steady_clock::now() + sust + fade;
}

KromitItem::DynResult KromitHighlight::dynamic() {
    const auto now = std::chrono::steady_clock::now();
    if (now > _faded) return KromitItem::DYN_DELITEM;
    if (now < _sustained) return KromitItem::DYN_CONTINUE;

    std::chrono::duration<double, std::milli> elapsed = now - _sustained;
    std::chrono::duration<double, std::milli> delta = _faded - _sustained;

    const double alpha =
        cap<double>(1.0 - elapsed.count() / delta.count(), 0.0, 1.0);
    setOpacity(0.5*alpha);
    return KromitItem::DYN_CONTINUE;
}

// KromitHighlight::dump
//
void KromitHighlight::dump(std::ostream& os) const {
    KromitPath::dump(os);
}

// ============================== undo ===============================

// KromitUndoHighlight::KromitUndoHighlight
//
KromitUndoHighlight::KromitUndoHighlight(const KromitHighlight* kpath) :
    KromitUndoItem(static_cast<const KromitItem*>(kpath)) {
    pen = kpath->pen();
    arrow_type = kpath->arrow_type();
    arrow_size = kpath->arrow_size();
    xy_path = kpath->path();
    faded = kpath->faded();
    sustained = kpath->sustained();
}

// KromitUndoHighlight::create_from_undo_item
//
KromitItem* KromitUndoHighlight::create_from_undo_item() const {
    KromitHighlight* kfader = new KromitHighlight(
        nullptr, pen.color(), pen.widthF(), arrow_type, arrow_size);
    kfader->setData(KromitHighlight::KeyID, id);
    kfader->set_path(xy_path);
    kfader->set_faded(faded);
    kfader->set_sustained(sustained);
    return kfader;
}
