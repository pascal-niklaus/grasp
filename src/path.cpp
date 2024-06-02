/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "path.h"

#include <QTransform>
#include <QDebug>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "utility.h"

Section::Section(int i0, int i1) {
    istart = i0;
    iend = i1;
}

XY Section::center() const {
    return start.midpoint(end);
}

XY Section::dxdy() const {
    return start.vector_to(end);
}

double Section::len() const {
    return start.distance(end);
}

bool Section::is_ortho() const {
    return (direction % 90) == 0;
}

bool Section::is_vertical() const {
    return direction % 180 == 90;
}

double Section::angle() const {
    return start.angle_to(end);
}

// check if angle is close to horizontal or vertical, within
// tolerance, and if they are, "snap" angle to these
double Section::angle_deg_snap(double angle, double tolerance, bool *snapped) {
    if (angle < 0) angle += 360.0;
    if (angle == 360.0) angle = 0.0;
    double ortho = round(angle / 90.0) * 90.0;
    if (fabs(angle - ortho) < tolerance) {
        if (snapped) *snapped = true;
        angle = remainderf(ortho, 360.0);
        if (angle < 0) angle += 360.0;
    } else {
        if (snapped) *snapped = false;
    }
    return angle;
}

std::ostream& operator<<(std::ostream& os, const Section& sec) {
    os << "Section(" << sec.istart << "-" << sec.iend << " : " << sec.start
       << " - " << sec.end << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Section>& secvec) {
    for (const auto& sec : secvec) os << sec << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<XY>& xyvec) {
    for (const auto& xy : xyvec) os << xy;
    os << "\n";
    return os;
}

// Path
//
// a path: a vector of XY coordinates
//
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

// Path::add_points
//
// insert points into segment so that inter-point distance becomes
// smaller than maxdist
//
void Path::add_points(double maxdist) {
    if (_path.size() >= 2) {
        std::vector<XY> result;
        const int last = _path.size() - 1;
        result.emplace_back(_path.front());
        for (int i = 1; i <= last; i++) {
            const double dist = _path[i - 1].distance(_path[i]);
            if (dist > maxdist) {
                const int steps = ceil(dist / maxdist);
                const auto& start = _path[i-1];
                const auto vec = _path[i-1].vector_to(_path[i]);
                for (int j = 1; j < steps; j++) {
                    const double k = static_cast<double>(j) / steps;
                    result.emplace_back(start + k * vec);
                }
            }
          result.emplace_back(_path[i]);
        }
        _path = result;
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

    if (to_delete.size()) {
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
    for (int i = 0; i + 1 <= last; i++) {
        XY a1, a2, a3, b1, b2, pt;
        double t0 = 0.0;
        double t1 = 0.0;
        const bool is_last = (i + 1 == last);
        if (i == 0) {
            if (circular) {
                // first point: take 2nd last point because first=last
                t1 = catmull_rom_tj(t0, _path[last - 1], _path[i]);
            } else {
                t1 = 0.0;
            }
        } else {
            t1 = catmull_rom_tj(t0, _path[i - 1], _path[i]);
        }
        double t2 = catmull_rom_tj(t1, _path[i], _path[i + 1]);
        double t3;
        if (is_last) {
            if (circular) {
                // last point: take 2nd point because first = last
                t3 = catmull_rom_tj(t2, _path[i + 1], _path[1]);
            } else {
                t3 = t2;
            }
        } else {
            t3 = catmull_rom_tj(t2, _path[i + 1], _path[i + 2]);
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
                    a3 = cr_f_mul_xy(
                        (t3 - t) / (t3 - t2), _path[i + 1], _path[1]);
                } else {
                    a3 = _path[i + 1];
                }
            } else {
                a3 = cr_f_mul_xy(
                    (t3 - t) / (t3 - t2), _path[i + 1], _path[i + 2]);
            }
            const XY b1 = cr_f_mul_xy((t2 - t) / (t2 - t0), a1, a2);
            const XY b2 = cr_f_mul_xy((t3 - t) / (t3 - t1), a2, a3);
            const XY pt = cr_f_mul_xy((t2 - t) / (t2 - t1), b1, b2);
            result.emplace_back(pt);
        }
    }
    _path = result;
}

