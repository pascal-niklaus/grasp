
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>

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

  void fix_geometry();

  signals:
    void send_pixmap(const QPixmap &);

  public slots:
    void change_color(const QString &color);
    void take_screenshot(const QString &geometry);
    void take_partial_screenshot(const QRect &geometry);
    void toggle();
    void quit();
    void cycle_opacity();
    void undo();
    void redo();
    void tab();
    void save(const QString &filename);

  private slots:
    void screenshot_step1();
    void partial_screenshot_step1();
    void slot_screen_changed(QScreen *);

  private:
    QRect screen_rect;
    double pixel_ratio = 1.0;
    QRect shot_rect;
    bool on_top = true;
    DrawingScene *scene = nullptr;
    DrawingView *view = nullptr;
    QTimer *snapshot_timer = nullptr;
    QTimer *partial_snapshot_timer = nullptr;
    double _opacity;
};

#endif // MAINWINDOW_H
