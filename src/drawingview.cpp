/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "drawingview.h"
#include "drawingscene.h"
#include "screenres.h"

#include <QEvent>
#include <QApplication>
#include <QDirIterator>
#include <QResizeEvent>

DrawingView::DrawingView(DrawingScene *scene, int verbosity, QWidget *parent) :
  QGraphicsView(scene, parent) , _scene(scene) {
    _verbosity = verbosity;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(scene, &DrawingScene::tool_changed, this, &DrawingView::slot_tool_changed);
    // setCacheMode(QGraphicsView::CacheBackground);
    // setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QPixmap px;
    px = QPixmap(":/cursors/cursors/text.png");
    _cursor_text = QCursor(px, 15, 25);

    px = QPixmap(":/cursors/cursors/smooth.png");
    _cursor_smooth = QCursor(px);

    px = QPixmap(":/cursors/cursors/line.png");
    _cursor_line = QCursor(px);

    px = QPixmap(":/cursors/cursors/ortho.png");
    _cursor_ortho = QCursor(px);

    px = QPixmap(":/cursors/cursors/rect.png");
    _cursor_rect = QCursor(px);

    px = QPixmap(":/cursors/cursors/pen.png");
    _cursor_pen = QCursor(px);

    px = QPixmap(":/cursors/cursors/fader.png");
    _cursor_fader = QCursor(px);

    px = QPixmap(":/cursors/cursors/highlight.png");
    _cursor_highlight = QCursor(px);

    px = QPixmap(":/cursors/cursors/grab.png");
    _cursor_grab = QCursor(px);

    _cursor_select = QCursor(Qt::ArrowCursor);

    _screen_watcher = new ScreenChangeDetector(this);
    connect(_screen_watcher,
            &ScreenChangeDetector::screen_pixels_changed,
            this,
            &DrawingView::slot_screen_rect_changed);
}

DrawingView::~DrawingView() {
    // all allocated objects should be deleted automatically because the are
    // child QObjects
}

// DrawingView::resizeEvent
//
// detect size change events and adjust scene accordingly
//
void DrawingView::resizeEvent(QResizeEvent *event) {
    const auto sz = event->size();
    auto rect = _screen_watcher->rect_pixels();
    if (sz.width() > 500 && rect.width() > 500) {
        fitInView(rect);
        _scene->slot_reposition_dash(rect.width());
    }
}

// DrawingView::slot_screen_rect_changed(
//
// this is called when the screen resolution changes, for example,
// because an external screen is connected. I think resizeEvent does
// not cover all cases, but maybe there is some redundancy here...
//
void DrawingView::slot_screen_rect_changed(QRect rect) {
    rect.adjust(1, 1, -1, -1);
    setSceneRect(rect);
    fitInView(rect);
    _scene->slot_reposition_dash(rect.width());
}

// DrawingView::event
//
// Forward tablet events to scene -- TODO: check if this is really needed?
//
bool DrawingView::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::TabletPress:
            [[fallthrough]];
        case QEvent::TabletRelease:
            [[fallthrough]];
        case QEvent::TabletMove:
            QApplication::sendEvent(scene(), event);
            return true;
        default:
            return QGraphicsView::event(event);
    }
}

// DrawingView::slot_tool_changed
//
// the scene emits a signal when the tool is changed... the view then
// changes the cursor
//
void DrawingView::slot_tool_changed(ToolType tool) {
    switch (tool) {
        case TOOL_SMOOTH:
            viewport()->setCursor(_cursor_smooth);
            break;
        case TOOL_LINE:
            viewport()->setCursor(_cursor_line);
            break;
        case TOOL_ORTHO:
            viewport()->setCursor(_cursor_ortho);
            break;
        case TOOL_PEN:
            viewport()->setCursor(_cursor_pen);
            break;
        case TOOL_TEXT:
            viewport()->setCursor(_cursor_text);
            break;
        case TOOL_RECT:
            viewport()->setCursor(_cursor_rect);
            break;
        case TOOL_SELECT:
            viewport()->setCursor(_cursor_select);
            break;
        case TOOL_PIXMAP:
            viewport()->setCursor(_cursor_grab);
            break;
        case TOOL_FADER:
            viewport()->setCursor(_cursor_fader);
            break;
        case TOOL_HIGHLIGHT:
            viewport()->setCursor(_cursor_highlight);
            break;
        default:
            viewport()->setCursor(Qt::ArrowCursor);
            break;
    }
}
