#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include <QGraphicsScene>

#include "kromit.h"
#include "drawingview.h"
#include "tools.h"

class DrawingScene : public QGraphicsScene {
    Q_OBJECT
  public:
    explicit DrawingScene(QObject *parent = nullptr);
    ~DrawingScene();

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual bool event(QEvent *event);

  private:
    QPointF mapped(QPointF p) const;

  private:
    Tools::Settings _settings;

    QGraphicsItem *current_item = nullptr;
};

#endif // DRAWINGSCENE_H
