#include "kromit.h"


TransformCaps operator&(const TransformCaps& a, const TransformCaps& b) {
    auto ia = static_cast<std::uint16_t>(a);
    auto ib = static_cast<std::uint16_t>(b);
    return static_cast<TransformCaps>(ia & ib);
}

TransformCaps operator|(const TransformCaps& a, const TransformCaps& b) {
    auto ia = static_cast<std::uint16_t>(a);
    auto ib = static_cast<std::uint16_t>(b);
    return static_cast<TransformCaps>(ia | ib);
}

std::ostream &operator<<(std::ostream &os, const TransformCaps &cap) {
  if (cap == CAP_NONE) {
        os << "--";
  } else {
    if (cap & CAP_MOVE) os << "M";
    if (cap & CAP_ROTATE) os << "R";
    if (cap & CAP_DELETE) os << "D";
    if (cap & CAP_ISOSCALE) os << "S";
    if (cap & CAP_ANISOSCALE) os << "A";
  }
  return os;
}
