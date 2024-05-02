#include "path.h"

#include <QTransform>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "utility.h"

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
            last = it;
            if (square(x0 - it->x()) + square(y0 - it->y()) > r2) break;
        }

        return atan2(y0 - last->y(), x0 - last->x());
    } else {
        const double x0 = _path.back().x();
        const double y0 = _path.back().y();
        auto last = _path.crbegin();
        for (auto it = _path.crbegin(); it != _path.crend(); ++it) {
            last = it;
            if (square(x0 - it->x()) + square(y0 - it->y()) > r2) break;
        }
        return atan2(y0 - last->y(), x0 - last->x());
    }
}

// Path::snap_ends
//
// join ends (at their midpoint) is they are close enough and the
// first and last segment reasonably allows for it
//
bool Path::snap_ends(double max_distance) {
    if (_path.size() > 3) {
        const double dist = _path.front().distance(_path.back());
        const double start_seg_len = _path.front().distance(_path[1]);
        const double end_seg_len =
            _path.back().distance(_path[_path.size() - 2]);
        if (dist <= max_distance && start_seg_len > 0.7 * dist &&
            end_seg_len > 0.7 * dist) {
            _path.front() = _path.back() = _path.front().midpoint(_path.back());
            return true;
        }
    }
    return false;
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
    while (static_cast<size_t>(src_idx) < _path.size())
        _path[trg_idx++] = _path[src_idx++];
    _path.resize(trg_idx);
}

// Path::catmull_rom//
//
// centripetal Catmull-Rom interpolation with 'steps' steps between
// coordinates.  Based on Python implementation at
// https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline
//
// when circular = true, the first and last point of the path are
// identical
//
void Path::catmull_rom(int steps, bool circular) {
    if (_path.size() < 3) return;  // at least 4 points needed

    std::vector<XY> result;

    const int last = _path.size() - 1;  // last valid element
    // during last iteration, i==last-1
    for (int i = 0; i+1 <= last; i++) {
        XY a1, a2, a3, b1, b2, pt;
        double t0 = 0.0;
        double t1 = 0.0;
        const bool is_last = (i + 1 == last);
        if (i == 0) {
            if (circular) {
                // first point: take 2nd last point because first=last
                t1 = catmull_rom_tj(t0, _path[last-1], _path[i]);
            } else {
                t1 = 0.0;
            }
        } else {
            t1 = catmull_rom_tj(t0, _path[i-1], _path[i]);
        }
        double t2 = catmull_rom_tj(t1,_path[i], _path[i+1]);
        double t3;
        if (is_last) {
            if (circular) {
                // last point: take 2nd point because first = last
                t3 = catmull_rom_tj(t2, _path[i+1], _path[1]);
            } else {
                t3 = t2;
            }
        } else {
            t3 = catmull_rom_tj(t2, _path[i+1], _path[i+2]);
        }

        // interpolate curve
        const double k = (t2 - t1) / steps;
        for (int j = (i == 0 ? 0 : 1); j <= steps; j++) {
            const double t = t1 + k * j;
            if (i == 0) {
                if (circular) {
                    a1 = cr_f_mul_xy(
                        (t1 - t) / (t1 - t0), _path[last - 1], _path[i]);
                } else {
                    a1 = _path[i];
                }
            } else {
                a1 = cr_f_mul_xy((t1 - t) / (t1 - t0), _path[i - 1], _path[i]);
            }
            a2 = cr_f_mul_xy((t2 - t) / (t2 - t1), _path[i], _path[i + 1]);
            if (is_last) {
                if (circular) {
                    a3 = cr_f_mul_xy((t3 - t) / (t3 - t2), _path[i+1], _path[1]);
                } else {
                    a3 = _path[i+1];
                }
            } else {
                a3 = cr_f_mul_xy((t3 - t) / (t3 - t2), _path[i+1], _path[i+2]);
            }
            const XY b1 = cr_f_mul_xy((t2 - t) / (t2 - t0), a1, a2);
            const XY b2 = cr_f_mul_xy((t3 - t) / (t3 - t1), a2, a3);
            const XY pt = cr_f_mul_xy((t2 - t) / (t2 - t1), b1, b2);
            result.emplace_back(pt);
        }
    }
    _path = result;
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