// Path::build_section_list
//
// build list with 'Section's that are orthogonal (withing +-
// max_angular_deviation) or 'free'; unnecessary intermediate points
// in _path are deleted (in the orthogonal sections)
//
void Path::build_section_list(const double max_angular_deviation,
                              const double min_ortho_len) {
    _sections.clear();

    int i = 0;
    int last;

    while (i < (last = static_cast<int>(_path.size()) - 1)) {

        // start a section
        Section sec(i, i);

        // check if section is orthogonal
        bool ortho;
        int angle, angle0;
        while (sec.iend < last) {
            // look forward at next angle
            angle = _path[sec.iend].angle_to(_path[sec.iend + 1]) * 180 / M_PI;
            angle =
                Section::angle_deg_snap(angle, max_angular_deviation, &ortho);
            // if section not orthogonal: next step
            if (!ortho) break;
            if (sec.istart == sec.iend) angle0 = angle;
            // if change of direction: end of section reached
            else if (angle != angle0)
                break;
            sec.iend++;
        }

        // if section exceeds minimum length, add orthogonal section to list
        if (_path[sec.istart].distance(_path[sec.iend]) > min_ortho_len) {
            assert(sec.iend > sec.istart);
            // erase intermediate points in path
            _path.erase(_path.begin() + sec.istart + 1,
                        _path.begin() + sec.iend);
            sec.direction = angle0;
            sec.iend = sec.istart + 1;
            i = sec.iend;
            _sections.emplace_back(sec);
            continue;
        }

        // if not, include it in free (non-orthogonal) section
        while (sec.iend < last) {
            angle = _path[sec.iend].angle_to(_path[sec.iend + 1]) * 180 / M_PI;
            angle =
                Section::angle_deg_snap(angle, max_angular_deviation, &ortho);
            if (ortho) break;
            sec.iend++;
        }

        sec.direction = Section::NON_ORTHO_ANGLE;
        _sections.emplace_back(sec);

        i = sec.iend;
    }

    // cleanup: join successive non-orthogonal sections
    i = 0;
    while (i < (last = _sections.size() - 1)) {
        int dir = _sections[i].direction;
        int nextdir = _sections[i + 1].direction;
        if (dir == nextdir) {
            _sections[i].end = _sections[i + 1].end;
            _sections.erase(_sections.begin() + i + 1);
        } else {
            i++;
        }
    }

    // fill in start and end coordinates
    for (auto& sec : _sections) {
        sec.start = _path[sec.istart];
        sec.end = _path[sec.iend];
    }
}

