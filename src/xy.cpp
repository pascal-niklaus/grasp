#include <qpoint.h>
#include <qtransform.h>
#include <QTransform>

#include <cmath>

#include "xy.h"
#include "utility.h"
#include "string_format.h"

XY::XY() : QPointF() {}

XY::XY(double x, double y) : QPointF(x, y) {}

XY::XY(const QPointF& p) : QPointF(p) {}


// XY::midpoint
//
// return midpoint between XY and other point
//
XY XY::midpoint(const XY &p) const {
  return XY(0.5 * (x() + p.x()), 0.5 * (y() + p.y()));
};

// XY::vector_to
//
// return vector from XY to other XY
//
XY XY::vector_to(const XY& p) const {
    return XY(p.x() - x(), p.y() - y());
}

// XY::vector_length
//
// return length
//
double XY::vector_length() const {
    return sqrt(square(x()) + square(y()));
}

// XY::angle_to
//
// returns angle to point p
//
double XY::angle_to(const XY &p) const {
    return atan2(p.y() - y(), p.x() - x());
}

// XY::angle_between
//
// returns the angle [in radians] that is necessary, in a QTransform operation,
// to rotate (src1->src2) into (dst1->dst2)
//
double XY::angle_between(const XY& src1,
                         const XY& src2,
                         const XY& dst1,
                         const XY& dst2) {
    const XY v1(src2 - src1);
    const XY v2(dst2 - dst1);

    const double rot = atan2(v1.x() * v2.y() - v2.x() * v1.y(),
                             v1.x() * v2.x() + v1.y() * v2.y());
    return rot;
}

// XY::distance
//
// return distance between XY and other point
//
double XY::distance(const XY& p) const {
    return sqrt(square(p.x() - x()) + square(p.y() - y()));
}

// XY::distance
//
// return distance between two points
//
double XY::distance(const XY& p1, const XY& p2) {
  return sqrt(square(p1.x() - p2.x()) + square(p1.y() - p2.y()));
}

// XY::distance_from_line
//
// return distance from a line. If the line is very short, return the
// distance from the midpoint between the ends

double XY::distance_from_line(const XY& start, const XY& end) const {
    const double l =
        sqrt(square(start.x() - end.x()) + square(start.y() - end.y()));

    if (l < 0.1) {
        return distance(start.midpoint(end));
    } else {
        const double a = (start.y() - end.y()) * x() +
                         (end.x() - start.x()) * y() +
                         (start.x() * end.y() - end.x() * start.y());
        return fabs(a / l);
    }
}

// XY::distance_from_segment
//
// like distance_from_line, but assuming line ends at its endpoints
//
double XY::distance_from_segment(const XY &start, const XY &end) const {
  double dx = end.x() - start.x();
  double dy = end.y() - start.y();
  const double dot = (x() - start.x()) * dx + (y() - start.y()) * dy;
  const double l2  = dx*dx + dy*dy;

  double t = 0.5;
  if (l2 > 0.001) t = dot / l2;
  if (t < 0) t = 0;
  if (t > 1) t = 1;

  dx = start.x() + t*dx - x();
  dy = start.y() + t*dy - y();

  return sqrt(dx*dx + dy*dy);
};

// XY:apply2D
//
// apply a affine 2D transformation
//
// we don't use the cairo functions because this is simple and does
// not involved a function call, hence faster
//
void XY::apply2d(const QTransform &m) {
  qreal xt, yt;
  m.map(x(), y(), &xt, &yt);
  setX(xt);
  setY(yt);
}

void XY::apply2d(const QTransform &m, XY& result) const {
  qreal xt, yt;
  m.map(x(), y(),&xt, &yt);
  result.setX(xt);
  result.setY(yt);
}

// XY::transforme2d
//
// apply a affine 2D transformation and return as new XY
//
XY XY::transformed2d(const QTransform& m) const {
    qreal xt, yt;
    m.map(x(), y(), &xt, &yt);
    return XY(xt, yt);
}


// ------------------------ stream operators -------------------------

std::ostream & operator<<(std::ostream &os, const XY& p) {
  return os << string_format("(%.1f,%.1f)", p.x(), p.y());
}

std::ostream & operator<<(std::ostream &os, const XYW& p) {
  return os << string_format("(%.1f,%.1f, w=%.1f)", p.x(), p.y(), p.w());
}
