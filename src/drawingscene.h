#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include <QGraphicsScene>

#include "drawingview.h"
#include "kromit.h"
#include "kromitframe.h"
#include "kromititem.h"
#include "tools.h"

class DrawingScene : public QGraphicsScene {
    Q_OBJECT
  public:
    explicit DrawingScene(QObject *parent = nullptr);
    ~DrawingScene();

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    virtual void mouseDoubleClickEvent(
        QGraphicsSceneMouseEvent *mouseEvent) override;

    virtual bool event(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *keyEvent) override;

  private:
    void update_selection_frame();
    void delete_selection();

  private:
    Tools::Settings _settings;
    //    bool select = false;
    QGraphicsItem *_current_item = nullptr;
    KromitFrame *_selection_frame = nullptr;
    QList<KromitItem *> _selected_items;
};

#endif // DRAWINGSCENE_H
