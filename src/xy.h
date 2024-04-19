#ifndef XY_H
#define XY_H

#include <qpoint.h>
#include <ostream>

#include <QPointF>

class QTransform;

// XY
//
// Coordinate (x,y)
//
class XY : public QPointF {
  public:
    XY();
    explicit XY(const QPointF& p);
    explicit XY(double x, double y);

    ~XY() = default;

    void add(const XY &p);

    void apply2d(const QTransform& m);
    void apply2d(const QTransform&, XY &result) const;
    XY transformed2d(const QTransform& m) const;

    XY midpoint(const XY &p) const;

    double distance_from_line(const XY &line_start, const XY &line_end) const;
    double distance_from_segment(const XY &line_start, const XY &line_end) const;

    double distance(const XY &p) const;
    static double distance(const XY &p1, const XY &p2);

    friend std::ostream &operator<<(std::ostream &os, const XY &p);
};

// XYW
//
// A coordinate (x,y) with associated stroke width
//
class XYW : public XY {
  public:
    XYW() : XY(){};
    XYW(double x, double y, double w) : XY(x, y), _w(w) {}
    ~XYW() = default;

    double w() const { return _w; }
    void set_w(double w) { _w = w; }

    friend std::ostream &operator<<(std::ostream &os, const XYW &p);

  protected:
    double _w = 1.0;
};

#endif
