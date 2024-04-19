#ifndef _DRAWINGVIEW_H
#define _DRAWINGVIEW_H

#include <qgraphicsview.h>
#include <QGraphicsView>
#include "drawingscene.h"

class DrawingScene;

class DrawingView : public QGraphicsView {
  public:
    explicit DrawingView(DrawingScene *scene, QWidget *parent = nullptr);

  protected:
    virtual bool event(QEvent *event);
};

#endif
