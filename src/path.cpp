#include "path.h"
#include <math.h>
#include "utility.h"
#include <QPoint>
#include <QTransform>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

// ============================== Path ===============================

Path::Path() {}

Path::~Path() {}

void Path::append(const XY& p) {
    _path.emplace_back(p);
}

void Path::append(const double x, const double y) {
    _path.emplace_back(XY(x, y));
}

void Path::append(const QPointF& p) {
    _path.emplace_back(XY(p));
}

void Path::clear() {
    _path.clear();
}

// Path::apply2D
//
// apply a 2d affine transformtion to the path
//
void Path::apply2D(const QTransform& m) {
    for (auto& p : _path) {
        qreal tx, ty;
        m.map(p.x(), p.y(), &tx, &ty);
        p.setX(tx);
        p.setY(ty);
    }
}

double Path::end_angle(double radius, bool end_is_beginning) const {
    const double r2 = square(radius);
    if (end_is_beginning) {
        const double x0 = _path.front().x();
        const double y0 = _path.front().y();
        auto last = _path.cbegin();
        for (auto it = _path.cbegin(); it != _path.cend(); ++it) {
            if (square(x0 - it->x()) + square(y0 - it->y()) > r2) break;
            last = it;
        }
        return atan2(y0 - last->y(), x0 - last->x());
        //
    } else {
        const double x0 = _path.back().x();
        const double y0 = _path.back().y();
        auto last = _path.crbegin();
        for (auto it = _path.crbegin(); it != _path.crend(); ++it) {
            if (square(x0 - it->x()) + square(y0 - it->y()) > r2) break;
            last = it;
        }
        return atan2(last->y() - y0, last->x() - x0);
    }
}

// Path::douglas_peucker
//
// perform Douglas-Peucker smoothing of path points with distance
// threshold 'epsilon'. Based on
// https://namekdev.net/2014/06/iterative-version-of-ramer-douglas-peucker-line-simplification-algorithm/
//
void Path::douglas_peucker(double epsilon) {
    if (_path.size() < 3) return;

    std::vector<int> to_delete;               // indices of elements to delete
    std::vector<std::pair<int, int>> ranges;  // sections of the path

    int first = 0;
    int last = _path.size() - 1;

    ranges.emplace_back(std::pair<int, int>(first, last));

    while (!ranges.empty()) {
        first = ranges.back().first;
        last = ranges.back().second;

        ranges.pop_back();

        double dmax = 0.0;
        int max_element = first;

        bool too_close = _path[first].distance(_path[last]) < 10.0;
        XY midp = _path[first].midpoint(_path[last]);

        for (int i = first + 1; i < last; i++) {
            double d;
            if (too_close) {
                d = _path[i].distance(midp);
            } else {
                d = _path[i].distance_from_segment(_path[first], _path[last]);
            }
            if (d > dmax) {
                dmax = d;
                max_element = i;
            }
        }
        if (dmax > epsilon) {
            ranges.emplace_back(std::pair<int, int>(first, max_element));
            ranges.emplace_back(std::pair<int, int>(max_element, last));
        } else {
            for (int i = first + 1; i < last; i++) to_delete.emplace_back(i);
        }
    }

    std::sort(to_delete.begin(), to_delete.end());

    auto index_it = to_delete.cbegin();
    int src_idx = 0;
    int trg_idx = 0;
    for (;;) {
      if (src_idx == *index_it) {
            src_idx++;
            index_it++;
            if (index_it == to_delete.cend()) break;
            continue;
        }
        _path[trg_idx] = _path[src_idx];
        trg_idx++;
        src_idx++;
    }
    while (src_idx < _path.size()) _path[trg_idx++] = _path[src_idx++];
    _path.resize(trg_idx);
}

std::ostream& operator<<(std::ostream& os, const Path& p) {
  os << "Path @ " << &p << ", len=" << p.size() << " :";
  for (const auto& point : p._path) {
    os << " " << point;
  }
  os << "\n";
  return os;
}

