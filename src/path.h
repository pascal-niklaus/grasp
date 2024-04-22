#ifndef PATH_H
#define PATH_H

#include <qpoint.h>
#include <QTransform>

#include "xy.h"
#include <vector>


class Path {
  public:
    Path();
    ~Path();

    bool is_empty() const { return _path.empty(); }
    int size() const { return _path.size(); }
    XY& operator[](int i) { return _path[i]; }

    void append(const QPointF& p);
    void append(const XY& p);
    void append(const double x, const double y);
    void clear();

    QPointF first() const {
        if (_path.empty())
            return QPointF(0, 0);
        else
            return _path.front();
    }
    QPointF last() const {
        if (_path.empty())
            return QPointF(0, 0);
        else
            return _path.back();
    }

    double w() const { return _w; }
    void set_w(double w) { _w = w; }

    void apply2D(const QTransform& m);

    void douglas_peucker(double epsilon);
    double end_angle(double radius, bool end_is_beginning) const;

    friend std::ostream& operator<<(std::ostream& os, const Path& p);

  private:
    double _w;
    std::vector<XY> _path;
};

class PathW {
  public:
    PathW();
    ~PathW();

    bool is_empty() const { return _path.empty(); }
    int size() const { return _path.size(); }

    void append(const XYW& p);
    void append(const double x, const double y, double w);
    void clear();

    void apply2D(const QTransform& m);
    bool has_constant_width() const;
    double average_width() const;
    double end_width(double radius, bool end_is_beginning) const;
    void simplify(double max_dev, double max_width_dev);

    friend std::ostream& operator<<(std::ostream& os, const PathW& p);

  private:
    std::vector<XYW> _path;
    };

#endif
