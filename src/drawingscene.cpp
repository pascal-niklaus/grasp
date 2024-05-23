/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "drawingscene.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScreen>
#include <QTabletEvent>
#include <QTimer>

#include <chrono>

#include "colors.h"
#include "gfxdash.h"
#include "gfxpath.h"
#include "gfxpixmap.h"
#include "gfxtext.h"
#include "gfxrect.h"
#include "selectkey.h"
#include "settings.h"
#include "utility.h"

// DrawingScene::DrawingScene
//
DrawingScene::DrawingScene(QSettings &qset, int verbosity, QObject *parent) :
    QGraphicsScene{parent}, _qset(qset) {
    _verbosity = verbosity;
    // fix geometry, otherwise scene moves around when items are added
    QRectF r = QGuiApplication::primaryScreen()->geometry();
    const double k = QGuiApplication::primaryScreen()->devicePixelRatio();
    r.setWidth(round(r.width() * k));
    r.setHeight(round(r.height() * k));

    setSceneRect(r);

    _settings.retrieve(_qset);

    // create selection frame
    _selection_frame = new GfxFrame(nullptr, Qt::gray, 1);
    addItem(_selection_frame);
    _selection_frame->hide();

    // prepare RMB menu
    _rmb_menu = new QMenu();
    {
        QAction *action = _rmb_menu->addAction("Delete drawing");
        action->setData(MENU_DELETE);
        action = _rmb_menu->addAction("Save drawing");
        action->setData(MENU_SAVE);
        action = _rmb_menu->addAction("New drawing");
        action->setData(MENU_NEW);
        _rmb_menu->addSeparator();
        action = _rmb_menu->addAction("Cancel");
        action->setData(MENU_CANCEL);
    }

    // dynamic update timer
    _dyn_tick = new QTimer();
    _dyn_tick->setInterval(100);  // 100 milliseconds

    // clang-format off
    connect(_dyn_tick, &QTimer::timeout,
            this, &DrawingScene::slot_dynamic_updates,
            Qt::QueuedConnection);
    connect(this, &DrawingScene::pointerPressed,
            this, &DrawingScene::slot_pointerPressed,
            Qt::QueuedConnection);
    connect(this, &DrawingScene::pointerMoved,
            this, &DrawingScene::slot_pointerMoved,
            Qt::QueuedConnection);
    connect(this, &DrawingScene::pointerReleased,
            this, &DrawingScene::slot_pointerReleased,
            Qt::QueuedConnection);
    connect(this, &DrawingScene::keyPressed,
            this, &DrawingScene::slot_keyPressed,
            Qt::QueuedConnection);
    connect(this, &DrawingScene::save_scene,
            this, &DrawingScene::slot_save_scene,
            Qt::QueuedConnection);
    connect(this, &DrawingScene::restore_scene,
            this, &DrawingScene::slot_restore_scene,
            Qt::QueuedConnection);
    connect(this, &DrawingScene::delete_scene,
            this, &DrawingScene::slot_delete_scene,
            Qt::QueuedConnection);
    // clang-format on

    _dash = new GfxDash(nullptr, sceneRect().width(), 25, _settings);
    _dash->setPos(0, 0);
    addItem(_dash);

    // clang-format off
    connect(this, &DrawingScene::set_color,
            this, &DrawingScene::slot_set_color);
    connect(this, &DrawingScene::set_arrow,
            this, &DrawingScene::slot_set_arrow);
    connect(this, &DrawingScene::setup,
            this, &DrawingScene::slot_setup);
    // clang-format on

    emit set_arrow(ARROW_END);

    auto px = QPixmap(":/icons/icons/grasp.png");
    _icon = new QGraphicsPixmapItem(px);
    addItem(_icon);
    _icon->setScale(50.0 / px.width());
    _icon->setPos(5, 5);

    _current_scene = new_scene_id();
    _dash->add_scene(_current_scene);
    _dash->select_scene(_current_scene);
}

DrawingScene::~DrawingScene() {
    slot_clear_background();
    for (auto &it : _undo_store) delete it.second;
    _undo_store.clear();
    _undo_records.clear();
    _redo_records.clear();

    for (auto &sc : _saved_scenes) { sc.second.free_items(); }

    delete _rmb_menu;
    delete _dyn_tick;
}

void DrawingScene::slot_reposition_dash(int width) {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_reposition_dash" << width;
    _dash->reposition(width);
}

