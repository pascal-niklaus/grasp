#include "kromitfader.h"
#include "string_format.h"
#include "utility.h"

#include <chrono>

#include <QDebug>
#include <QPainter>


// KromitFader::KromitFader
//
KromitFader::KromitFader(QGraphicsItem* parent,
                         const QColor& color,
                         double width,
                         ArrowType arrowtype,
                         double arrowsize,
                         double sustain,
                         double fade) :
  KromitPath(parent, color, width, arrowtype, arrowsize) {
    _sustain_secs = sustain;
    _fading_secs = fade;
    update_time();
}

KromitFader::~KromitFader() {}

// KromitFader::create_undo_item
//
KromitUndoItem* KromitFader::create_undo_item() const {
    auto* undo = new KromitUndoFader(this);
    return undo;
}

// KromitFader::start
//
// start a path
//
void KromitFader::start(const QPointF& p, double pressure) {
    KromitPath::start(p, pressure);
    update_time();
}

// KromitFader::stop
//
// stop a path
//
void KromitFader::stop(const QPointF& p, double pressure) {
    KromitPath::stop(p, pressure);
    update_time();
}

// KromitFader::move
//
// extend the path
//
void KromitFader::move(const QPointF& p, double pressure) {
    KromitPath::move(p, pressure);
    update_time();
}

void KromitFader::update_time() {
    std::chrono::milliseconds sust(static_cast<int>(_sustain_secs * 1000));
    std::chrono::milliseconds fade(static_cast<int>(_fading_secs * 1000));
    _sustained = std::chrono::steady_clock::now() + sust;
    _faded = std::chrono::steady_clock::now() + sust + fade;
}

KromitItem::DynResult KromitFader::dynamic() {
    const auto now = std::chrono::steady_clock::now();
    if (now > _faded) return KromitItem::DYN_DELITEM;
    if (now < _sustained) return KromitItem::DYN_CONTINUE;

    std::chrono::duration<double, std::milli> elapsed = now - _sustained;
    std::chrono::duration<double, std::milli> delta = _faded - _sustained;

    const double alpha =
        cap<double>(1.0 - elapsed.count() / delta.count(), 0.0, 1.0);
    setOpacity(alpha);
    return KromitItem::DYN_CONTINUE;
}

// KromitFader::dump
//
void KromitFader::dump(std::ostream& os) const {
    KromitPath::dump(os);
}

// ============================== undo ===============================

// KromitUndoFader::KromitUndoFader
//
KromitUndoFader::KromitUndoFader(const KromitFader* kpath) :
    KromitUndoItem(static_cast<const KromitItem*>(kpath)) {
    pen = kpath->pen();
    arrow_type = kpath->arrow_type();
    arrow_size = kpath->arrow_size();
    xy_path = kpath->path();
    faded = kpath->faded();
    sustained = kpath->sustained();
}

// KromitUndoFader::create_from_undo_item
//
KromitItem* KromitUndoFader::create_from_undo_item() const {
    KromitFader* kfader = new KromitFader(
        nullptr, pen.color(), pen.widthF(), arrow_type, arrow_size);
    kfader->setData(KromitFader::KeyID, id);
    kfader->set_path(xy_path);
    kfader->set_faded(faded);
    kfader->set_sustained(sustained);
    return kfader;
}
