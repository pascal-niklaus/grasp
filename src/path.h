#ifndef PATH_H
#define PATH_H

#include "xy.h"

#include <vector>

class Path {
  public:
    Path();
    ~Path();

    bool is_empty() const { return _path.empty(); }
    int size() const { return _path.size(); }

    void append(const XY& p);
    void append(const double x, const double y);
    void clear();

    double w() const { return _w; }
    void set_w(double w) { _w = w; }

    void apply2D(const cairo_matrix_t& m);

    void douglas_peucker(double epsilon);

    friend std::ostream& operator<<(std::ostream& os, const Path& p);

  private:
    double _w;
    std::vector<XY> _path;
};


#endif
