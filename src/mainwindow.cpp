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

#include "drawingscene.h"
#include "drawingview.h"
#include "mainwindow.h"
#include "screenres.h"
#include "verbosity.h"

#include <QAction>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMainWindow>
#include <QPixmap>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <QGuiApplication>
#include <QWindow>

#include <iostream>

// MainWindow::MainWindow
//
MainWindow::MainWindow(QSettings &qset, int verbosity, QWidget *parent) :
    QMainWindow(parent) {

    // graphics scene
    _scene = new DrawingScene(qset, verbosity);
    _view = new DrawingView(_scene, verbosity);
    setCentralWidget(_view);

    new ScreenChangeDetector(this);

    slot_screen_changed();

    // setup timer for snapshot delay
    _snapshot_timer = new QTimer(this);
    _snapshot_timer->setSingleShot(true);
    _snapshot_timer->setInterval(10);
    connect(_snapshot_timer,
            &QTimer::timeout,
            this,
            &MainWindow::slot_screenshot_step1);

    // setup timer for pixmap grabbing
    _partial_snapshot_timer = new QTimer(this);
    _partial_snapshot_timer->setSingleShot(true);
    _partial_snapshot_timer->setInterval(10);

    // clang-format off

    connect(_partial_snapshot_timer, &QTimer::timeout,
            this, &MainWindow::slot_partial_screenshot_step1);
    connect(this, &MainWindow::send_pixmap, _scene, &DrawingScene::slot_pixmap);

    connect(_scene, &DrawingScene::take_partial_screenshot,
            this, &MainWindow::slot_grab_partial);

    connect(_scene, &DrawingScene::take_screenshot,
            this, &MainWindow::slot_grab);

    connect(_scene, &DrawingScene::opacity,
            this, &MainWindow::slot_cycle_opacity);

    connect(_scene, &DrawingScene::toggle,
            this, &MainWindow::slot_toggle);

    connect(_scene, &DrawingScene::set_transparent,
            [this]() { this->setWindowOpacity(.3); });

    connect(_scene, &DrawingScene::set_opaque,
            [this]() { this->setWindowOpacity(1); });

    connect(_scene, &DrawingScene::take_screenshot,
            [this]() { this->slot_grab(); });

    connect(_scene, &DrawingScene::quit,
            this, &MainWindow::slot_quit);

    // clang-format on
}

MainWindow::~MainWindow() {
    delete _view;
    delete _scene;
    delete _snapshot_timer;
    delete _partial_snapshot_timer;
}

void MainWindow::slot_screen_changed() {
    _screen_rect = QGuiApplication::primaryScreen()->geometry();
    _pixel_ratio = QGuiApplication::primaryScreen()->devicePixelRatio();
    _screen_rect.translate(-_screen_rect.topLeft());
    _screen_rect.setWidth(_screen_rect.width() * _pixel_ratio + 0.2);
    _screen_rect.setHeight(_screen_rect.height() * _pixel_ratio + 0.2);
    _view->slot_screen_rect_changed(_screen_rect);
}

void MainWindow::slot_quit() {
    close();
}

void MainWindow::slot_cycle_opacity() {
    if (_opacity < 0.5) {
        _opacity = 1.0;
        setStyleSheet("background:black;");
    } else {
        _opacity = 0.3;
        setStyleSheet("background:transparent;");
    }
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_cycle_opacity: setting to" << _opacity;
    setWindowOpacity(_opacity);
}

void MainWindow::slot_undo() {
    if (Verbosity::get() >= VerboseAll) qDebug() << "MainWindow::slot_undo";
    _scene->undo();
}

void MainWindow::slot_redo() {
    if (Verbosity::get() >= VerboseAll) qDebug() << "MainWindow::slot_redo";
    _scene->redo();
}

// ------------------- commands received via dbus --------------------

void MainWindow::slot_change_color(int index) {
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_chance_color" << index;

    _scene->slot_set_color_from_index(index);
}

