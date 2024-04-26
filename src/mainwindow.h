#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "drawingscene.h"
#include "drawingview.h"

class QAction;
class QGraphicsScene;
class QGraphicsView;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  public slots:
    void change_color(const QString &color);
    void take_screenshot(const QString &geometry);
    void toggle();
    void quit();
    void cycle_opacity();
    void undo();
    void redo();

  protected:
  //    virtual void keyPressEvent(QKeyEvent *event) override;

  private slots:
    void screenshot_step1();

  private:
    bool on_top = true;
    DrawingScene *scene = nullptr;
    DrawingView *view = nullptr;
    QTimer *snapshot_timer = nullptr;
};


#endif // MAINWINDOW_H
