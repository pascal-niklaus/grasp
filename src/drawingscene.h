#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include "kromitframe.h"
#include "kromitdash.h"
#include "undo.h"

#include <QGraphicsScene>

class QGraphicsItemGroup;

class Transformation {
  public:
    Transformation() { clear(); }
    // clear
    void clear() { handle = KromitFrame::POS_NONE; }
    // is any operation ongoing?
    bool is_ongoing() const { return handle != KromitFrame::POS_NONE; }
    // which of the handles is dragged?
    KromitFrame::HandlePos handle;
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

// Stash
//
// items temporarily removed during screen grabbing
//
class Stash {
  public:
    QPixmap background;
    std::unordered_map<int, KromitUndoItem *> stash;
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
    std::unordered_map<int, KromitUndoItem *> gfxitems;
    // invisible items
    std::unordered_map<int, KromitUndoItem *> undo_store;
    std::vector<UndoRecord> undo_records;
    std::vector<UndoRecord> redo_records;
    Tools::Settings settings;
};

// DrawingScene
//
class DrawingScene : public QGraphicsScene {
    Q_OBJECT
  public:
    explicit DrawingScene(QObject *parent = nullptr);
    ~DrawingScene();

    void fix_geometry(QRect real_rect);

    void redo();
    void undo();
    void tab();

    // stash/unstash during grabbing
    void stash_scene();
    void unstash_scene();

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
    void keyPressed(int, Qt::KeyboardModifiers, QString);
    void pointerPressed(QPointF, Qt::KeyboardModifiers);
    void pointerMoved(QPointF, Qt::KeyboardModifiers);
    void pointerReleased(QPointF, Qt::KeyboardModifiers);
    void take_partial_screenshot(const QRect &rect);
    void set_transparent();
    void set_opaque();
    void tool_changed(ToolType);
    void save_scene(int id);
    void restore_scene(int id);
    void set_color(const QColor &color);
    void set_arrow(ArrowType type);

  public slots:
    void slot_pixmap(const QPixmap &pixmap);
    void set_background(QPixmap px);
    void clear_background();
    void slot_set_color(const QColor &color);
    void slot_set_arrow(ArrowType type);

  private slots:
    void slot_keyPressed(int key, Qt::KeyboardModifiers mods, QString text);
    void slot_pointerPressed(QPointF p, Qt::KeyboardModifiers mods);
    void slot_pointerMoved(QPointF p, Qt::KeyboardModifiers mods);
    void slot_pointerReleased(QPointF p, Qt::KeyboardModifiers mods);

    void slot_dynamic_updates();

    void slot_save_scene(int id);
    void slot_restore_scene(int id);

  private:
    void update_selection_frame();
    void delete_selection();
    void clear_selection();
    void garbage_collect();
    void init_tool();

    KromitItem *find_item(int id) const;

  bool handle_dash_clicks(const QPointF &p);
  void handle_selection_clicks(const QPointF &p, SelectType sel);
  bool handle_transformation_clicks(const QPointF &p);
  void handle_transformation_move(const QPointF &p, Qt::KeyboardModifiers mods);
  void handle_transformation_release(const QPointF &p);

  void removeItemSafely(QGraphicsItem *item);

private:
  Tools::Settings _settings;
  KromitDash *_dash;

  QGraphicsItem *_current_item = nullptr;

  QGraphicsPixmapItem *_background_item = nullptr;

  bool _feed_keystrokes = false;
  QTimer *dyn_tick = nullptr;

  KromitFrame *_selection_frame = nullptr;
  QList<KromitItem *> _selected_items;
  Transformation trans;

  // undo records
  std::unordered_map<int, KromitUndoItem *> undo_store;
  std::vector<UndoRecord> undo_records;
  std::vector<UndoRecord> redo_records;

  // temporarily stash entire scene here during grabbing
  Stash stash;

  // saved scenes
    std::unordered_map<int, SavedScene> saved_scenes;
};

#endif  // DRAWINGSCENE_H
