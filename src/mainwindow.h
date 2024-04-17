#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QGraphicsScene;
class QGraphicsView;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void quit();

private:
    QGraphicsScene *scene = nullptr;
    QGraphicsView *view = nullptr;
};


#endif // MAINWINDOW_H