void MainWindow::slot_change_tool(const QString &tool) {
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_chance_tool" << tool;

    if (tool == "pen") {
        _scene->slot_set_tool(TOOL_PEN);
    } else if (tool == "smooth") {
        _scene->slot_set_tool(TOOL_SMOOTH);
    } else if (tool == "line") {
        _scene->slot_set_tool(TOOL_LINE);
    } else if (tool == "rect") {
        _scene->slot_set_tool(TOOL_RECT);
    } else if (tool == "text") {
        _scene->slot_set_tool(TOOL_TEXT);
    } else if (tool == "ortho") {
        _scene->slot_set_tool(TOOL_ORTHO);
    } else if (tool == "fader") {
        _scene->slot_set_tool(TOOL_FADER);
    } else if (tool == "highlight") {
        _scene->slot_set_tool(TOOL_HIGHLIGHT);
    } else if (tool == "select") {
        _scene->slot_set_tool(TOOL_SELECT);
    } else if (tool == "grab") {
        _scene->slot_set_tool(TOOL_PIXMAP);
    } else {
        std::cerr << "Unknown tool: " << tool.toStdString();
    }
}

void MainWindow::slot_change_arrow(const QString &arrow) {
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_chance_arrow" << arrow;

    if (arrow == "->") {
        _scene->slot_set_arrow(ARROW_END);
    } else if (arrow == "<-") {
        _scene->slot_set_arrow(ARROW_START);
    } else if (arrow == "<->") {
        _scene->slot_set_arrow(ARROW_DOUBLE);
    } else {
        _scene->slot_set_arrow(ARROW_NONE);
    }
}

void MainWindow::slot_change_width(int width) {
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_chance_width" << width;
    if (width > 0 && width < 80) _scene->slot_set_width(width);
}

void MainWindow::slot_rect(const QRectF& rect, int radius) {
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_rect" << rect;
    if (_screen_rect.contains(rect.toRect()) && radius >= 0 && radius <= 100)
        _scene->slot_add_rect(rect, radius);
}

void MainWindow::slot_grab_and_new() {
    if (Verbosity::get() >= VerboseAll) qDebug() << "MainWindow::slot_grab_and_new";
    _scene->slot_new();
    slot_grab();
}

void MainWindow::slot_grab() {
    if (Verbosity::get() >= VerboseAll) qDebug() << "MainWindow::slot_grab";
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    _snapshot_timer->start(1000 * _scene->grab_delay());
}

void MainWindow::slot_grab_partial(const QRect &geometry) {
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_grab_partial" << geometry;
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    _shot_rect = geometry;
    _partial_snapshot_timer->start(1000 * _scene->grab_delay());
}

void MainWindow::slot_screenshot_step1() {
    QPixmap px = screen()->grabWindow();
    _scene->slot_set_background(px);
    setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
    slot_show();
}

void MainWindow::slot_partial_screenshot_step1() {
    QPixmap px = screen()->grabWindow().copy(_shot_rect);
    qDebug() << "partial " << px.rect();
    show();
    setWindowOpacity(1.0);
    setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
    emit send_pixmap(px);
}

void MainWindow::slot_toggle() {
    if (Verbosity::get() >= VerboseAll) qDebug() << "MainWindow::slot_toggle";

    if (_on_top) {
        // ------------------------ hide window ------------------------
        setWindowState(Qt::WindowMinimized);
        setWindowFlags(
            (windowFlags() | Qt::FramelessWindowHint) &
            ~(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint));
        hide();
    } else {
        // -------------- show window maximized, on top --------------
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint |
                       Qt::WindowStaysOnTopHint);
        setWindowState(Qt::WindowMaximized | Qt::WindowFullScreen);
        show();
    }
    _on_top = !_on_top;
}

void MainWindow::slot_show() {
    if (Verbosity::get() >= VerboseAll) qDebug() << "MainWindow::slot_show";

    _on_top = false;
    slot_toggle();
}

void MainWindow::slot_hide() {
    if (Verbosity::get() >= VerboseAll) qDebug() << "MainWindow::slot_hide";

    _on_top = true;
    slot_toggle();
}

void MainWindow::slot_save(const QString &filename) {
    if (Verbosity::get() >= VerboseAll)
        qDebug() << "MainWindow::slot_save" << filename;

    setWindowOpacity(1.0);
    setStyleSheet("background:black;");
    QPixmap px(size());
    render(&px);
    px.save(filename);
    setStyleSheet("background:transparent;");
}
