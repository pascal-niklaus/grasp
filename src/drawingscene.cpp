#include "drawingscene.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QScreen>
#include <QTabletEvent>
#include <QTimer>
#include <chrono>

#include "colors.h"
#include "kromitpath.h"
#include "kromitpixmap.h"
#include "kromittext.h"
#include "utility.h"

// DrawingScene::DrawingScene
//
DrawingScene::DrawingScene(QObject *parent) : QGraphicsScene{parent} {
    // fix geometry, otherwise scene moves around when items are added
    setSceneRect(QGuiApplication::primaryScreen()->geometry());
    qDebug() << "scene rect" << sceneRect();

    // set default tool settings
    _settings.tool = TOOL_SMOOTH;
    _settings.color = Qt::yellow;
    _settings.width = 5;
    _settings.arrow_type = ARROW_END;
    _settings.arrow_size = 3;
    _settings.minwidth = 1;
    _settings.maxwidth = 4;
    _settings.radius = 10;
    _settings.minlen = 30;
    _settings.simplify = 10;
    _settings.snapdist = 20;
    _settings.fontsize = 20;
    _settings.sustain = 5.0;
    _settings.fade = 5.0;

    // create selection frame
    _selection_frame = new KromitFrame(nullptr, Qt::gray, 1);
    addItem(_selection_frame);
    _selection_frame->hide();

    // dynamic update timer

    dyn_tick = new QTimer();
    dyn_tick->setInterval(100);  // 100 milliseconds
    connect(dyn_tick,
            &QTimer::timeout,
            this,
            &DrawingScene::slot_dynamic_updates,
            Qt::QueuedConnection);

    connect(this,
            &DrawingScene::pointerPressed,
            this,
            &DrawingScene::slot_pointerPressed,
            Qt::QueuedConnection);
    connect(this,
            &DrawingScene::pointerMoved,
            this,
            &DrawingScene::slot_pointerMoved,
            Qt::QueuedConnection);
    connect(this,
            &DrawingScene::pointerReleased,
            this,
            &DrawingScene::slot_pointerReleased,
            Qt::QueuedConnection);
    connect(this,
            &DrawingScene::keyPressed,
            this,
            &DrawingScene::slot_keyPressed,
            Qt::QueuedConnection);
}

DrawingScene::~DrawingScene() {}

KromitItem *DrawingScene::find_item(int ID) const {
    for (auto &item : items()) {
        if (item->type() > GFX_SELECTABLE) {
            bool ok;
            auto id = item->data(KromitItem::KeyID).toInt(&ok);
            if (ok && id == ID) return static_cast<KromitItem *>(item);
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
            _current_item = new KromitPath(
                nullptr, Qt::yellow, pressure * 3.0, ARROW_DOUBLE, 10);
            KromitPath *path = static_cast<KromitPath *>(_current_item);
            path->start(p);
            path->move(p);
            addItem(_current_item);
            ev->accept();
            return true;
        }

        case QEvent::TabletMove: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            // double pressure = ev->pressure();
            if (_current_item) {
                KromitPath *path = static_cast<KromitPath *>(_current_item);
                path->move(ev->scenePosition());
            }
            ev->accept();
            return true;
        }

        case QEvent::TabletRelease: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            // double pressure = ev->pressure();
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
            emit pointerPressed(p, mods);
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
    KromitItem *kitem = static_cast<KromitItem *>(_current_item);

    if (kitem && _feed_keystrokes) {
        auto result =
            kitem->keystroke(keyEvent->keyCombination(), keyEvent->text());
        if (result == KromitItem::KEY_STOP) {
            _feed_keystrokes = false;
            _current_item = nullptr;
            return;
        }
        if (result == KromitItem::KEY_PROCESSED) return;
    }

    const auto key = keyEvent->key();
    const auto mods = keyEvent->modifiers();
    const auto text = keyEvent->text();
    emit keyPressed(key, mods, text);
}

