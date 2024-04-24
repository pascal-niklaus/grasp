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
