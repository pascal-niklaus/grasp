#include "drawingscene.h"
#include <qbrush.h>
#include <qdebug.h>
#include <qevent.h>
#include <QGraphicsItemGroup>
#include <qgraphicsitem.h>
#include <qnamespace.h>
#include <qpoint.h>
#include <qtransform.h>

#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>

#include <QApplication>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QScreen>
#include <QTabletEvent>

#include <iostream>

#include "kromit.h"
#include "kromitframe.h"
#include "kromititem.h"
#include "kromitpath.h"
#include "kromithandle.h"
#include "kromitrotate.h"
#include "kromitmove.h"
#include "tools.h"
#include "utility.h"

// DrawingScene::DrawingScene
//
DrawingScene::DrawingScene(QObject *parent) : QGraphicsScene{parent} {
    // fix geometry, otherwise scene moves around when items are added
    setSceneRect(QGuiApplication::primaryScreen()->geometry());

    // set default tool settings
    _settings.tool = TOOL_LINE;
    _settings.color = Qt::yellow;
    _settings.width = 5;
    _settings.arrow_type = ARROW_NONE;
    _settings.arrow_size = 0;
    _settings.minwidth = 1;
    _settings.maxwidth = 4;
    _settings.radius = 10;
    _settings.minlen = 30;
    _settings.simplify = 1;
    _settings.snapdist = 0;

    // create selection frame
    _selection_frame = new KromitFrame(nullptr, Qt::gray, 1);
    addItem(_selection_frame);
    _selection_frame->hide();
}


DrawingScene::~DrawingScene() {}

// forward mouse events to the Scene...

void DrawingScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mousePressEvent(event);
}

void DrawingScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
}

void DrawingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
}

// DrawingScene::event
//
// here tablet and mouse events are processed. The coordinates for
// these events need to be mapped to the scene coordinates

bool DrawingScene::event(QEvent *event) {
    switch (event->type()) {
            // ------------- tablet press, move, release -------------

        case QEvent::TabletPress: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            QPointF p = ev->scenePosition();
            double pressure = ev->pressure();
            _current_item =
              new KromitPath(nullptr, Qt::yellow, pressure * 3.0, ARROW_DOUBLE, 10);
            KromitPath *path = static_cast<KromitPath *>(_current_item);
            path->start(p);
            path->move(p);
            addItem(_current_item);
            ev->accept();
            return true;
        }

        case QEvent::TabletMove: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            double pressure = ev->pressure();
            if (_current_item) {
                KromitPath *path = static_cast<KromitPath *>(_current_item);
                path->move(ev->scenePosition());
            }
            ev->accept();
            return true;
        }

        case QEvent::TabletRelease: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            double pressure = ev->pressure();
            if (_current_item) {
                KromitPath *path = static_cast<KromitPath *>(_current_item);
                path->stop(ev->scenePosition());
                _current_item = nullptr;
            }
            ev->accept();
            return true;
        }

            // ---------------- key press and release ----------------

        case QEvent::KeyPress: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            if (ev->key() == Qt::Key_Delete) {
              delete_selection();
              ev->accept();
              return true;
            }
        }

        case QEvent::KeyRelease:
            return QGraphicsScene::event(event);

            // --------------------- mouse event ---------------------

        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            QPointF p = ev->scenePos();
            if (_settings.tool == TOOL_SELECT) {
                SelectType sel = SEL_SINGLE;
                if (ev->modifiers() & Qt::ControlModifier) {
                    sel = SEL_TOGGLE;
                } else if (ev->modifiers() & Qt::ShiftModifier) {
                    sel = SEL_ADD;
                }
                if (!handle_transformation_clicks(p))
                    handle_selection_clicks(p, sel);
            } else {
                QGraphicsSceneMouseEvent *ev =
                    static_cast<QGraphicsSceneMouseEvent *>(event);
                _current_item = Tools::create_tool(_settings);
                KromitItem *kitem = static_cast<KromitItem *>(_current_item);
                addItem(_current_item);
                kitem->start(p);
                kitem->move(p);
            }
            ev->accept();
            return true;
        }

        case QEvent::GraphicsSceneMouseMove: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            if (_settings.tool == TOOL_SELECT) {
                if (trans.is_ongoing()) {
                  handle_transformation_move(ev->scenePos(), ev->modifiers());
                }
                ev->accept();
                return true;
            } else {
                if (_current_item) {
                    KromitItem *kitem =
                        static_cast<KromitItem *>(_current_item);
                    kitem->move(ev->scenePos());
                }
                ev->accept();
                return true;
            }
        }

        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            if (_settings.tool == TOOL_SELECT) {
                if (trans.is_ongoing()) {
                    handle_transformation_release(ev->scenePos());
                }
                ev->accept();
                return true;
            } else {
                if (_current_item) {
                    KromitItem *kitem =
                        static_cast<KromitItem *>(_current_item);
                    kitem->move(ev->scenePos());
                    _current_item = nullptr;
                }
                ev->accept();
                return true;
            }
        }
        default:
            return QGraphicsScene::event(event);
    }
}

void DrawingScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) {
    ev->accept();
}

void DrawingScene::keyPressEvent(QKeyEvent *keyEvent) {
    switch (keyEvent->key()) {
        case Qt::Key_L: {
            _settings.tool = TOOL_LINE;
        } break;

        case Qt::Key_P: {
            _settings.tool = TOOL_PEN;
        } break;

        case Qt::Key_E: {
            _settings.tool = TOOL_SELECT;
        } break;

        case Qt::Key_D: {
            qDebug() << "---------- items ----------";
            for (const auto &item : items()) {
                qDebug().nospace() << KromitItem::type(item->type())
                                   << " id=" << item->data(KromitItem::KeyID)
                                   << (item->isSelected() ? " selected " : "");
            }
            qDebug() << "---------------------------";
        } break;

        default:
            break;
    }
}

