#ifndef _DRAWINGVIEW_H
#define _DRAWINGVIEW_H

#include <QGraphicsView>
#include "grasp.h"

class DrawingScene;
class ScreenChangeDetector;

class DrawingView : public QGraphicsView {
    Q_OBJECT
  public:
    explicit DrawingView(DrawingScene *scene,
                         int verbosity = 0,
                         QWidget *parent = nullptr);
    ~DrawingView();

  public slots:
    void slot_tool_changed(ToolType tool);
    void slot_screen_rect_changed(QRect rect);

  protected:
    virtual bool event(QEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

  private:
    int _verbosity;
    DrawingScene *_scene;
    ScreenChangeDetector *_screen_watcher;

    QCursor _cursor_text;
    QCursor _cursor_smooth;
    QCursor _cursor_line;
    QCursor _cursor_ortho;
    QCursor _cursor_pen;
    QCursor _cursor_rect;
    QCursor _cursor_fader;
    QCursor _cursor_highlight;
    QCursor _cursor_grab;
    QCursor _cursor_select;
};

#endif
