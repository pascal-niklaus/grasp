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
class QGraphicsOpacityEffect;
class QTimer;
class PipewireScreenshot;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QSettings &qset, int verbosity = 0, QWidget *parent = nullptr);
    ~MainWindow();

  public slots:
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

    // Dims/restores the canvas opacity (see slot_cycle_opacity()).
    // Implemented with a QGraphicsOpacityEffect on _view rather than
    // QWidget::setWindowOpacity() - the latter has no effect under most
    // Wayland compositors, since there is no standard protocol for
    // whole-surface opacity there.
    void slot_set_see_through(double opacity);

    void slot_undo();
    void slot_redo();

    void slot_save(const QString &filename);

  private slots:
    void slot_screenshot_step1();
    void slot_screen_changed();

    void slot_portal_grab_finished(const QPixmap &pixmap);
    void slot_portal_grab_failed(const QString &reason);

  private:
    enum class GrabKind { None, Full };

    DrawingScene *_scene = nullptr;
    DrawingView *_view = nullptr;

    bool _on_top = true;
    double _opacity = 1.0;
    QRect _screen_rect;
    double _pixel_ratio = 1.0;

    QTimer *_snapshot_timer = nullptr;
    bool _use_portal = false;
    PipewireScreenshot *_portal = nullptr;
    GrabKind _pending_grab = GrabKind::None;

    QGraphicsOpacityEffect *_transparency_effect = nullptr;
};

#endif // MAINWINDOW_H