void Path::orthogonalize(const double max_angular_deviation,
                         const double min_ortho_len) {
    build_section_list(max_angular_deviation, min_ortho_len);

    const int last = _sections.size() - 1;
    if (last > 0) {
        // determine "fixed" coordinate of H and V sections (x for V and y
        // for H) and adjust start and end points of path accordingly
        for (int i = 0; i <= last; i++) {
            Section &sec = _sections[i];
            if (sec.is_ortho()) {
                XY center;
                if (i == 0)
                    center = _sections[0].start;
                else if (i == last)
                    center = _sections[last].end;
                else
                    center = sec.center();
                if (sec.is_vertical()) {
                  sec.start.setX(center.x());
                  sec.end.setX(center.x());
                } else {
                  sec.start.setY(center.y());
                  sec.end.setY(center.y());
                }
            }
        }

        // now "join" ends of adjacent sections
        for (int i = 0; i <= last; i++) {
            Section &sec = _sections[i];
            if (sec.is_ortho()) {
                if (i < last && _sections[i+1].is_ortho()) {
                    Section &next_sec = _sections[i + 1];
                    // join orthogonal section to other orthogonal section
                    const bool first_v = sec.is_vertical();
                    const bool second_v = next_sec.is_vertical();
                    if (first_v != second_v) {
                        // sections meet at 90 degrees -> join them
                        if (first_v) {
                            sec.end.setY(next_sec.start.y());
                            next_sec.start.setX(sec.end.x());
                        } else {
                            sec.end.setX(next_sec.start.x());
                            next_sec.start.setY(sec.end.y());
                        }
                    } else {
                        // sections are parallel -> align ends
                        if (first_v) {
                            const auto y =
                                0.5 * (sec.end.y() + next_sec.start.y());
                            sec.end.setY(y);
                            next_sec.start.setY(y);

                        } else {
                          const auto x = 0.5 * (sec.end.x() + next_sec.start.x());
                          sec.end.setX(x);
                          next_sec.start.setX(x);
                        }
                    }
                } else {
                    if (i > 0) {
                        Section& prev_sec = _sections[i - 1];
                        if (prev_sec.is_ortho()) {
                            sec.start = prev_sec.end;
                        }
                    }
                }
            } else {
                // section is not orthogonal
                QTransform m, t;
                if (i > 0 && i < last) {
                    Section& prev_sec = _sections[i - 1];
                    Section& sec = _sections[i];
                    Section& next_sec = _sections[i + 1];
                    bool prev_vert = prev_sec.is_vertical();
                    bool next_vert = next_sec.is_vertical();

                    // one could fine-tune the orientation of the free section,
                    // based on how skewed the adjacent orthogonal sections are.
                    // gfloat angle = 0.5 * (remainderf(section_angle(prev_sec), M_PI_2) +
                    //                       remainderf(section_angle(next_sec), M_PI_2));
                    // rotate2D(angle, &m);  // or -angle ??
                    // apply2D(sec->start, sec->end, &m);

                    XY delta = sec.dxdy();

                    if (prev_vert != next_vert) {
                        // prev & next sections are at 90 degrees ->
                        // adjust their lengths to accomodate intermediate section.
                        //
                        // TODO: this could produce artifacts when ends would be moved back
                        // too much; then, the section could be scaled down to fit or rotated.
                        // But this does not seem to happen...
                        if (prev_vert) {
                            next_sec.start.setX(prev_sec.end.x() + delta.x());
                            prev_sec.end.setY(next_sec.start.y() - delta.y());
                        } else {
                            next_sec.start.setY(prev_sec.end.y() + delta.y());
                            prev_sec.end.setX(next_sec.start.x() - delta.x());
                        }
                        // update fields in Section
                        sec.start = prev_sec.end;
                        sec.end = next_sec.start;
                        // move section to right place
                        m = QTransform::fromTranslate(prev_sec.end.x() - sec.start.x(),
                                                      prev_sec.end.y() - sec.start.y());
                    } else {
                      // prev & next are somehow parallel -> fit free
                      // section between adjacent sections
                      double x0 = prev_sec.end.x();
                      double y0 = prev_sec.end.y();
                      m = QTransform::fromTranslate(x0, y0);

                      const double rot = XY::angle_between(
                          sec.start, sec.end, prev_sec.end, next_sec.start);
                      m.rotateRadians(rot);

                      const double scale = prev_sec.end.distance(next_sec.start) / sec.len();
                      m.scale(scale, scale);

                      m.translate(-sec.start.x(), -sec.start.y());
                    }

                    for (int j=sec.istart; j<=sec.iend; j++)
                        _path[j].apply2d(m);
                }
            }
        }

        // copy start and end points of orthogonal sections to coords
        for (int i = 0; i <= last; i++) {
            Section& sec = _sections[i];
            if (sec.is_ortho()) {
                _path[sec.istart] = sec.start;
                _path[sec.iend] = sec.end;
            }
        }
    }
}

// the code below works ok, the sequence is
//
// 1)