GfxItem *DrawingScene::find_item(int ID) const {
    for (auto &item : items()) {
        if (item->type() > GFX_SELECTABLE) {
            bool ok;
            auto id = item->data(GfxItem::KeyID).toInt(&ok);
            if (ok && id == ID) return static_cast<GfxItem *>(item);
        }
    }
    return nullptr;
}

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
            Q_UNUSED(pressure);
            const auto mods = ev->modifiers();
            const auto buttons = ev->buttons();
            emit pointerPressed(p, buttons, mods);
            ev->accept();
            return true;
        }

        case QEvent::TabletMove: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            QPointF p = ev->scenePosition();
            double pressure = ev->pressure();
            Q_UNUSED(pressure);
            const auto mods = ev->modifiers();
            emit pointerMoved(p, mods);
            ev->accept();
            return true;
        }

        case QEvent::TabletRelease: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            QPointF p = ev->scenePosition();
            double pressure = ev->pressure();
            Q_UNUSED(pressure);
            const auto mods = ev->modifiers();
            emit pointerReleased(p, mods);
            ev->accept();
            return true;
        }

            // ---------------- key press and release ----------------

        case QEvent::KeyPress: {
            return QGraphicsScene::event(event);
        }

        case QEvent::KeyRelease:
            return QGraphicsScene::event(event);

            // --------------------- mouse event ---------------------

        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            const QPointF p = ev->scenePos();
            const auto mods = ev->modifiers();
            const auto buttons = ev->buttons();
            emit pointerPressed(p, buttons, mods);
            ev->accept();
            return true;
        }

        case QEvent::GraphicsSceneMouseMove: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            const QPointF p = ev->scenePos();
            const auto mods = ev->modifiers();
            emit pointerMoved(p, mods);
            ev->accept();
            return true;
        }

        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            const QPointF p = ev->scenePos();
            const auto mods = ev->modifiers();
            emit pointerReleased(p, mods);
            ev->accept();
            return true;
        }

        default:
            return QGraphicsScene::event(event);
    }
}

void DrawingScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) {
    ev->accept();
}

void DrawingScene::keyPressEvent(QKeyEvent *keyEvent) {
    GfxItem *kitem = static_cast<GfxItem *>(_current_item);

    if (kitem && _feed_keystrokes) {
        auto result =
            kitem->keystroke(keyEvent->keyCombination(), keyEvent->text());
        if (result == GfxItem::KEY_STOP) {
            _feed_keystrokes = false;
            _current_item = nullptr;
            return;
        }
        if (result == GfxItem::KEY_PROCESSED) return;
    }

    const auto key = keyEvent->keyCombination();
    const auto text = keyEvent->text();
    emit keyPressed(key, text);
}

// DrawingScene::pointerPressed
//
void DrawingScene::slot_pointerPressed(QPointF p,
                                       Qt::MouseButtons buttons,
                                       Qt::KeyboardModifiers mods) {
    if (_current_item && _current_item->type() == GFX_TEXT) {
        GfxText *textitem = static_cast<GfxText *>(_current_item);

        if (textitem->text().isEmpty()) {
            textitem->deselect();
            textitem->pgc();
            removeItemSafely(textitem);
        } else {
            textitem->finalize();
        }
        _current_item = nullptr;
    }

    if (handle_dash_clicks(p, buttons, mods)) {
        // event is processed by handle_dash_clicks, do nothing here...
    } else if (_settings.tool == TOOL_SELECT) {
        SelectType sel = SEL_SINGLE;
        if (mods & Qt::ControlModifier)
            sel = SEL_TOGGLE;
        else if (mods & Qt::ShiftModifier)
            sel = SEL_ADD;
        if (!handle_transformation_clicks(p)) handle_selection_clicks(p, sel);
    } else if (!(buttons & Qt::RightButton)) {
        _current_item = Tools::create_tool(_settings);
        GfxItem *kitem = static_cast<GfxItem *>(_current_item);

        _feed_keystrokes = (kitem->caps() & CAP_KEYSTROKES);

        if (kitem->caps() & CAP_DYNAMIC) _dyn_tick->start();

        _redo_records.clear();
        garbage_collect();

        UndoRecord rec;
        rec.ids_to_remove.insert(kitem->data(GfxItem::KeyID).toInt());
        _undo_records.emplace_back(rec);

        addItem(_current_item);
        kitem->start(p);
        kitem->move(p);
    }
}

// DrawingScene::pointerMoved
//
void DrawingScene::slot_pointerMoved(QPointF p, Qt::KeyboardModifiers mods) {
    if (_settings.tool == TOOL_SELECT) {
        if (_trans.is_ongoing()) { handle_transformation_move(p, mods); }
    } else {
        if (_current_item) {
            GfxItem *kitem = static_cast<GfxItem *>(_current_item);
            if (kitem->type() != GFX_TEXT) kitem->move(p);
        }
    }
}

