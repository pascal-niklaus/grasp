#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include <qnamespace.h>
#include <QGraphicsItem>
#include <QTransform>
#include <QGraphicsScene>

#include "drawingview.h"
#include "kromit.h"
#include "kromitframe.h"
#include "kromititem.h"
#include "tools.h"
#include "undo.h"

class QGraphicsItemGroup;

class Transformation {
  public:
    Transformation() { clear(); }
    // clear
    void clear() { handle = KromitFrame::POS_NONE; }
    // is any operation ongoing?
    bool is_ongoing() const { return handle != KromitFrame::POS_NONE; }
    // type of operation:
    //
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

class DrawingScene : public QGraphicsScene {
    Q_OBJECT
  public:
    explicit DrawingScene(QObject *parent = nullptr);
    ~DrawingScene();

    void redo();
    void undo();

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mouseReleaseEvent(
        QGraphicsSceneMouseEvent *mouseEvent) override;

    virtual void mouseDoubleClickEvent(
        QGraphicsSceneMouseEvent *mouseEvent) override;

    virtual bool event(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *keyEvent) override;

  private:
    void update_selection_frame();
    void delete_selection();

    KromitItem *find_item(int id) const;
    KromitItem *find_undo_item(int id) const;
    bool delete_undo_item(int id);

    void handle_selection_clicks(const QPointF &p, SelectType sel);
    bool handle_transformation_clicks(const QPointF &p);
    void handle_transformation_move(const QPointF &p,
                                    Qt::KeyboardModifiers mods);
    void handle_transformation_release(const QPointF &p);

  private:
    Tools::Settings _settings;
    //    bool select = false;
    QGraphicsItem *_current_item = nullptr;
    KromitFrame *_selection_frame = nullptr;
    QList<KromitItem *> _selected_items;
    Transformation trans;

    std::vector<KromitItem *> undo_store;
    std::vector<UndoRecord> undo_records;
    std::vector<UndoRecord> redo_records;
};

#endif // DRAWINGSCENE_H