// DrawingScene::pointerPressed
//
void DrawingScene::slot_pointerPressed(QPointF p, Qt::KeyboardModifiers mods) {
    if (_current_item && _current_item->type() == GFX_TEXT) {
        KromitText *textitem = static_cast<KromitText *>(_current_item);

        if (textitem->text().isEmpty()) {
            textitem->deselect();
            textitem->pgc();
            removeItemSafely(textitem);
        } else {
            textitem->finalize();
        }
    }
    if (_settings.tool == TOOL_SELECT) {
        SelectType sel = SEL_SINGLE;
        if (mods & Qt::ControlModifier)
            sel = SEL_TOGGLE;
        else if (mods & Qt::ShiftModifier)
            sel = SEL_ADD;
        if (!handle_transformation_clicks(p)) handle_selection_clicks(p, sel);
    } else {
        _current_item = Tools::create_tool(_settings);
        KromitItem *kitem = static_cast<KromitItem *>(_current_item);

        _feed_keystrokes = (kitem->caps() & CAP_KEYSTROKES);

        if (kitem->caps() & CAP_DYNAMIC) dyn_tick->start();

        redo_records.clear();
        garbage_collect();

        UndoRecord rec;
        rec.ids_to_remove.insert(kitem->data(KromitItem::KeyID).toInt());
        undo_records.emplace_back(rec);

        addItem(_current_item);
        kitem->start(p);
        kitem->move(p);
    }
}

void DrawingScene::slot_pointerMoved(QPointF p, Qt::KeyboardModifiers mods) {
    if (_settings.tool == TOOL_SELECT) {
        if (trans.is_ongoing()) { handle_transformation_move(p, mods); }
    } else {
        if (_current_item) {
            KromitItem *kitem = static_cast<KromitItem *>(_current_item);
            kitem->move(p);
        }
    }
}

void DrawingScene::slot_pointerReleased(QPointF p, Qt::KeyboardModifiers mods) {
    if (_settings.tool == TOOL_SELECT) {
        if (trans.is_ongoing()) { handle_transformation_release(p); }
    } else {
        if (_current_item) {
            KromitItem *kitem = static_cast<KromitItem *>(_current_item);
            kitem->stop(p);
            switch (kitem->type()) {
                case GFX_TEXT:
                    // special case of text item: keep item after mouse released
                    break;
                case GFX_PIXMAP: {
                    KromitPixmap *kpixmap = static_cast<KromitPixmap *>(kitem);
                    const QRect rect = kpixmap->sel_rect().toRect();
                    qDebug() << "requesting " << rect;
                    emit take_partial_screenshot(rect);
                } break;

                default:
                    _current_item = nullptr;
            }
        }
    }
}

