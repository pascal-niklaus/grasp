#include "path.hpp"
#include <utility>
#include "utility.hpp"

#include <cmath>
#include <numeric>
#include <algorithm>

// ============================== Path ===============================

void Path::append(const XY& p) {
    _path.emplace_back(p);
}

void Path::append(const double x, const double y) {
    _path.emplace_back(XY(x, y));
}

void Path::clear() {
    _path.clear();
}

// Path::apply2D
//
// apply a 2d affine transformtion to the path
//
void Path::apply2D(const cairo_matrix_t& m) {
    for (auto& p : _path) {
        auto tmp = m.xx * p.x() + m.xy * p.y() + m.x0;
        p.set_y(m.xx * p.x() + m.xy * p.y() + m.x0);
        p.set_x(tmp);
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
            for (int i = first + 1; i < last - 1; i++)
                to_delete.emplace_back(i);
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
void PathW::apply2D(const cairo_matrix_t& m) {
    for (auto& p : _path) {
        auto tmp = m.xx * p.x() + m.xy * p.y() + m.x0;
        p.set_y(m.xx * p.x() + m.xy * p.y() + m.x0);
        p.set_x(tmp);
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


std::ostream& operator<<(std::ostream& os, const PathW& p) {
  os << "Path @ " << &p << ", len=" << p.size() << " :";
  for (const auto& point : p._path) {
    os << " " << point;
  }
  os << "\n";
  return os;
}
