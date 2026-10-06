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

#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include <QGraphicsScene>

#include "gfxdash.h"
#include "gfxframe.h"
#include "undo.h"

class QGraphicsItemGroup;
class QMenu;
class QSettings;

class Transformation {
  public:
    Transformation() { clear(); }
    // clear
    void clear() { handle = GfxFrame::POS_NONE; }
    // is any operation ongoing?
    bool is_ongoing() const { return handle != GfxFrame::POS_NONE; }
    // which of the handles is dragged?
    GfxFrame::HandlePos handle;
    // where was cursor when it was clicked?
    QPointF pos_start;
    // where was the selection frame when it was clicked?
    QPointF pos_frame_start;
    // position of handle that was clicked
    QPointF pos_cur;
    // position of oppositve handle
    QPointF pos_ref;
    // transformation matrix
    QTransform m;
};

// SavedScene
//
// contains the entire scene including undo/redo history
//
// stash: the items to put in the drawingscene
// undo_store: the undo_store at the time of saving
// undo/redo_records: these records at the time of saving
// settings: all tool settings at the time of saving
class SavedScene {
  public:
    void free_items() {
        std::cerr << "SavedScene::free_items " << this << "\n";
        for (auto &it : undo_store) {
            std::cerr << "- del " << it.second << "\n";
            delete it.second;
        }
        for (auto &it : gfxitems) {
            std::cerr << "- del " << it.second << "\n";
            delete it.second;
        }
        std::cerr << "SavedScene::free_items -- end\n";
    }

    // the visible items
    QPixmap background;
    std::unordered_map<int, GfxUndoItem *> gfxitems;
    // invisible items
    std::unordered_map<int, GfxUndoItem *> undo_store;
    std::vector<UndoRecord> undo_records;
    std::vector<UndoRecord> redo_records;
    Settings settings;
};

// DrawingScene
//
class DrawingScene : public QGraphicsScene {
    Q_OBJECT
  private:
    enum MenuItem : int { MENU_CANCEL = 0, MENU_DELETE, MENU_NEW, MENU_SAVE };

  public:
    explicit DrawingScene(QSettings &qset,
                          int verbosity = 0,
                          QObject *parent = nullptr);
    ~DrawingScene();

    double grab_delay() const { return _settings.grab_delay; }

    void redo();
    void undo();
    void tab();

    // begin/abort a partial screen grab: see slot_keyPressed() for the
    // full explanation. Public so both DrawingScene's own event
    // handling and MainWindow (D-Bus-triggered commands) can call
    // abort_partial_grab() uniformly whenever some other action starts.
    void start_partial_grab();
    void abort_partial_grab();

    // save/restore entire scenes and context
    void clear_scene();

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mouseReleaseEvent(
        QGraphicsSceneMouseEvent *mouseEvent) override;

    virtual void mouseDoubleClickEvent(
        QGraphicsSceneMouseEvent *mouseEvent) override;

    virtual bool event(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *keyEvent) override;

  signals:
    void keyPressed(QKeyCombination, QString);
    void pointerPressed(QPointF, Qt::MouseButtons, Qt::KeyboardModifiers);
    void pointerMoved(QPointF, Qt::KeyboardModifiers);
    void pointerReleased(QPointF, Qt::KeyboardModifiers);
    void tool_changed(ToolType);
    void save_scene(int id);
    void restore_scene(int id);
    void set_color(const QColor &color);
    void set_arrow(ArrowType type);
    void delete_scene(int id);
    void setup();
    void take_screenshot();
    void quit();
    void toggle();
    void opacity();

  public slots:
    void slot_new();
    void slot_set_background(QPixmap px);
    void slot_clear_background();
    void slot_grab_failed();  // abort a pending partial grab if one was outstanding
    void slot_set_color(const QColor &color);
    void slot_set_color_from_index(int index);

    void slot_set_tool(ToolType tool);
    void slot_set_arrow(ArrowType type);
    void slot_set_width(int width);
    void slot_setup();
    void slot_reposition_dash(int width);

    void slot_add_rect(const QRectF &rect, int radius);

  private slots:
    void slot_keyPressed(QKeyCombination kc, QString text);
    void slot_pointerPressed(QPointF p,
                             Qt::MouseButtons buttons,
                             Qt::KeyboardModifiers mods);
    void slot_pointerMoved(QPointF p, Qt::KeyboardModifiers mods);
    void slot_pointerReleased(QPointF p, Qt::KeyboardModifiers mods);

    void slot_dynamic_updates();

    void slot_save_scene(int id);
    void slot_restore_scene(int id);
    void slot_delete_scene(int id);

  private:
    void update_selection_frame();
    void delete_selection();
    void clear_selection();
    void garbage_collect();
    void init_tool();
    void abort_current_item();
    void finish_text_edit();

    // shows px as a temporary, full-scene, high-z overlay for partial
    // grab's region selection (see start_partial_grab()); only called
    // from slot_set_background() when _partial_grab_active.
    void show_grab_overlay(const QPixmap &px);

    GfxItem *find_item(int id) const;

    bool handle_dash_clicks(const QPointF &p,
                            const Qt::MouseButtons buttons,
                            const Qt::KeyboardModifiers mods);
    void handle_selection_clicks(const QPointF &p, SelectType sel);
    bool handle_transformation_clicks(const QPointF &p);
    void handle_transformation_move(const QPointF &p,
                                    Qt::KeyboardModifiers mods);
    void handle_transformation_release(const QPointF &p);

    void removeItemSafely(QGraphicsItem *item);

    int new_scene_id(void);

  private:
    QSettings &_qset;
    Settings _settings;
    GfxDash *_dash;
    QGraphicsPixmapItem *_icon;

    QGraphicsItem *_current_item = nullptr;
    QGraphicsPixmapItem *_background_item = nullptr;
    ToolType _pre_grab_tool = TOOL_PEN;

    bool _feed_keystrokes = false;
    QTimer *_dyn_tick = nullptr;
    QMenu *_rmb_menu = nullptr;

    GfxFrame *_selection_frame = nullptr;
    QList<GfxItem *> _selected_items;
    Transformation _trans;

    // undo records
    std::unordered_map<int, GfxUndoItem *> _undo_store;
    std::vector<UndoRecord> _undo_records;
    std::vector<UndoRecord> _redo_records;

    // partial grab: a temporary full-screen overlay the user selects a
    // region from; the real scene is never touched, so there's nothing
    // to restore if this is interrupted -- see start_partial_grab(). The
    // dash is hidden rather than just drawn-under, so the grabbed region
    // can include the dash's own screen area too; _dash_was_visible
    // remembers whether to show it again on abort. _partial_grab_timer
    // is a safety net that auto-aborts if the user never completes or
    // cancels the selection (e.g. forgets, confused by the missing dash).
    QGraphicsPixmapItem *_grab_overlay = nullptr;
    bool _partial_grab_active = false;
    bool _dash_was_visible = true;
    QTimer *_partial_grab_timer = nullptr;

    // scenes
    int _current_scene = 0;
    std::unordered_map<int, SavedScene> _saved_scenes;
};

#endif  // DRAWINGSCENE_H