void Path::test_rotation() {
  // destination
  XY dest1(2, 3);
  XY dest2(7, 5);

  // points to move there
  XY src1(3, 2);
  XY src2(6, 2.5);

  auto m = QTransform::fromTranslate(dest1.x(), dest1.y());

  double rot = XY::angle_between(src1, src2, dest1, dest2);
  // const XY v1(src2 - src1);
  // const XY v2(dest2 - dest1);

  // const double rot = atan2(v1.x() * v2.y() - v2.x() * v1.y(),
  //                          v1.x() * v2.x() + v1.y() * v2.y());

  m.rotateRadians(rot);

  const double scale = dest1.distance(dest2) / src1.distance(src2);
  m.scale(scale, scale);
  qDebug() << "rot=" << rot * 180 / M_PI << "°, scale=" << scale << "\n";

  m.translate(-src1.x(), -src1.y());

  qDebug() << "src1 was at" << src1 << ", maps to" << m.map(src1)
           << ", should be at " << dest1;
  qDebug() << "src2 was at" << src2 << ", maps to" << m.map(src2)
           << ", should be at " << dest2;
}

void Path::round_corners(double radius, int npoints, bool ends_join) {
    const int last = _path.size() - 1;
    if (last > 1) {
        std::vector<XY> result;
        double prev_len = 0, next_len = 0;
        for (int i = 0; i <= last; i++) {
            const XY& next_pt = (i == last) ? _path[1] : _path[i + 1];
            if (ends_join || i < last) {
                prev_len = next_len;
                next_len = _path[i].distance(next_pt);
                if (i > 0 && next_len > 2 * radius && prev_len > 2 * radius) {
                    // there is space for a round corner !
                    const XY& prev_pt = _path[i - 1];
                    XY pt = _path[i];

                    const XY v1(pt - prev_pt);
                    const XY v2(next_pt - pt);
                    const double rot = atan2(v1.x() * v2.y() - v2.x() * v1.y(),
                                             v1.x() * v2.x() + v1.y() * v2.y());
                    const double beta = rot / npoints;
                    const double a = 2 * radius * tan((M_PI - rot) / 2) *
                                     sin(rot / (2 * npoints));

                    // move back current point by radius
                    XY dirvec = prev_pt.vector_to(pt);
                    double k = radius / dirvec.vector_length();
                    pt -= k * dirvec;

                    result.emplace_back(pt);

                    // initial step with length a
                    k *= (a / radius);
                    dirvec.setX(dirvec.x() * k);
                    dirvec.setY(dirvec.y() * k);

                    QTransform m;
                    m.rotateRadians(-beta / 2.0);
                    dirvec.apply2d(m);
                    m.reset();
                    m.rotateRadians(beta);

                    for (int j = 0; j < npoints; j++) {
                        dirvec.apply2d(m);
                        pt += dirvec;
                        result.emplace_back(pt);
                    }

                    if (i==last)
                      result.front() = result.back();


                } else {
                    // there is no space, just insert the point !
                    result.emplace_back(_path[i]);
                }
            } else {
                result.emplace_back(_path[i]);
            }
        }
        _path = result;
    }
}

/*
 * check for back-tracking once there are at least 'minpoints' points
 * in the path.  The 'nsteps' last points are checked for a monotonic
 * decrease of the distance to the last point. If this is not the
 * case, this is interpreted as sign of back-tracking and 'ndelete'
 * points before the last point are removed.
 */

void Path::backtrack(int minpoints, int nsteps, int ndelete) {
  if (static_cast<int>(_path.size()) > minpoints) {
        std::vector<double> distances;
        int last = _path.size() - 1;

        // calculate and store distances
        for (int n = 0; n < nsteps; n++)
            distances.emplace_back(_path[last].distance(_path[last - 1 - n]));

        // check whether distances increase monotonically, and find
        // likely "closest" point after which one should remove points
        bool backtracking = false;
        int last_decr = 0;
        for (int n = nsteps - 2; n >= 0; n--) {
            if (distances[n] > distances[n + 1]) {
                backtracking = true;
            } else if (last_decr == 0) {
                last_decr = n;
            }
        }
        if (backtracking) {
            // delete n points before last one
            int n = std::min<int>(last_decr, ndelete);
            _path[last - n] = _path[last];
            _path.resize(last - n);
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Path& p) {
  os << "Path @ " << &p << ", len=" << p.size() << " :";
  for (const auto& point : p._path) {
    os << " " << point;
  }
  os << "\n";
  return os;
}

// PathW
//
// A path with variable stroke width

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