// DrawingScene::pointerReleased
//
void DrawingScene::slot_pointerReleased(QPointF p, Qt::KeyboardModifiers mods) {
    if (_settings.tool == TOOL_SELECT) {
        if (_trans.is_ongoing()) { handle_transformation_release(p); }
    } else {
        if (_current_item) {
            GfxItem *kitem = static_cast<GfxItem *>(_current_item);
            kitem->stop(p);
            switch (kitem->type()) {
                case GFX_TEXT:
                    // special case of text item: keep item after mouse released
                    break;
                case GFX_PIXMAP: {
                    GfxPixmap *kpixmap = static_cast<GfxPixmap *>(kitem);
                    const QRect rect = kpixmap->sel_rect().toRect();
                    emit take_partial_screenshot(rect);
                } break;

                default:
                    _current_item = nullptr;
            }
        }
    }
}

// DrawingScene::slot_keyPressed
//
void DrawingScene::slot_keyPressed(QKeyCombination kc, QString text) {
    kc = SelectKey::canonicalize(kc);
    if (kc == _settings.grab_key) {
        emit take_screenshot();
    } else if (kc == _settings.quit_key) {
        emit quit();
    } else if (kc == _settings.undo_key) {
        emit undo();
    } else if (kc == _settings.redo_key) {
        emit redo();
    } else if (kc == _settings.toggle_key) {
        emit toggle();
    } else if (kc == _settings.opacity_key) {
        emit opacity();
    } else if (kc == _settings.arrow_start_key) {
        emit set_arrow(ARROW_START);
    } else if (kc == _settings.arrow_end_key) {
        emit set_arrow(ARROW_END);
    } else if (kc == _settings.arrow_double_key) {
        emit set_arrow(ARROW_DOUBLE);
    } else if (kc == _settings.arrow_none_key) {
        emit set_arrow(ARROW_NONE);
    } else if (kc == _settings.grab_partial_key) {
        slot_set_tool(TOOL_PIXMAP);
        emit set_transparent();
        stash_scene();
    } else if (kc == _settings.dash_key) {
        if (_dash->isVisible())
            _dash->hide();
        else
            _dash->show();
    } else if (kc == _settings.del_key) {
        delete_selection();
    } else if (kc == _settings.del_all_key) {
        clear_scene();
    } else if (kc == _settings.line_key) {
        slot_set_tool(TOOL_LINE);
    } else if (kc == _settings.rect_key) {
        slot_set_tool(TOOL_RECT);
    } else if (kc == _settings.pen_key) {
        slot_set_tool(TOOL_PEN);
    } else if (kc == _settings.fader_key) {
        slot_set_tool(TOOL_FADER);
    } else if (kc == _settings.highlight_key) {
        slot_set_tool(TOOL_HIGHLIGHT);
    } else if (kc == _settings.smooth_key) {
        slot_set_tool(TOOL_SMOOTH);
    } else if (kc == _settings.ortho_key) {
        slot_set_tool(TOOL_ORTHO);
    } else if (kc == _settings.edit_key) {
        slot_set_tool(TOOL_SELECT);
    } else if (kc == _settings.text_key) {
        slot_set_tool(TOOL_TEXT);
    } else {
        // check for line width keystrokes
        const int nw =
            sizeof(_settings.width_key) / sizeof(_settings.width_key[0]);
        for (int i = 0; i < nw; i++) {
            if (_settings.width_key[i] == kc) {
                _settings.width = _settings.line_widths[i];
                _settings.highlight_width = _settings.highlight_line_width[i];
                return;
            }
        }

        // check for color keystrokes
        QColor col = _settings.get_color(kc);
        if (col.isValid()) {
            slot_set_color(col);
            return;
        }
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
            auto kitem = static_cast<GfxItem *>(item);
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

// DrawingScene::delete_selection
//
// delete all items selected (and create an undo record)
//
void DrawingScene::delete_selection() {
    if (_verbosity >= VerboseAll) qDebug() << "DrawingScene::delete_selection";
    if (_selection_frame) {
        UndoRecord rec;
        for (auto &item : _selected_items) {
            int id = item->data(GfxFrame::KeyID).toInt();
            GfxUndoItem *undo_item = item->create_undo_item();
            item->deselect();
            item->pgc();
            removeItemSafely(item);
            rec.ids_to_add.insert(id);
            _undo_store.insert({id, undo_item});
        }
        _undo_records.emplace_back(rec);
        update_selection_frame();
    }
}

// DrawingScene::clear_selection
//
// make sure everything is unselected
//
void DrawingScene::clear_selection() {
    for (auto &item : _selected_items) item->deselect();
    update_selection_frame();
}

// DrawingScene::handle_selection_clicks
//
// handle item selection / deselection
//
void DrawingScene::handle_selection_clicks(const QPointF &p, SelectType sel) {
    for (auto &item : items()) {
        if (item->type() > GfxItemType::GFX_SELECTABLE) {
            auto *kitem = static_cast<GfxItem *>(item);
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

// DrawingScene::dash_clicked
//
// check if a dashboard item clicked
//
bool DrawingScene::handle_dash_clicks(const QPointF &p,
                                      const Qt::MouseButtons buttons,
                                      const Qt::KeyboardModifiers mods) {
    if (_dash->isVisible())
        for (auto &item : items()) {
            const auto itype = item->type();
            if (itype > GFX_DASH_MIN && itype < GFX_DASH_MAX) {
                auto *ditem = static_cast<DashIcon *>(item);
                if (ditem->contains(ditem->mapFromScene(p))) {
                    const auto type = ditem->data(GfxItem::KeyDash).toInt() &
                                      DashIcon::DashMask;
                    const auto data = ditem->data(GfxItem::KeyDashAux).toInt();
                    switch (type) {
                        case DashIcon::DashCol:
                            emit set_color(Color::qcolor_from_uint32(data));
                            break;
                        case DashIcon::DashScene: {
                            if (buttons & Qt::RightButton) {
                                auto chosen = _rmb_menu->exec(QCursor::pos());
                                if (chosen && chosen->data().isValid()) {
                                    auto item = chosen->data().toInt();
                                    if (item == MENU_SAVE) {
                                        QString filename =
                                            QFileDialog::getSaveFileName(
                                                nullptr,
                                                tr("Save sketch..."),
                                                _settings.save_dir,
                                                tr("PNG images (*.png)"));
                                        if (!filename.isEmpty()) {
                                            QFileInfo fi(filename);
                                            if (fi.suffix().toLower() != ".png")
                                                filename += ".png";
                                            bool visibility =
                                                _dash->isVisible();
                                            _dash->hide();
                                            _icon->hide();
                                            QImage image(
                                                sceneRect().size().toSize(),
                                                QImage::Format_RGB32);
                                            QPainter painter(&image);
                                            painter.setRenderHint(
                                                QPainter::Antialiasing);
                                            render(&painter);
                                            _icon->show();
                                            _dash->setVisible(visibility);
                                            image.save(filename);
                                        }
                                    } else if (item == MENU_NEW) {
                                        slot_new();
                                    } else if (item == MENU_DELETE) {
                                        if (data == _current_scene) {
                                            QMessageBox::warning(
                                                nullptr,
                                                "Cannot delete",
                                                "Cannot delete currently "
                                                "active sketch",
                                                QMessageBox::Ok,
                                                QMessageBox::Ok);
                                        } else {
                                            emit delete_scene(data);
                                        }
                                    }
                                }
                            } else {
                                if (type == DashIcon::DashScene) {
                                    if (data != _current_scene) {
                                        slot_save_scene(_current_scene);
                                        slot_restore_scene(_current_scene =
                                                               data);
                                        _dash->select_scene(_current_scene);
                                    } else {
                                        // is already current scene !
                                    }
                                }
                            }
                        } break;
                        case DashIcon::DashArrow:
                            emit set_arrow(static_cast<ArrowType>(data));
                            break;
                        case DashIcon::DashSetup:
                            emit setup();
                            break;
                    }  // switch(type)

                    return true;
                }  // ditem->contains(...)
            }      // dash item clicked
        }
    return false;
}

// DrawingScene::handle_clicked
//
// check if a transformation handle was clicked
//
// if yes, store cursor position and operation type and return true
//
bool DrawingScene::handle_transformation_clicks(const QPointF &p) {
    _trans.handle = GfxFrame::POS_NONE;
    for (auto &item : items()) {
        const auto itype = item->type();
        if (itype == GfxItemType::GFX_HANDLE ||
            itype == GfxItemType::GFX_ROTATEHANDLE ||
            itype == GfxItemType::GFX_MOVEHANDLE) {
            if (item->contains(item->mapFromScene(p))) {
                _trans.handle = static_cast<GfxFrame::HandlePos>(
                    item->data(GfxItem::KeyHandle).toUInt());
                _trans.pos_ref = _selection_frame->opposite_pos(_trans.handle);
                _trans.pos_cur = _selection_frame->handle_pos(_trans.handle);
                _trans.pos_start = p;
                _trans.pos_frame_start = _selection_frame->pos();

                UndoRecord rec;
                for (auto &item : _selected_items) {
                    GfxUndoItem *undo_item = item->create_undo_item();
                    const int old_id = item->data(GfxItem::KeyID).toInt();
                    item->set_new_ID();
                    const int new_id = item->data(GfxItem::KeyID).toInt();
                    _undo_store.insert({old_id, undo_item});
                    rec.ids_to_add.insert(old_id);
                    rec.ids_to_remove.insert(new_id);
                    item->stash();
                }
                _undo_records.emplace_back(rec);
                return true;
            }
        }
    }
    return false;
}

void DrawingScene::handle_transformation_move(const QPointF &p,
                                              Qt::KeyboardModifiers mods) {
    QPointF delta = p - _trans.pos_start;
    _trans.m.reset();
    _trans.m.translate(_trans.pos_ref.x(), _trans.pos_ref.y());

    if (_trans.handle & GfxFrame::POS_MOVE) {
        _trans.m.translate(delta.x(), delta.y());
    } else if (_trans.handle & GfxFrame::POS_ISOSCALE) {
        delta += (_trans.pos_cur - _trans.pos_ref);
        double kx = (delta.x() / (_trans.pos_cur.x() - _trans.pos_ref.x()));
        double ky = (delta.y() / (_trans.pos_cur.y() - _trans.pos_ref.y()));
        if (mods & Qt::ShiftModifier) { kx = ky = max<double>(kx, ky); }
        _trans.m.scale(kx, ky);
    } else if (_trans.handle & GfxFrame::POS_ANISOSCALE) {
        delta += (_trans.pos_cur - _trans.pos_ref);
        double kx = delta.x() / (_trans.pos_cur.x() - _trans.pos_ref.x());
        double ky = delta.y() / (_trans.pos_cur.y() - _trans.pos_ref.y());
        auto which = _trans.handle & GfxFrame::POS_WHICHMASK;
        if (which == GfxFrame::POS_LEFT || which == GfxFrame::POS_RIGHT)
            ky = 1.0;
        else
            kx = 1.0;
        _trans.m.scale(kx, ky);
    } else if (_trans.handle & GfxFrame::POS_ROTATE) {
        double alpha0 = atan2(_trans.pos_cur.y() - _trans.pos_ref.y(),
                              _trans.pos_cur.x() - _trans.pos_ref.x());
        double alpha1 =
            atan2(delta.y() + _trans.pos_cur.y() - _trans.pos_ref.y(),
                  delta.x() + _trans.pos_cur.x() - _trans.pos_ref.x());
        _trans.m.rotate((alpha1 - alpha0) * 180 / M_PI);
    }
    _trans.m.translate(-_trans.pos_ref.x(), -_trans.pos_ref.y());
    for (auto &item : _selected_items) item->apply(_trans.m);
    update_selection_frame();
}

// DrawingScene::handle_transformation_release
//
void DrawingScene::handle_transformation_release(const QPointF &p) {
    _trans.m.reset();
    _trans.handle = GfxFrame::POS_NONE;
    update_selection_frame();
}

// DrawingScene::undo
//
// execute last undo record and place it in redo
//
void DrawingScene::undo() {
    if (_verbosity >= VerboseAll) qDebug() << "DrawingScene::undo";
    clearSelection();
    if (_undo_records.empty()) return;
    UndoRecord &rec = _undo_records.back();
    for (auto id : rec.ids_to_remove) {
        GfxItem *ptr = find_item(id);
        if (ptr) {
            ptr->deselect();
            GfxUndoItem *undo_item = ptr->create_undo_item();
            ptr->pgc();
            removeItemSafely(ptr);
            _undo_store.insert({id, undo_item});
        } else {
            if (_verbosity >= VerboseLow)
                qDebug() << "undo: Could not find item" << id << "ptr=" << ptr;
        }
    }
    for (auto id : rec.ids_to_add) {
        auto it = _undo_store.find(id);
        if (it != _undo_store.end()) {
            GfxItem *item = it->second->create_from_undo_item();
            addItem(item);
            delete it->second;
            _undo_store.erase(it);
        } else {
            if (_verbosity >= VerboseLow)
                qDebug() << "undo: Could not find item" << id;
        }
    }
    _redo_records.emplace_back(rec);
    _undo_records.pop_back();
    clearSelection();
    update_selection_frame();
}

// DrawingScene::undo
//
// execute last redo record and place it in undo
//
void DrawingScene::redo() {
    if (_verbosity >= VerboseAll) qDebug() << "DrawingScene::redo";
    clearSelection();
    if (_redo_records.empty()) return;
    UndoRecord &rec = _redo_records.back();
    for (auto id : rec.ids_to_add) {
        GfxItem *ptr = find_item(id);
        if (ptr) {
            ptr->deselect();
            GfxUndoItem *redo_item = ptr->create_undo_item();
            ptr->pgc();
            removeItemSafely(ptr);
            _undo_store.insert({id, redo_item});
        } else {
            if (_verbosity >= VerboseLow)
                qDebug() << "redo: Could not find item" << id << "ptr=" << ptr;
        }
    }
    for (auto id : rec.ids_to_remove) {
        auto it = _undo_store.find(id);
        if (it != _undo_store.end()) {
            GfxItem *item = it->second->create_from_undo_item();
            addItem(item);
            delete it->second;
            _undo_store.erase(it);
        } else {
            if (_verbosity >= VerboseLow)
                qDebug() << "redo: Could not find item" << id;
        }
    }
    _undo_records.emplace_back(rec);
    _redo_records.pop_back();
    update_selection_frame();
}

// DrawingScene::garbage_collect
//
// remove items from undo_store that are no longer references, and remove items
// from undo/redo records that contain references to items that neither exist in
// items() or in undo_store
//
// this is repeated until a consistent set remains
//
// the implementation is not really efficient, but this should not be important
//
void DrawingScene::garbage_collect() {
    if (_verbosity >= VerboseAll) qDebug() << "DrawingScene::garbage_collect";
    int last_n = 0;
    return;
    for (;;) {
        int new_n =
            _undo_store.size() + _undo_records.size() + _redo_records.size();
        if (new_n == last_n) break;
        last_n = new_n;

        // remove records from undo_store that are no longer references by
        // any undo or redo operation
        std::unordered_set<int> ids_in_undoredo;
        for (const auto &rec : _undo_records) {
            for (auto v : rec.ids_to_add) ids_in_undoredo.insert(v);
            for (auto v : rec.ids_to_remove) ids_in_undoredo.insert(v);
        }
        for (const auto &rec : _redo_records) {
            for (auto v : rec.ids_to_add) ids_in_undoredo.insert(v);
            for (auto v : rec.ids_to_remove) ids_in_undoredo.insert(v);
        }

        std::erase_if(_undo_store, [&](auto &v) -> bool {
            bool do_erase = !ids_in_undoredo.contains(v.first);
            if (do_erase) delete v.second;
            return do_erase;
        });

        // remove undo or redo operations that reference objects that no
        // longer exist in items() or undo_store
        std::unordered_set<int> ids_in_stores;
        for (const auto &item : items())
            ids_in_stores.insert(item->data(GfxItem::KeyID).toInt());

        for (const auto &item : _undo_store)
            ids_in_stores.insert(item.second->id);

        std::erase_if(_undo_records, [&](auto &rec) -> bool {
            return !rec.all_ids_in(ids_in_stores);
        });
        std::erase_if(_redo_records, [&](auto &rec) -> bool {
            return !rec.all_ids_in(ids_in_stores);
        });
    }
}


// DrawingScene::removeItemSafely
//
// remove an item from the scene, first deleting all childs, and
// finally the item itself. Removing the item directly caused crashes,
// not sure why, but this works.
//
void DrawingScene::removeItemSafely(QGraphicsItem *item) {
    static int rem_level = 0;
    rem_level++;
    if (_verbosity >= VerboseAll) {
        qDebug() << "DrawingScene::removeItemSafely..."
                 << "level =" << rem_level << item;

        if (item->childItems().size())
            qDebug() << "- removing" << item->childItems().size() << "children";
    }

    while (item->childItems().size() > 0)
        removeItemSafely(item->childItems().first());

    removeItem(item);
    delete item;
    if (_verbosity >= VerboseAll) {
        qDebug() << "DrawingScene::removeItemSafely level =" << rem_level
                 << "done ";
    }
    rem_level--;
}

// DrawingScene::slot_pixmap
//
void DrawingScene::slot_pixmap(const QPixmap &pixmap) {
    if (_verbosity >= VerboseAll) qDebug() << "DrawingScene::slot_pixmap";

    if (_current_item) {
        switch (_current_item->type()) {
            case GFX_PIXMAP: {
                GfxPixmap *kpixmap = static_cast<GfxPixmap *>(_current_item);
                kpixmap->set_pixmap(pixmap, kpixmap->sel_rect());
                _current_item = nullptr;
                unstash_scene();
                slot_set_tool(_pre_grab_tool);
            } break;

            default: {
                qDebug() << "DrawingScene::slot_pixmap called, but current "
                            "item if not a GFX_PIXMAP !";
            } break;
        }
    }
}

// DrawingScene::stash_scene
//
void DrawingScene::stash_scene() {
    if (_verbosity >= VerboseAll) qDebug() << "DrawingScene::stash_scene...";

    clearSelection();

    _stash.stash.clear();
    if (_background_item) {
        _stash.background = _background_item->pixmap();
        slot_clear_background();
    } else {
        _stash.background = QPixmap();
    }

    for (auto &it : items()) {
        if (it->type() > GFX_SELECTABLE) {
            GfxItem *kitem = static_cast<GfxItem *>(it);
            auto id = kitem->data(GfxItem::KeyID).toInt();
            GfxUndoItem *undo_item = kitem->create_undo_item();
            kitem->pgc();
            removeItemSafely(kitem);
            _stash.stash.insert({id, undo_item});
        }
    }
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::stash_scene... done";
}

// DrawingScene::unstash_scene
//
void DrawingScene::unstash_scene() {
    if (_verbosity >= VerboseAll) qDebug() << "DrawingScene::unstash_scene...";
    clear_selection();

    slot_clear_background();
    if (!_stash.background.isNull()) slot_set_background(_stash.background);

    for (auto &it : _stash.stash) {
        GfxItem *item = it.second->create_from_undo_item();
        delete it.second;
        addItem(item);
    }
    _stash.stash.clear();
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::unstash_scene... done";
}

// DrawingScene::slot_set_background
//
// set background pixmap
//
void DrawingScene::slot_set_background(QPixmap px) {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_set_background";
    slot_clear_background();
    if (px.isNull()) return;
    _background_item = addPixmap(px);
    _background_item->setOffset(0, 0);
    _background_item->setScale(
        QGuiApplication::primaryScreen()->devicePixelRatio());
    _background_item->setZValue(-1000.0);
}

// DrawingScene::slot_clear_background
//
// remove background pixmap, if there is any
//
void DrawingScene::slot_clear_background() {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_clear_background";
    if (_background_item) {
        removeItemSafely(_background_item);
        _background_item = nullptr;
    }
}
// DrawingScene::slot_dynamic_updates
//
// periodically called when there are items that request this by
// returning CAP_DYNAMIC. Stop respective timer when there are no such
// items remaining.
//
void DrawingScene::slot_dynamic_updates() {
    bool any_remaining = false;
    bool selected_deleted = false;
    for (auto &it : items()) {
        if (it->type() > GFX_SELECTABLE) {
            GfxItem *kitem = static_cast<GfxItem *>(it);
            if (kitem->caps() & CAP_DYNAMIC) {
                any_remaining = true;
                auto result = kitem->dynamic();
                if (result == GfxItem::DYN_DELITEM) {
                    if (_selected_items.contains(kitem))
                        selected_deleted = true;
                    kitem->pgc();
                    removeItemSafely(kitem);
                }
            }
        }
    }
    if (any_remaining == false) _dyn_tick->stop();
    if (selected_deleted) update_selection_frame();
}

// DrawingScene::init_tool
//
// do stuff needed when a new tool is selected.
//
void DrawingScene::init_tool() {
    emit tool_changed(_settings.tool);

    switch (_settings.tool) {
        case TOOL_PIXMAP: {
            emit set_transparent();
            break;
        }
        default: {
            emit set_opaque();
            break;
        }
    }
}

// ----------------------- save / load scenes ------------------------

// important: the same undo item pointer must not be saved in two
// places, otherwise this will eventually cause a double deletion!

// DrawingScene::slot_save_scene
//
// moves a scene into _saved_scenes, under id 'id'
//
void DrawingScene::slot_save_scene(int id) {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_save_scene" << id;

    clear_selection();

    {  // delete saved scene if it already exists
        auto it = _saved_scenes.find(id);
        if (it != _saved_scenes.end()) {
            it->second.free_items();
            _saved_scenes.erase(it);
        }
    }

    SavedScene scene;

    // copy settings
    scene.settings = _settings;

    // save and remove background, if any
    if (_background_item) {
        scene.background = _background_item->pixmap();
        slot_clear_background();
    }

    // copy over undo/redo records
    scene.undo_records = _undo_records;
    scene.redo_records = _redo_records;
    _undo_records.clear();
    _redo_records.clear();

    // CAREFUL: here the undo record pointer is moved, but the item
    // itself remains allocated where it is
    scene.undo_store = _undo_store;
    _undo_store.clear();

    // construct records in gfxitems
    for (auto &it : items()) {
        if (it->type() > GFX_SELECTABLE) {
            GfxItem *kitem = static_cast<GfxItem *>(it);
            auto id = kitem->data(GfxItem::KeyID).toInt();
            GfxUndoItem *undo_item = kitem->create_undo_item();
            kitem->pgc();
            removeItemSafely(kitem);
            scene.gfxitems.insert({id, undo_item});
        }
    }

    // insert everything into map with saved scenes
    _saved_scenes.insert({id, scene});
}

// DrawingScene::slot_restore_scene
//
// Re-create scene with id 'id', re-creating all GfxItems based on
// the undo records in _saved_scenes
//
void DrawingScene::slot_restore_scene(int id) {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_restore_scene" << id;

    clear_selection();
    // find the stored scene, and return if it does not exist
    auto it = _saved_scenes.find(id);
    if (it == _saved_scenes.end()) {
        if (_verbosity >= VerboseLow) std::cerr << "- scene not found !\n";
        return;
    }

    auto &scene = it->second;

    // the scene must be empty to start with
    clear_scene();

    // copy settings
    _settings = scene.settings;

    // restore background
    slot_set_background(scene.background);

    _undo_records = scene.undo_records;
    _redo_records = scene.redo_records;
    scene.undo_records.clear();
    scene.redo_records.clear();

    // copy over the undo_store
    // IMPORTANT: the undo item references in the scene are removed,
    // so no double pointers to the same item remain!
    _undo_store = scene.undo_store;
    scene.undo_store.clear();

    // reconstruct all visible items
    for (auto &it : scene.gfxitems) {
        GfxItem *item = it.second->create_from_undo_item();
        addItem(item);
    }

    // there may be dynamic items that were restored
    _dyn_tick->start();
}

// DrawingScene::slot_delete_scene
//
// delete a scene, cannot be undone
//
void DrawingScene::slot_delete_scene(int id) {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_delete_scene" << id;

    // find the stored scene, and return if it does not exist
    auto it = _saved_scenes.find(id);
    if (it == _saved_scenes.end()) {
        if (_verbosity >= VerboseLow) std::cerr << "ERROR: scene not found !\n";
        return;
    }

    // remove saved scene
    SavedScene &saved = it->second;
    saved.free_items();
    _saved_scenes.erase(id);

    // also remove icon from dash
    _dash->remove_scene(id);
}

// DrawingScene::clear_scene
//
// remove everything, cannot be undone
//
void DrawingScene::clear_scene() {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_clear_scene";

    clearSelection();

    // remove background, if any
    if (_background_item) slot_clear_background();

    for (auto &it : _undo_store) delete it.second;
    _undo_store.clear();

    _redo_records.clear();
    _undo_records.clear();

    // remove all items
    for (auto &it : items()) {
        if (it->type() > GFX_SELECTABLE) {
            GfxItem *kitem = static_cast<GfxItem *>(it);
            kitem->pgc();
            removeItemSafely(kitem);
        }
    }
}

// DrawingScene::slot_new
//
// start a new scene
//
void DrawingScene::slot_new() {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_new";

    slot_save_scene(_current_scene);
    _current_scene = new_scene_id();
    _dash->add_scene(_current_scene);
    _dash->select_scene(_current_scene);
}

// DrawingScene::slot_set_color
//
// change the color
//
void DrawingScene::slot_set_color(const QColor &color) {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_set_color" << color;

    _settings.color = color;
    _dash->select_color(color);
}

// DrawingScene::slot_set_color_from_index
//
// change the color, given an index (for example received via dbus command)
//
void DrawingScene::slot_set_color_from_index(int index) {
    if (index < 0 || index >= static_cast<int>(_settings.color_table.size())) {
        qDebug()
            << "DrawingScene::slot_set_color_from_index: illegal color index"
            << index;
        return;
    }
    _settings.color = _settings.color_table[index].color;
    _dash->select_color(_settings.color);
}

// DrawingScene::slot_set_tool
//
// set tool type. the grab tool also remembers what tool was set
// before; it is later reset when the pixmap was obtained
//
void DrawingScene::slot_set_tool(ToolType tool) {
    if (tool == TOOL_PIXMAP) {
        if (_settings.tool != TOOL_PIXMAP) _pre_grab_tool = _settings.tool;
    }
    _settings.tool = tool;
    init_tool();
    clear_selection();
}

// DrawingScene::slot_set_arrow
//
// set arrow heads
//
void DrawingScene::slot_set_arrow(ArrowType type) {
    _settings.arrow_type = type;
    _dash->select_arrow(type);
}

// DrawingScene::slot_set_width
//
// set line width
//
void DrawingScene::slot_set_width(int width) {
    _settings.width = width;
}

// DrawingScene::new_scene_id
//
// return a new, unique scene id
//
int DrawingScene::new_scene_id(void) {
    static int id = 0;
    return ++id;
}

// DrawingScene::slot_setup
//
// setup dialog
//
void DrawingScene::slot_setup() {
    SettingsDialog settings(_settings);
    if (settings.exec() == QDialog::Accepted) _settings.store(_qset);
}

// --------------- add items without user interaction ----------------

// These are some items that can be added via dbus commands. the idea
// is not to make all functions available, and these actions are not
// undoable.

void DrawingScene::slot_add_rect(const QRectF &rect, int radius) {
    if (_verbosity >= VerboseAll)
        qDebug() << "DrawingScene::slot_add_rect" << rect;
    auto *rectitem = new GfxRect(
        nullptr, _settings.color, _settings.width, radius);
    addItem(rectitem);
    rectitem->setData(GfxItem::KeyID, Tools::generate_ID());
    rectitem->start(rect.topLeft());
    rectitem->stop(rect.bottomRight());
}
