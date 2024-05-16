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
    MainWindow(QSettings &qset, int verbosity = 0, QWidget *parent = nullptr);
    ~MainWindow();

  signals:
    void send_pixmap(const QPixmap &);

  public slots:
    void slot_grab_partial(const QRect &geometry);

    void slot_quit();

    void slot_toggle();
    void slot_show();
    void slot_hide();

    void slot_grab();
    void slot_grab_and_new();

    void slot_change_color(int index);
    void slot_change_tool(const QString &tool);
    void slot_change_arrow(const QString &arrow);
    void slot_change_width(int width);

    // see-through for debugging
    void slot_cycle_opacity();

    void slot_undo();
    void slot_redo();

    void slot_save(const QString &filename);

  private slots:
    void slot_screenshot_step1();
    void slot_partial_screenshot_step1();
    void slot_screen_changed();

  private:
    DrawingScene *_scene = nullptr;
    DrawingView *_view = nullptr;

    bool _on_top = true;
    int _verbosity;
    double _opacity;
    QRect _screen_rect;
    QRect _shot_rect;
    double _pixel_ratio = 1.0;

    QTimer *_snapshot_timer = nullptr;
    QTimer *_partial_snapshot_timer = nullptr;
};

#endif // MAINWINDOW_H

