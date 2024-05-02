#include "mainwindow.h"

#include <qnamespace.h>

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
#include <iostream>

#include "drawingscene.h"
#include "drawingview.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // graphics scene
    scene = new DrawingScene;
    view = new DrawingView(scene);
    setCentralWidget(view);

    // setup timer for snapshot delay
    snapshot_timer = new QTimer(this);
    snapshot_timer->setSingleShot(true);
    snapshot_timer->setInterval(10);
    connect(
        snapshot_timer, &QTimer::timeout, this, &MainWindow::screenshot_step1);

    // setup timer for pixmap grabbing
    partial_snapshot_timer = new QTimer(this);
    partial_snapshot_timer->setSingleShot(true);
    partial_snapshot_timer->setInterval(10);
    connect(partial_snapshot_timer,
            &QTimer::timeout,
            this,
            &MainWindow::partial_screenshot_step1);
    connect(this, &MainWindow::send_pixmap, scene, &DrawingScene::slot_pixmap);
    connect(scene,
            &DrawingScene::take_partial_screenshot,
            this,
            &MainWindow::take_partial_screenshot);
    connect(scene, &DrawingScene::set_transparent, [this]() {
        this->setWindowOpacity(.3);
    });

    // set up "quit" keyboard shortcut
    QAction *quitAction = new QAction(tr("Quit"), this);
    addAction(quitAction);
    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quitAction, &QAction::triggered, this, &MainWindow::quit);

    QAction *grabAction = new QAction(tr("Grab"), this);
    addAction(grabAction);
    grabAction->setShortcut(Qt::CTRL | Qt::Key_G);
    connect(grabAction, &QAction::triggered, [this]() {
        this->take_screenshot("");
    });

    QAction *toggleAction = new QAction(tr("Toggle"), this);
    addAction(toggleAction);
    toggleAction->setShortcut(Qt::CTRL | Qt::Key_T);
    connect(toggleAction, &QAction::triggered, this, &MainWindow::toggle);

    QAction *topAction = new QAction(tr("Top"), this);
    addAction(topAction);
    topAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_T);
    connect(topAction, &QAction::triggered, [this]() {
        this->on_top = false;
        this->toggle();
    });

    QAction *opaqAction = new QAction(tr("Opaq"), this);
    addAction(opaqAction);
    opaqAction->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(opaqAction, &QAction::triggered, this, &MainWindow::cycle_opacity);

    QAction *undoAction = new QAction(tr("Undo"), this);
    addAction(undoAction);
    undoAction->setShortcut(Qt::CTRL | Qt::Key_Z);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);

    QAction *redoAction = new QAction(tr("Redo"), this);
    addAction(redoAction);
    redoAction->setShortcut(Qt::CTRL | Qt::Key_Y);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);

    QAction *tabAction = new QAction(tr("Dash"), this);
    addAction(tabAction);
    tabAction->setShortcut(Qt::Key_Tab);
    connect(tabAction, &QAction::triggered, this, &MainWindow::tab);

    QAction *saveAction = new QAction(tr("Save"), this);
    addAction(saveAction);
    saveAction->setShortcut(Qt::CTRL | Qt::Key_S);
    connect(
        saveAction, &QAction::triggered, [this]() { this->save("test.png"); });
}

MainWindow::~MainWindow() {
    delete view;
    delete scene;
    delete snapshot_timer;
}

void MainWindow::quit() {
    close();
}

void MainWindow::cycle_opacity() {
    double o = windowOpacity();
    if (o < 0.5)
        setWindowOpacity(1);
    else
        setWindowOpacity(.3);
}

void MainWindow::undo() {
    scene->undo();
}

void MainWindow::redo() {
    scene->redo();
}

void MainWindow::tab() {
    scene->tab();
}

// ------------------- commands received via dbus --------------------

void MainWindow::change_color(const QString &color) {
    qDebug() << "MainWindow::change_color" << color;
}

void MainWindow::take_screenshot(const QString &geometry) {
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    snapshot_timer->start(200);
}

void MainWindow::take_partial_screenshot(const QRect &geometry) {
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    shot_rect = geometry;
    partial_snapshot_timer->start(200);
}

void MainWindow::screenshot_step1() {
    QPixmap px = screen()->grabWindow();
    view->set_background(px);
    show();
}

void MainWindow::partial_screenshot_step1() {
    QRect screenRect = screen()->geometry();
    shot_rect.adjust(-screenRect.left(),
                     -screenRect.top(),
                     -screenRect.left(),
                     -screenRect.top());
    QPixmap px = screen()->grabWindow().copy(shot_rect);
    show();
    setWindowOpacity(1.0);
    emit send_pixmap(px);
}

void MainWindow::toggle() {
    if (on_top) {
        // ------------------------ hide window ------------------------
        setWindowState(Qt::WindowMinimized);
        setWindowFlags(
            (windowFlags() | Qt::FramelessWindowHint) &
            ~(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint));
    } else {
        // -------------- show window maximized, on top --------------
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint |
                       Qt::WindowStaysOnTopHint);
        setWindowState(Qt::WindowMaximized | Qt::WindowFullScreen);
        show();
    }
    show();
    on_top = !on_top;
}

void MainWindow::fix_geometry() {
    // TODO: this is an ugly hack because otherwise we don't get the
    // full screen resolution in multi-monitor mode
    QRect real_size = screen()->grabWindow().rect();
    scene->fix_geometry(real_size);
}

void MainWindow::save(const QString &filename) {
    setWindowOpacity(1.0);
    setStyleSheet("background:black;");
    QPixmap px(size());
    render(&px);
    px.save(filename);
    setStyleSheet("background:transparent;");
}
