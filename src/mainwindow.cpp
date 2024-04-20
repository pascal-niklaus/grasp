#include "mainwindow.h"
#include <qmainwindow.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qwindowdefs.h>

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    scene = new DrawingScene;
    view = new DrawingView(scene);

    scene->addText("Hello world!");
    setCentralWidget(view);

    // setup timer
    snapshot_timer = new QTimer(this);
    snapshot_timer->setSingleShot(true);
    snapshot_timer->setInterval(10);

    connect(snapshot_timer, &QTimer::timeout, this, &MainWindow::screenshot_step1);

    QAction *quitAction = new QAction(tr("Quit"), this);
    addAction(quitAction);

    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);

    connect(quitAction, &QAction::triggered,  this, &MainWindow::quit);
}

MainWindow::~MainWindow()
{
}


void MainWindow::change_color(const QString &color) {
  qDebug() << "MainWindow::change_color" << color;
}

void MainWindow::quit() {
    qDebug() << "MainWindow::quit()";
    close();

}

void MainWindow::toggle() {}

void MainWindow::take_screenshot(const QString &geometry) {
    hide();
    // 10 ms is too short
    // 50 ms seems to work
    // we use 100ms for safety
    snapshot_timer->start(100);
}

void MainWindow::screenshot_step1() {
  qDebug() << "MainWindow::screenshot_step1";
  QPixmap px = screen()->grabWindow();
  view->set_background(px);
  show();
  // QFile file("screenshot.png");
  // file.open(QIODevice::WriteOnly);
  // px.save(&file, "PNG");
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_H) {
        setWindowState(Qt::WindowMinimized);
        setWindowFlags(windowFlags()
                       | Qt::FramelessWindowHint
                       & ~(Qt::WindowStaysOnTopHint
                           | Qt::X11BypassWindowManagerHint));
        show();
    } else if (event->key() == Qt::Key_S) {
         setWindowFlags(windowFlags()
                        | Qt::FramelessWindowHint
                        | Qt::WindowStaysOnTopHint);
        setWindowState(Qt::WindowMaximized | Qt::WindowFullScreen);
        show();
    } else if (event->key() == Qt::Key_G) {
        take_screenshot("");
    } else if (event->key() == Qt::Key_O) {
      double o = windowOpacity();
      if (o < 0.5)
        setWindowOpacity(1);
      else
        setWindowOpacity(.3);
    } else {
        QMainWindow::keyPressEvent(event);
    }
}
