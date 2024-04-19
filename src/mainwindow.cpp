#include "mainwindow.h"

#include <QShortcut>
#include <QKeySequence>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "drawingview.h"
#include "drawingscene.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    scene = new DrawingScene;
    view = new DrawingView(scene);

    scene->addText("Hello world!");
    setCentralWidget(view);

    QAction *quitAction = new QAction(tr("Quit"), this);
    addAction(quitAction);

    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);

    connect(quitAction, &QAction::triggered,  this, &MainWindow::quit);
}

MainWindow::~MainWindow()
{
}

void MainWindow::quit() {

    qDebug() << "MainWindow::quit()";
    close();

}
