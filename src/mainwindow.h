#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "drawingscene.h"
#include "drawingview.h"

class QAction;
class QGraphicsScene;
class QGraphicsView;


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void quit();

private:
    DrawingScene *scene = nullptr;
    DrawingView *view = nullptr;
};


#endif // MAINWINDOW_H
