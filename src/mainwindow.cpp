#include "drawingscene.h"
#include "drawingview.h"
#include "mainwindow.h"
#include "screenres.h"

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
MainWindow::MainWindow(QSettings& qset, QWidget *parent) : QMainWindow(parent) {
    // graphics scene
    _scene = new DrawingScene(qset);
    _view = new DrawingView(_scene);
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
    connect(_partial_snapshot_timer,
            &QTimer::timeout,
            this,
            &MainWindow::slot_partial_screenshot_step1);
    connect(this, &MainWindow::send_pixmap, _scene, &DrawingScene::slot_pixmap);
    connect(_scene,
            &DrawingScene::take_partial_screenshot,
            this,
            &MainWindow::slot_grab_partial);
    connect(_scene, &DrawingScene::set_transparent, [this]() {
        this->setWindowOpacity(.3);
    });
    connect(_scene, &DrawingScene::set_opaque, [this]() {
        this->setWindowOpacity(1);
    });

    // set up keyboard shortcut actions
    QAction *quitAction = new QAction(tr("Quit"), this);
    addAction(quitAction);
    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quitAction, &QAction::triggered, this, &MainWindow::slot_quit);

    QAction *grabAction = new QAction(tr("Grab"), this);
    addAction(grabAction);
    grabAction->setShortcut(Qt::CTRL | Qt::Key_G);
    connect(grabAction, &QAction::triggered, [this]() { this->slot_grab(); });

    QAction *opaqAction = new QAction(tr("Opacity"), this);
    addAction(opaqAction);
    opaqAction->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(
        opaqAction, &QAction::triggered, this, &MainWindow::slot_cycle_opacity);

    QAction *undoAction = new QAction(tr("Undo"), this);
    addAction(undoAction);
    undoAction->setShortcut(Qt::CTRL | Qt::Key_Z);
    connect(undoAction, &QAction::triggered, this, &MainWindow::slot_undo);

    QAction *redoAction = new QAction(tr("Redo"), this);
    addAction(redoAction);
    redoAction->setShortcut(Qt::CTRL | Qt::Key_Y);
    connect(redoAction, &QAction::triggered, this, &MainWindow::slot_redo);


    // this action is questionable and better set up as system-wide
    // shortcut, because it obviously only works when the application
    // has focus
    QAction *toggleAction = new QAction(tr("Toggle"), this);
    addAction(toggleAction);
    toggleAction->setShortcut(Qt::CTRL | Qt::Key_T);
    connect(toggleAction, &QAction::triggered, this, &MainWindow::slot_toggle);

    QAction *resizeAction = new QAction(tr("Resize"), this);
    addAction(resizeAction);
    resizeAction->setShortcut(Qt::Key_F2);
    connect(resizeAction, &QAction::triggered, this, &MainWindow::slot_screen_changed);
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
    qDebug() << "Screen changed:" << _screen_rect
             << "pixelRatio=" << _pixel_ratio
             << "view rect: " << _view->rect();
    _screen_rect.translate(-_screen_rect.topLeft());
    // qDebug() << "translated: " << _screen_rect;
    _screen_rect.setWidth(_screen_rect.width() * _pixel_ratio + 0.2);
    _screen_rect.setHeight(_screen_rect.height() * _pixel_ratio + 0.2);
    qDebug() << "scaled: " << _screen_rect;
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
    setWindowOpacity(_opacity);
}

void MainWindow::slot_undo() {
    _scene->undo();
}

void MainWindow::slot_redo() {
    _scene->redo();
}

// ------------------- commands received via dbus --------------------

void MainWindow::slot_change_color(int index) {
    _scene->slot_set_color_from_index(index);
}

void MainWindow::slot_change_tool(const QString &tool) {
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
    if (width > 0 && width < 80) _scene->slot_set_width(width);
}

void MainWindow::slot_grab_and_new() {
    _scene->slot_new();
    slot_grab();
}

void MainWindow::slot_grab() {
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    _snapshot_timer->start(200);
}

void MainWindow::slot_grab_partial(const QRect &geometry) {
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    _shot_rect = geometry;
    qDebug() << "_shot rect=" << geometry;
    _partial_snapshot_timer->start(200);
}

void MainWindow::slot_screenshot_step1() {
    QPixmap px = screen()->grabWindow();
    _scene->slot_set_background(px);
    setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
    slot_show();
}

void MainWindow::slot_partial_screenshot_step1() {
    // QRect screenRect = screen()->geometry();
    // _shot_rect.adjust(-screenRect.left(),
    //                  -screenRect.top(),
    //                  -screenRect.left(),
    //                  -screenRect.top());
    QPixmap px = screen()->grabWindow().copy(_shot_rect);
    qDebug() << "partial " << px.rect();
    show();
    setWindowOpacity(1.0);
    setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
    emit send_pixmap(px);
}

void MainWindow::slot_toggle() {
    if (_on_top) {
        qDebug() << "slot_toggle: hiding";
        // ------------------------ hide window ------------------------
        setWindowState(Qt::WindowMinimized);
        setWindowFlags(
            (windowFlags() | Qt::FramelessWindowHint) &
            ~(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint));
        hide();
    } else {
        qDebug() << "slot_toggle: showing";
        // -------------- show window maximized, on top --------------
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint |
                       Qt::WindowStaysOnTopHint);
        setWindowState(Qt::WindowMaximized | Qt::WindowFullScreen);
        show();
    }
    _on_top = !_on_top;
}

void MainWindow::slot_show() {
    _on_top = false;
    slot_toggle();
}

void MainWindow::slot_hide() {
    _on_top = true;
    slot_toggle();
}

void MainWindow::slot_save(const QString &filename) {
    setWindowOpacity(1.0);
    setStyleSheet("background:black;");
    QPixmap px(size());
    render(&px);
    px.save(filename);
    setStyleSheet("background:transparent;");
}
