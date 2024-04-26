#include "mainwindow.h"

#include <QMainWindow>
#include <QPixmap>
#include <QShortcut>
#include <QKeySequence>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QScreen>
#include <QPixmap>
#include <QFile>
#include <QKeyEvent>

#include "drawingview.h"
#include "drawingscene.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // graphics scene
    scene = new DrawingScene;
    view = new DrawingView(scene);
    setCentralWidget(view);

    // setup timer for snapshot delay
    snapshot_timer = new QTimer(this);
    snapshot_timer->setSingleShot(true);
    snapshot_timer->setInterval(10);
    connect(snapshot_timer, &QTimer::timeout,
            this, &MainWindow::screenshot_step1);

    // set up "quit" keyboard shortcut
    QAction *quitAction = new QAction(tr("Quit"), this);
    addAction(quitAction);
    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quitAction, &QAction::triggered,  this, &MainWindow::quit);

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

// ------------------- commands received via dbus --------------------

void MainWindow::change_color(const QString &color) {
  qDebug() << "MainWindow::change_color" << color;
}

void MainWindow::take_screenshot(const QString &geometry) {
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    snapshot_timer->start(100);
}

void MainWindow::screenshot_step1() {
  QPixmap px = screen()->grabWindow();
  view->set_background(px);
  show();
}

void MainWindow::toggle() {
    if (on_top) {
        // ------------------------ hide window ------------------------
        setWindowState(Qt::WindowMinimized);
        setWindowFlags((windowFlags() | Qt::FramelessWindowHint)
                       &~(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint));
    } else {
        // -------------- show window maximized, on top --------------
        setWindowFlags(windowFlags()
                       | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        setWindowState(Qt::WindowMaximized | Qt::WindowFullScreen);
        show();
    }
    show();
    on_top = !on_top;
}
