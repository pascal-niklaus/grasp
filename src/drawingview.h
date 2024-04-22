#ifndef _DRAWINGVIEW_H
#define _DRAWINGVIEW_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QObject>
#include <QGraphicsView>

class DrawingScene;

class DrawingView : public QGraphicsView {
  Q_OBJECT
  public:
    explicit DrawingView(DrawingScene *scene, QWidget *parent = nullptr);
    ~DrawingView();

  public slots:
    void set_background(QPixmap px);

  protected:
    virtual bool event(QEvent *event);

  private:
    QGraphicsPixmapItem *background_item = nullptr;
};

#endif
