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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>

#include "drawingscene.h"
#include "drawingview.h"

class QAction;
class QGraphicsScene;
class QGraphicsView;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QSettings &qset, int verbosity = 0, QWidget *parent = nullptr);
    ~MainWindow();

  signals:
    void send_pixmap(const QPixmap &);

  public slots:
    void slot_grab_partial(const QRect &geometry);

    void slot_quit();

    void slot_toggle();
    void slot_show();
    void slot_hide();

    void slot_grab();
    void slot_grab_and_new();

    void slot_change_color(int index);
    void slot_change_tool(const QString &tool);
    void slot_change_arrow(const QString &arrow);
    void slot_change_width(int width);

    void slot_rect(const QRectF &rect, int radius);

    // see-through for debugging
    void slot_cycle_opacity();

    void slot_undo();
    void slot_redo();

    void slot_save(const QString &filename);

  private slots:
    void slot_screenshot_step1();
    void slot_partial_screenshot_step1();
    void slot_screen_changed();

  private:
    DrawingScene *_scene = nullptr;
    DrawingView *_view = nullptr;

    bool _on_top = true;
    double _opacity = 1.0;
    QRect _screen_rect;
    QRect _shot_rect;
    double _pixel_ratio = 1.0;

    QTimer *_snapshot_timer = nullptr;
    QTimer *_partial_snapshot_timer = nullptr;
};

#endif // MAINWINDOW_H

