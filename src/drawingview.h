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
