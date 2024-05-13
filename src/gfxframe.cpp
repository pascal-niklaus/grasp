#include <qnamespace.h>
#include <qpainterpath.h>
#include <qpen.h>
#include <qpoint.h>
#include <QBrush>
#include <QGraphicsItem>
#include <QDebug>
#include <QPainter>

#include "grasp.h"
#include "gfxframe.h"
#include "gfxmove.h"
#include "gfxrotate.h"
#include "utility.h"

#include <cassert>
#include <iostream>
#include <cmath>

GfxFrame::GfxFrame(QGraphicsItem* parent,
                         const QColor& color,
                         double width) :
    GfxItem(parent) {
    _pen = QPen(color, width);

    _move_handle = new GfxMove(this);
    _move_handle->setPen(_pen);
    _move_handle->setData(KeyHandle, POS_MOVE | POS_CENTER);
    _move_handle->hide();

    prepareGeometryChange();

    for (int i = 0; i < 4; i++) {
        _rot_handle[i] = new GfxRotHandle(this);
        _rot_handle[i]->setPen(_pen);
        _rot_handle[i]->setRotation((i - 1) * 90);
        _rot_handle[i]->setData(KeyHandle, POS_ROTATE + i);
        _rot_handle[i]->hide();
        _iso_handle[i] = new GfxHandle(this);
        _iso_handle[i]->setPen(_pen);
        _iso_handle[i]->setData(KeyHandle, POS_ISOSCALE + i);
        _iso_handle[i]->hide();
        _aniso_handle[i] = new GfxHandle(this);
        _aniso_handle[i]->setPen(_pen);
        _aniso_handle[i]->setData(KeyHandle, POS_ANISOSCALE + i);
        _aniso_handle[i]->hide();
    }
}

GfxFrame::~GfxFrame() {
    delete _move_handle;
    for (int i = 0; i < 4; i++) {
        delete _rot_handle[i];
        delete _iso_handle[i];
        delete _aniso_handle[i];
    }
}

// GfxFrame::setCaps
//
// show handles according to capabilities
//
void GfxFrame::setCaps(TransformCaps caps) {
    prepareGeometryChange();
    if (caps & CAP_MOVE)
        _move_handle->show();
    else
        _move_handle->hide();
    for (int i = 0; i < 4; i++) {
        if (caps & CAP_ROTATE)
            _rot_handle[i]->show();
        else
            _rot_handle[i]->hide();
        if (caps & CAP_ISOSCALE) _iso_handle[i]->show();
        else
            _iso_handle[i]->hide();
        if (caps & CAP_ANISOSCALE) _aniso_handle[i]->show(); else
            _aniso_handle[i]->hide();
    }
}

// GfxFrame::handle_pos
//
// return position of handle
//
QPointF GfxFrame::handle_pos(HandlePos pos) const {
    if (pos & POS_ANISOSCALE) {
        switch (pos & POS_WHICHMASK) {
            case POS_LEFT:
                return midleft();
            case POS_RIGHT:
                return midright();
            case POS_TOP:
                return midtop();
            case POS_BOTTOM:
                return midbottom();
            default:
                assert(false);
        }
    } else {
        switch (pos & POS_WHICHMASK) {
            case POS_TOPLEFT:
                return topleft();
            case POS_TOPRIGHT:
                return topright();
            case POS_BOTTOMLEFT:
                return bottomleft();
            case POS_BOTTOMRIGHT:
                return bottomright();
            case POS_CENTER:
                return center();
            default:
                assert(false);
        }
    }
    return QPointF();
}

// GfxFrame::opposite_pos
//
// return position of opposite handle
//
QPointF GfxFrame::opposite_pos(HandlePos pos) const {
    if (pos & POS_ANISOSCALE) {
        switch (pos & POS_WHICHMASK) {
            case POS_LEFT:
                return midright();
            case POS_RIGHT:
                return midleft();
            case POS_TOP:
                return midbottom();
            case POS_BOTTOM:
                return midtop();
            default:
                assert(false);
        }
    } else {
        switch (pos & POS_WHICHMASK) {
            case POS_TOPLEFT:
                return bottomright();
            case POS_TOPRIGHT:
                return bottomleft();
            case POS_BOTTOMLEFT:
                return topright();
            case POS_BOTTOMRIGHT:
                return topleft();
            case POS_CENTER:
                return center();
            default:
                assert(false);
        }
    }
    return QPointF();
}

// GfxFrame::start
//
// start a path
//
void GfxFrame::start(const QPointF& p, double pressure) {}

// GfxFrame::stop
//
// stop a path
//
void GfxFrame::stop(const QPointF& p, double pressure) {}

// GfxFrame::move
//
// move end of path
//
void GfxFrame::move(const QPointF& p, double pressure) {}

QRectF GfxFrame::boundingRect() const {
    QRectF rect = QRectF(_p1, _p2).normalized();
    auto dx = _pen.width() + 5.0;
    rect.adjust(-dx, -dx, dx, dx);
    return rect;
}

void GfxFrame::setRect(const QPointF& p1, const QPointF& p2) {
    prepareGeometryChange();
    _p1 = p1;
    _p2 = p2;

    if (_move_handle) { _move_handle->setPos(center()); }
    if (_rot_handle[0]) {
        _rot_handle[0]->setPos(topleft());
        _rot_handle[1]->setPos(topright());
        _rot_handle[2]->setPos(bottomright());
        _rot_handle[3]->setPos(bottomleft());
    }
    if (_iso_handle[0]) {
        _iso_handle[0]->setPos(topleft());
        _iso_handle[1]->setPos(topright());
        _iso_handle[2]->setPos(bottomright());
        _iso_handle[3]->setPos(bottomleft());
    }
    if (_aniso_handle[0]) {
        _aniso_handle[0]->setPos(midtop());
        _aniso_handle[1]->setPos(midright());
        _aniso_handle[2]->setPos(midbottom());
        _aniso_handle[3]->setPos(midleft());
    }
}

void GfxFrame::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget* widget) {
    QPen p = _pen;
    p.setStyle(Qt::DashLine);
    painter->setPen(p);
    painter->drawRect(QRectF(_p1, _p2));
}

GfxFrame::HandlePos operator&(const GfxFrame::HandlePos& a,
                                 const GfxFrame::HandlePos& b) {
    const auto ia = static_cast<std::uint16_t>(a);
    const auto ib = static_cast<std::uint16_t>(b);
    return static_cast<GfxFrame::HandlePos>(ia & ib);
}

GfxFrame::HandlePos operator|(const GfxFrame::HandlePos& a,
                                 const GfxFrame::HandlePos& b) {
    const auto ia = static_cast<std::uint16_t>(a);
    const auto ib = static_cast<std::uint16_t>(b);
    return static_cast<GfxFrame::HandlePos>(ia | ib);
}

GfxFrame::HandlePos operator^(const GfxFrame::HandlePos& a,
                                 const GfxFrame::HandlePos& b) {
    const auto ia = static_cast<std::uint16_t>(a);
    const auto ib = static_cast<std::uint16_t>(b);
    return static_cast<GfxFrame::HandlePos>(ia ^ ib);
}