void DrawingScene::slot_keyPressed(int key,
                                   Qt::KeyboardModifiers mods,
                                   QString text) {
    if (text == "<") {
        _settings.arrow_type = ARROW_START;
    } else if (text == ">") {
        _settings.arrow_type = ARROW_END;
    } else if (text == "=") {
      qDebug() << "arrow <-->";
        _settings.arrow_type = ARROW_DOUBLE;
    } else if (text == "-") {
        _settings.arrow_type = ARROW_NONE;
    }

        if (mods == Qt::ShiftModifier) {
        switch (key) {
            case Qt::Key_G: {
                _settings.tool = TOOL_PIXMAP;
                clear_selection();
                stash_scene();
                emit set_transparent();
            } break;
        }
    }

    if (mods == Qt::NoModifier) {
        switch (key) {
            case Qt::Key_Delete: {
                delete_selection();
            } break;

            case Qt::Key_1:
                _settings.width = 1;
                break;

            case Qt::Key_2:
                _settings.width = 2;
                break;

            case Qt::Key_3:
                _settings.width = 3;
                break;

            case Qt::Key_4:
                _settings.width = 5;
                break;

            case Qt::Key_5:
                _settings.width = 8;
                break;

            case Qt::Key_6:
                _settings.width = 12;
                break;

            case Qt::Key_7:
                _settings.width = 25;
                break;

            case Qt::Key_8:
                _settings.width = 40;
                break;

            case Qt::Key_9:
                _settings.width = 60;
                break;

            case Qt::Key_L: {
                _settings.tool = TOOL_LINE;
                clear_selection();
            } break;

            case Qt::Key_R: {
                _settings.tool = TOOL_RECT;
                clear_selection();
            } break;

            case Qt::Key_P: {
                _settings.tool = TOOL_PEN;
                clear_selection();
            } break;

            case Qt::Key_F: {
                _settings.tool = TOOL_FADER;
                clear_selection();
            } break;

            case Qt::Key_H: {
                _settings.tool = TOOL_HIGHLIGHT;
                clear_selection();
            } break;

        case Qt::Key_S: {
                _settings.tool = TOOL_SMOOTH;
                clear_selection();
            } break;

            case Qt::Key_E: {
                _settings.tool = TOOL_SELECT;
                clear_selection();
            } break;

            case Qt::Key_T: {
                _settings.tool = TOOL_TEXT;
                clear_selection();
            } break;

            case Qt::Key_D: {
                qDebug() << "---------- items ----------";
                for (const auto &item : items()) {
                    if (item->type() > GFX_SELECTABLE) {
                        std::cerr << static_cast<KromitItem &>(*item) << "\n";
                    } else {
                        qDebug() << "  " << KromitItem::type_txt(item->type())
                                 << item->isVisible() << item;
                    }
                }
                qDebug() << "-------- undo_store -------";
                for (const auto &it : undo_store) {
                    std::cerr << static_cast<KromitUndoItem &>(*it.second)
                              << "\n";
                }
                qDebug() << "------ undo_records -------";
                for (const auto &rec : undo_records) { std::cerr << rec; }
                qDebug() << "------ redo_records -------";
                for (const auto &rec : redo_records) { std::cerr << rec; }
                qDebug() << "---------------------------";

            } break;

            default:
                break;
        }
    }

    if (std::uint32_t col = Color::get_color(key, mods)) {
        _settings.color = QColor(
            col >> 24, (col >> 16) & 0xff, (col >> 8) & 0xff, col & 0xff);
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

// DrawingScene::delete_selection
//
// delete all items selected (and create an undo record)
//
void DrawingScene::delete_selection() {
    if (_selection_frame) {
        UndoRecord rec;
        for (auto &item : _selected_items) {
            int id = item->data(KromitFrame::KeyID).toInt();
            KromitUndoItem *undo_item = item->create_undo_item();
            item->deselect();
            item->pgc();
            removeItemSafely(item);
            // delete item;
            rec.ids_to_add.insert(id);
            undo_store.insert({id, undo_item});
        }
        undo_records.emplace_back(rec);
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
    trans.handle = KromitFrame::POS_NONE;
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

                UndoRecord rec;
                for (auto &item : _selected_items) {
                    KromitUndoItem *undo_item = item->create_undo_item();
                    const int old_id = item->data(KromitItem::KeyID).toInt();
                    item->set_new_ID();
                    const int new_id = item->data(KromitItem::KeyID).toInt();
                    undo_store.insert({old_id, undo_item});
                    rec.ids_to_add.insert(old_id);
                    rec.ids_to_remove.insert(new_id);
                    item->stash();
                }
                undo_records.emplace_back(rec);
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
        if (mods & Qt::ShiftModifier) { kx = ky = max<double>(kx, ky); }
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

// DrawingScene::handle_transformation_release
//
void DrawingScene::handle_transformation_release(const QPointF &p) {
    trans.m.reset();  // not really required
    update_selection_frame();
}

// DrawingScene::undo
//
// execute last undo record and place it in redo
//
void DrawingScene::undo() {
    clearSelection();
    if (undo_records.empty()) return;
    UndoRecord &rec = undo_records.back();
    for (auto id : rec.ids_to_remove) {
        KromitItem *ptr = find_item(id);
        if (ptr) {
            ptr->deselect();
            KromitUndoItem *undo_item = ptr->create_undo_item();
            ptr->pgc();
            removeItemSafely(ptr);
            // delete ptr;
            undo_store.insert({id, undo_item});
        } else {
            qDebug() << "undo: Could not find item" << id << "ptr=" << ptr;
        }
    }
    for (auto id : rec.ids_to_add) {
        auto it = undo_store.find(id);
        if (it != undo_store.end()) {
            KromitItem *item = it->second->create_from_undo_item();
            addItem(item);
            undo_store.erase(it);
        } else {
            qDebug() << "undo: Could not find item" << id;
        }
    }
    redo_records.emplace_back(rec);
    undo_records.pop_back();
    clearSelection();
    update_selection_frame();
}

// DrawingScene::undo
//
// execute last redo record and place it in undo
//
void DrawingScene::redo() {
    clearSelection();
    if (redo_records.empty()) return;
    UndoRecord &rec = redo_records.back();
    for (auto id : rec.ids_to_add) {
        KromitItem *ptr = find_item(id);
        if (ptr) {
            ptr->deselect();
            KromitUndoItem *redo_item = ptr->create_undo_item();
            ptr->pgc();
            removeItemSafely(ptr);
            //            delete ptr;
            undo_store.insert({id, redo_item});
        } else {
            qDebug() << "redo: Could not find item" << id << "ptr=" << ptr;
        }
    }
    for (auto id : rec.ids_to_remove) {
        auto it = undo_store.find(id);
        if (it != undo_store.end()) {
            KromitItem *item = it->second->create_from_undo_item();
            addItem(item);
            undo_store.erase(it);
        } else {
            qDebug() << "redo: Could not find item" << id;
        }
    }
    undo_records.emplace_back(rec);
    redo_records.pop_back();
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
    int last_n = 0;
    return;
    for (;;) {
        int new_n =
            undo_store.size() + undo_records.size() + redo_records.size();
        if (new_n == last_n) break;
        last_n = new_n;

        // remove records from undo_store that are no longer references by
        // any undo or redo operation
        std::unordered_set<int> ids_in_undoredo;
        for (const auto &rec : undo_records) {
            for (auto v : rec.ids_to_add) ids_in_undoredo.insert(v);
            for (auto v : rec.ids_to_remove) ids_in_undoredo.insert(v);
        }
        for (const auto &rec : redo_records) {
            for (auto v : rec.ids_to_add) ids_in_undoredo.insert(v);
            for (auto v : rec.ids_to_remove) ids_in_undoredo.insert(v);
        }

        std::erase_if(undo_store, [&](auto &v) -> bool {
            return !ids_in_undoredo.contains(v.first);
        });

        // remove undo or redo operations that reference objects that no
        // longer exist in items() or undo_store
        std::unordered_set<int> ids_in_stores;
        for (const auto &item : items())
            ids_in_stores.insert(item->data(KromitItem::KeyID).toInt());

        for (const auto &item : undo_store)
            ids_in_stores.insert(item.second->id);

        std::erase_if(undo_records, [&](auto &rec) -> bool {
            return !rec.all_ids_in(ids_in_stores);
        });
        std::erase_if(redo_records, [&](auto &rec) -> bool {
            return !rec.all_ids_in(ids_in_stores);
        });
    }
}

void DrawingScene::removeItemSafely(QGraphicsItem *item) {
    while (item->childItems().size() > 0)
        removeItemSafely(item->childItems().first());
    removeItem(item);
    delete item;
}

void DrawingScene::tab() {
    // toggle dash
}

void DrawingScene::slot_pixmap(const QPixmap &pixmap) {
    qDebug() << "received pixmap" << pixmap.rect();
    if (_current_item) {
        switch (_current_item->type()) {
            case GFX_PIXMAP: {
                KromitPixmap *kpixmap = static_cast<KromitPixmap *>(_current_item);
                kpixmap->set_pixmap(pixmap, kpixmap->sel_rect());
                _current_item = nullptr;
                unstash_scene();
            } break;
        }
    }
}

void DrawingScene::stash_scene() {
    clearSelection();

    stash.stash.clear();
    if (_background_item) {
        stash.background = _background_item->pixmap();
        clear_background();
    } else {
        stash.background = QPixmap();
    }

    for (auto &it : items()) {
        if (it->type() > GFX_SELECTABLE) {
            KromitItem *kitem = static_cast<KromitItem *>(it);
            auto id = kitem->data(KromitItem::KeyID).toInt();
            KromitUndoItem *undo_item = kitem->create_undo_item();
            kitem->pgc();
            removeItemSafely(kitem);
            stash.stash.insert({id, undo_item});
        } else {
        }
    }
}

void DrawingScene::unstash_scene() {
    clear_selection();

    clear_background();
    if (!stash.background.isNull()) set_background(stash.background);

    for (auto &it : stash.stash) {
        KromitItem *item = it.second->create_from_undo_item();
        addItem(item);
    }
    stash.stash.clear();
}

void DrawingScene::fix_geometry(QRect real_rect) {
    // this only yields the top left corner correctly...
    QRect r = QGuiApplication::primaryScreen()->geometry();
    real_rect.adjust(-r.left(), -r.top(), -r.left(), -r.top());
    setSceneRect(real_rect);
}

void DrawingScene::set_background(QPixmap px) {
    clear_background();
    _background_item = addPixmap(px);
    _background_item->setOffset(0, 0);
    _background_item->setZValue(-1000.0);
}

void DrawingScene::clear_background() {
    if (_background_item) {
        removeItem(_background_item);
        _background_item = nullptr;
    }
}

void DrawingScene::slot_dynamic_updates() {
    bool any_remaining = false;
    bool any_deleted = true;
    bool selected_deleted = false;
    for (auto &it : items()) {
        if (it->type() > GFX_SELECTABLE) {
            KromitItem *kitem = static_cast<KromitItem *>(it);
            if (kitem->caps() & CAP_DYNAMIC) {
                any_remaining = true;
                auto result = kitem->dynamic();
                if (result == KromitItem::DYN_DELITEM) {
                    any_deleted = true;
                    if (_selected_items.contains(kitem))
                        selected_deleted = true;
                    removeItemSafely(kitem);
                }
            }
        }
    }
    if (any_remaining == false) dyn_tick->stop();
    if (selected_deleted) update_selection_frame();
}
