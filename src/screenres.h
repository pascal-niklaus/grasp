#ifndef SCREENRES_H
#define SCREENRES_H

#include <QObject>
#include <QRect>

class ScreenChangeDetector : public QObject {
    Q_OBJECT
  public:
    explicit ScreenChangeDetector(QObject *parent = nullptr);

    QRect rect_pixels() const { return _screen_rect_pixels; }
    QRect rect_logical() const { return _screen_rect_logical; }

 signals:
    void screen_pixels_changed(QRect);

  private:
    void setup();
    void adjust();
  private slots:
    void slot_geometry_changed(const QRect &newGeometry);
    void slot_physical_size_changed(const QSizeF &newPhysicalSize);
    void slot_dpi_changed(qreal dpi);

  private:
    QRect _screen_rect_logical;
    QRect _screen_rect_pixels;
    double _pixel_ratio;
};

#endif
