#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include "kromitframe.h"
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

class Stash {
  public:
    QPixmap background;
    std::unordered_map<int, KromitUndoItem *> stash;
};

class DrawingScene : public QGraphicsScene {
    Q_OBJECT
  public:
    explicit DrawingScene(QObject *parent = nullptr);
    ~DrawingScene();

    void fix_geometry(QRect real_rect);

    void redo();
    void undo();
    void tab();

    void stash_scene();
    void unstash_scene();

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

  public slots:
    void slot_pixmap(const QPixmap &pixmap);
    void set_background(QPixmap px);
    void clear_background();

  private slots:
    void slot_keyPressed(int key, Qt::KeyboardModifiers mods, QString text);
    void slot_pointerPressed(QPointF p, Qt::KeyboardModifiers mods);
    void slot_pointerMoved(QPointF p, Qt::KeyboardModifiers mods);
    void slot_pointerReleased(QPointF p, Qt::KeyboardModifiers mods);

    void slot_dynamic_updates();

  private:
    void update_selection_frame();
    void delete_selection();
    void clear_selection();
    void garbage_collect();
    void init_tool();

    KromitItem *find_item(int id) const;

    void handle_selection_clicks(const QPointF &p, SelectType sel);
    bool handle_transformation_clicks(const QPointF &p);
    void handle_transformation_move(const QPointF &p,
                                    Qt::KeyboardModifiers mods);
    void handle_transformation_release(const QPointF &p);

    void removeItemSafely(QGraphicsItem *item);

  private:
    Tools::Settings _settings;
    //    bool select = false;
    QGraphicsItem *_current_item = nullptr;
    QGraphicsPixmapItem *_background_item = nullptr;
    bool _feed_keystrokes = false;
    KromitFrame *_selection_frame = nullptr;
    QList<KromitItem *> _selected_items;
    Transformation trans;

    QTimer *dyn_tick = nullptr;

    std::unordered_map<int, KromitUndoItem *> undo_store;
    std::vector<UndoRecord> undo_records;
    std::vector<UndoRecord> redo_records;

    // temporarily store entire scene
    Stash stash;
};

#endif  // DRAWINGSCENE_H
