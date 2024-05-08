#ifndef _DRAWINGVIEW_H
#define _DRAWINGVIEW_H

#include <QGraphicsView>
#include "kromit.h"

class DrawingScene;

class DrawingView : public QGraphicsView {
    Q_OBJECT
  public:
    explicit DrawingView(DrawingScene *scene, QWidget *parent = nullptr);
    ~DrawingView();

  public slots:
    void slot_tool_changed(ToolType tool);
    void slot_screen_rect_changed(QRect rect);

  protected:
    virtual bool event(QEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

  private:
    QRect screen_rect;

    QCursor cursor_text;
    QCursor cursor_smooth;
    QCursor cursor_line;
    QCursor cursor_ortho;
    QCursor cursor_pen;
    QCursor cursor_rect;
    QCursor cursor_fader;
    QCursor cursor_highlight;
    QCursor cursor_grab;
    QCursor cursor_select;
};

#endif
