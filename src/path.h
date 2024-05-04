#ifndef PATH_H
#define PATH_H

#include "xy.h"

#include <vector>
#include <iostream>

class QTransform;
class Path;

// Section
//
// internal helper class used during orthogonalize
//
class Section {
  public:
    static const int NON_ORTHO_ANGLE = -999;

  public:
    explicit Section(int i0, int i1);
    XY dxdy() const;
    XY center() const;
    double len() const;
    bool is_ortho() const;
    bool is_vertical() const;
    double angle() const;
    static double angle_deg_snap(double angle, double tolerance, bool* snapped);

    friend std::ostream& operator<<(std::ostream& os, const Section& sec);

  public:
    XY start;
    XY end;
    int istart;
    int iend;
    int direction;
};

// Path
//
class Path {
  public:
    Path();
    ~Path();

    bool is_empty() const { return _path.empty(); }
    int size() const { return _path.size(); }
    XY& operator[](int i) { return _path[i]; }
    const XY& operator[](int i) const { return _path[i]; }
    const std::vector<XY>& path() const { return _path; };
    std::vector<XY>& path() { return _path; }

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
    void catmull_rom(int steps, bool circular);
    void orthogonalize(double max_angular_deviation, double min_ortho_len);
    void round_corners(double radius, int npoints, bool ends_join);
    bool snap_ends(double max_distance);
    double end_angle(double radius, bool end_is_beginning) const;
    void backtrack(int minpoints, int nsteps, int ndelete);

    friend std::ostream& operator<<(std::ostream& os, const Path& p);

  private:
    static inline XY cr_f_mul_xy(double f, const XY& p1, const XY& p2) {
        return XY(f * p1.x() + (1.0 - f) * p2.x(),
                  f * p1.y() + (1.0 - f) * p2.y());
    }
    static inline double catmull_rom_tj(double ti, const XY& pi, const XY& pj) {
        return ti + sqrt(pi.distance(pj));
    }
    void build_section_list(const double max_angular_deviation,
                       const double min_ortho_len);

  private:
    double _w;
    std::vector<XY> _path;
    std::vector<Section> _sections;
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
