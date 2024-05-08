#include "drawingview.h"
#include "drawingscene.h"

#include <QEvent>
#include <QApplication>
#include <QDirIterator>
#include <QResizeEvent>

DrawingView::DrawingView(DrawingScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(scene, &DrawingScene::tool_changed, this, &DrawingView::slot_tool_changed);
    // setCacheMode(QGraphicsView::CacheBackground);
    // setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    // QDirIterator it(":", QDirIterator::Subdirectories);
    // while (it.hasNext()) { qDebug() << it.next(); }

    QPixmap px;
    px = QPixmap(":/cursors/cursors/text.png");
    cursor_text = QCursor(px, 15, 25);

    px = QPixmap(":/cursors/cursors/smooth.png");
    cursor_smooth = QCursor(px);

    px = QPixmap(":/cursors/cursors/line.png");
    cursor_line = QCursor(px);

    px = QPixmap(":/cursors/cursors/ortho.png");
    cursor_ortho = QCursor(px);

    px = QPixmap(":/cursors/cursors/rect.png");
    cursor_rect = QCursor(px);

    px = QPixmap(":/cursors/cursors/pen.png");
    cursor_pen = QCursor(px);

    px = QPixmap(":/cursors/cursors/fader.png");
    cursor_fader = QCursor(px);

    px = QPixmap(":/cursors/cursors/highlight.png");
    cursor_highlight = QCursor(px);

    px = QPixmap(":/cursors/cursors/grab.png");
    cursor_grab = QCursor(px);

    cursor_select = QCursor(Qt::ArrowCursor);
}

DrawingView::~DrawingView() {}

// DrawingView::resizeEvent
//
// detect size change events and adjust scene accordingly
//
void DrawingView::resizeEvent(QResizeEvent *event) {
    const auto sz = event->size();
    if (sz.width() > 500 && screen_rect.width() > 500) fitInView(screen_rect);
}

bool DrawingView::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TabletMove:
            QApplication::sendEvent(scene(), event);
            return true;
        default:
            return QGraphicsView::event(event);
    }
}

void DrawingView::slot_screen_rect_changed(QRect rect) {
    qDebug() << "view::screen_rect_changed" << rect << width();
    rect.adjust(1, 1, -1, -1);
    screen_rect = rect;
    setSceneRect(rect);
    fitInView(rect);
}

void DrawingView::slot_tool_changed(ToolType tool) {
    switch (tool) {
        case TOOL_SMOOTH:
          //            viewport()->setCursor(Qt::CrossCursor);
          viewport()->setCursor(cursor_smooth);
          break;
        case TOOL_LINE:
            viewport()->setCursor(cursor_line);
            break;
        case TOOL_ORTHO:
            viewport()->setCursor(cursor_ortho);
            break;
        case TOOL_PEN:
            viewport()->setCursor(cursor_pen);
            break;
        case TOOL_TEXT:
            viewport()->setCursor(cursor_text);
            break;
        case TOOL_RECT:
            viewport()->setCursor(cursor_rect);
            break;
        case TOOL_SELECT:
            viewport()->setCursor(cursor_select);
            break;
        case TOOL_PIXMAP:
            viewport()->setCursor(cursor_grab);
            break;
        case TOOL_FADER:
            viewport()->setCursor(cursor_fader);
            break;
        case TOOL_HIGHLIGHT:
            viewport()->setCursor(cursor_highlight);
            break;
        default:
            viewport()->setCursor(Qt::ArrowCursor);
            break;
    }
}
