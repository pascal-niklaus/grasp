#ifndef _DRAWINGVIEW_H
#define _DRAWINGVIEW_H

#include <QGraphicsView>

class DrawingScene;

class DrawingView : public QGraphicsView {
  Q_OBJECT
  public:
    explicit DrawingView(DrawingScene *scene, QWidget *parent = nullptr);
    ~DrawingView();

  protected:
    virtual bool event(QEvent *event);
};

#endif