// DrawingScene::update_selection_frame
//
// determine items that are selected and show frame
//
void DrawingScene::update_selection_frame() {
    int n_selected = 0;
    QRectF bbox;
    TransformCaps caps = CAP_NONE;

    _selected_items.clear();
    for (auto &item : items()) {
        if (item->isSelected()) {
            auto kitem = static_cast<KromitItem *>(item);
            _selected_items.append(kitem);
            if (n_selected == 0) {
                bbox = kitem->selectionBox();
                caps = kitem->caps();
            } else {
                bbox = bbox.united(kitem->selectionBox());
                caps = caps & kitem->caps();
            }
            n_selected++;
        }
    }

    bbox = bbox.normalized();

    if (n_selected > 0) {
        _selection_frame->setRect(bbox.topLeft(), bbox.bottomRight());
        _selection_frame->setCaps(caps);
        _selection_frame->show();
    } else {
        _selection_frame->hide();
    }
}

void DrawingScene::delete_selection() {
    if (_selection_frame) {
        for (auto &item : _selected_items) {
            removeItem(item);
            delete (item);
        }
        update_selection_frame();
    }
}

// DrawingScene::handle_selection_clicks
//
// handle item selection / deselection
//
void DrawingScene::handle_selection_clicks(const QPointF &p, SelectType sel) {
    for (auto &item : items()) {
        if (item->type() > GfxItemType::GFX_SELECTABLE) {
            auto *kitem = static_cast<KromitItem *>(item);
            if (item->contains(p)) {
                if (sel == SEL_ADD || sel == SEL_SINGLE)
                    kitem->select();
                else if (sel == SEL_TOGGLE)
                    kitem->toggleselect();
                else
                    kitem->deselect();
            } else {
                if (sel == SEL_SINGLE || sel == SEL_NONE) kitem->deselect();
            }
        }
    }
    update_selection_frame();
}

// DrawingScene::handle_clicked
//
// check if a transformation handle was clicked
//
// if yes, store cursor position and operation type and return true
//
bool DrawingScene::handle_transformation_clicks(const QPointF &p) {
    KromitFrame::HandlePos htype = KromitFrame::POS_NONE;
    for (auto &item : items()) {
        const auto itype = item->type();
        if (itype == GfxItemType::GFX_HANDLE ||
            itype == GfxItemType::GFX_ROTATEHANDLE ||
            itype == GfxItemType::GFX_MOVEHANDLE) {
            auto *kitem = static_cast<KromitItem *>(item);
            if (kitem->contains(kitem->mapFromScene(p))) {
                trans.handle = static_cast<KromitFrame::HandlePos>(
                    kitem->data(KromitItem::KeyHandle).toUInt());
                trans.pos_ref = _selection_frame->opposite_pos(trans.handle);
                trans.pos_cur = _selection_frame->handle_pos(trans.handle);
                trans.pos_start = p;
                trans.pos_frame_start = _selection_frame->pos();
                for (auto &item : _selected_items) item->stash();
                return true;
            }
        }
    }
    return false;
}

void DrawingScene::handle_transformation_move(const QPointF &p,
                                              Qt::KeyboardModifiers mods) {
    QPointF delta = p - trans.pos_start;
    trans.m.reset();
    trans.m.translate(trans.pos_ref.x(), trans.pos_ref.y());

    if (trans.handle & KromitFrame::POS_MOVE) {
        trans.m.translate(delta.x(), delta.y());
    } else if (trans.handle & KromitFrame::POS_ISOSCALE) {
        delta += (trans.pos_cur - trans.pos_ref);
        double kx = (delta.x() / (trans.pos_cur.x() - trans.pos_ref.x()));
        double ky = (delta.y() / (trans.pos_cur.y() - trans.pos_ref.y()));
        if (mods & Qt::ShiftModifier) {
          kx = ky = max<double>(kx, ky);
        }
        trans.m.scale(kx, ky);
    } else if (trans.handle & KromitFrame::POS_ANISOSCALE) {
        delta += (trans.pos_cur - trans.pos_ref);
        double kx = delta.x() / (trans.pos_cur.x() - trans.pos_ref.x());
        double ky = delta.y() / (trans.pos_cur.y() - trans.pos_ref.y());
        auto which = trans.handle & KromitFrame::POS_WHICHMASK;
        if (which == KromitFrame::POS_LEFT || which == KromitFrame::POS_RIGHT)
            ky = 1.0;
        else
            kx = 1.0;
        trans.m.scale(kx, ky);
    } else if (trans.handle & KromitFrame::POS_ROTATE) {
        double alpha0 = atan2(trans.pos_cur.y() - trans.pos_ref.y(),
                              trans.pos_cur.x() - trans.pos_ref.x());
        double alpha1 =
            atan2(delta.y() + trans.pos_cur.y() - trans.pos_ref.y(),
                  delta.x() + trans.pos_cur.x() - trans.pos_ref.x());
        trans.m.rotate((alpha1 - alpha0) * 180 / M_PI);
    }
    trans.m.translate(-trans.pos_ref.x(), -trans.pos_ref.y());
    for (auto &item : _selected_items) item->apply(trans.m);
    update_selection_frame();
}

void DrawingScene::handle_transformation_release(const QPointF &p) {
    update_selection_frame();
}