// ============================== PathW ==============================

PathW::PathW() {}

PathW::~PathW() {}

void PathW::append(const XYW& p) {
    _path.emplace_back(p);
}

void PathW::append(const double x, const double y, const double w) {
    _path.emplace_back(XYW(x, y, w));
}

void PathW::clear() {
    _path.clear();
}

// Path::apply2D
//
// apply a 2d affine transformtion to the path
//
void PathW::apply2D(const QTransform& m) {
    for (auto& p : _path) {
        qreal tx, ty;
        m.map(p.x(), p.y(), &tx, &ty);
        p.setX(tx);
        p.setY(ty);
    }
}

// PathW::has_constant_width()
//
// check if path has a constant width (within tolerance of 0.01)
//
bool PathW::has_constant_width() const {
    if (_path.empty()) return true;
    double w = _path.front().w();
    for (const auto& p : _path) {
        if (fabs(p.w() - w) > 0.01) return false;
    }
    return true;
}

// PathW::average_width()
//
// calculate average width of path
//
double PathW::average_width() const {
    if (_path.empty()) return 0.0;

    double sum_w = 0.0;
    for (const auto& p : _path) sum_w += p.w();

    return sum_w / _path.size();
}

// PathW::end_width()
//
// calculate average width of one end of the path (within radius from
// end point)
//
double PathW::end_width(double radius, bool end_is_beginning) const {
    if (_path.empty()) return 0.0;

    const double r2 = square(radius);
    double sum_w = 0.0;
    int i = 0;
    if (end_is_beginning) {
        const double x0 = _path.front().x();
        const double y0 = _path.front().y();
        for (auto it = _path.cbegin(); it != _path.cend(); ++it) {
            if (square(x0 - it->x()) + square(y0 - it->y()) > r2) break;
            sum_w += it->w();
            i++;
        }
    } else {
        const double x0 = _path.back().x();
        const double y0 = _path.back().y();
        for (auto it = _path.crbegin(); it != _path.crend(); ++it) {
            if (square(x0 - it->x()) + square(y0 - it->y()) > r2) break;
            sum_w += it->w();
            i++;
        }
    }
    return sum_w / i;
}

// PathW::simplify
//
// remove intermediate points if (1) deviation from straight line <
// max_dev, and (2) deviation from last width < max_width_dev.
//
void PathW::simplify(double max_dev, double max_width_dev) {
    if (_path.size() < 4) return;
    const int last = _path.size() - 1;

    int i0 = 0;

    while (i0 < last - 2) {
        // first point
        double w0 = _path[i0].w();
        XY p0 = XY(_path[i0].x(), _path[i0].y());

        // scan forward checking width -> imax is last possible
        int imax = i0 + 1;
        while (imax < last) {
            if (fabs(_path[imax + 1].w() - w0) <= max_dev) break;
            imax++;
        }

        // test possible ends
        int i1 = i0 + 2;
        while (i1 <= imax) {
            XY p1 = XY(_path[i1].x(), _path[i1].y());
            for (int i = i0 + 1; i < i1; i++) {
                XY pi = XY(_path[i].x(), _path[i].y());
                if (pi.distance_from_line(p0, p1) > max_dev) {
                    i1--;
                    goto end_found;
                }
            }
        }
    end_found:
        // segment is [i0, i1]: mark intermediate points for deletion
        for (int i = i0 + 1; i < i1 - 1; i++) _path[i].set_w(-1.0);
        i0 = i1;
    }

    // remove all unnecessary points

    int trg = 0;
    for (int src = 0; src <= last; src++) {
        if (_path[src].w() >= 0.0) { _path[trg++] = _path[src]; }
    }
    _path.resize(trg);
}


std::ostream& operator<<(std::ostream& os, const PathW& p) {
  os << "Path @ " << &p << ", len=" << p.size() << " :";
  for (const auto& point : p._path) {
    os << " " << point;
  }
  os << "\n";
  return os;
}
