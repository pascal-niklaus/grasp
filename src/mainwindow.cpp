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
    connect(snapshot_timer, &QTimer::timeout, this, &MainWindow::screenshot_step1);

    // set up "quit" keyboard shortcut
    QAction *quitAction = new QAction(tr("Quit"), this);
    addAction(quitAction);
    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quitAction, &QAction::triggered,  this, &MainWindow::quit);
}

MainWindow::~MainWindow() {
    delete view;
    delete scene;
    delete snapshot_timer;
}

void MainWindow::quit() {
    close();
}

// ------------------------- keypress events -------------------------

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_H) {
        on_top = true;
        toggle();
    } else if (event->key() == Qt::Key_S) {
        on_top = false;
        toggle();
    } else if (event->key() == Qt::Key_G) {
        // -------------------- take a screenshot --------------------
        take_screenshot("");
    } else if (event->key() == Qt::Key_O) {
        // --------------------- toggle opacity ----------------------
        double o = windowOpacity();
        if (o < 0.5)
            setWindowOpacity(1);
        else
            setWindowOpacity(.3);
    } else {
      // -------------- key not grabbed, pass on event ---------------
        QMainWindow::keyPressEvent(event);
    }
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
