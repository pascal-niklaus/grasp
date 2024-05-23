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

#include "grasp.h"

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

std::ostream& operator<<(std::ostream& os, const TransformCaps& cap) {
    if (cap == CAP_NONE) {
        os << "--";
    } else {
        if (cap & CAP_MOVE) os << "M";
        if (cap & CAP_ROTATE) os << "R";
        if (cap & CAP_DELETE) os << "D";
        if (cap & CAP_ISOSCALE) os << "S";
        if (cap & CAP_ANISOSCALE) os << "A";
        if (cap & CAP_KEYSTROKES) os << "K";
        if (cap & CAP_DYNAMIC) os << "T";
    }
    return os;
}
