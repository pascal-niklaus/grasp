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

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>
#include <sstream>
#include <concepts>

template <typename T, typename U>
concept Multiplicable = requires(T a, U b) {
    a * b;
};

// square
//
// square of value
//
template <typename T>
requires Multiplicable<T, T>
T square(T x) { return x*x; };

// max
//
// maximum of two values (must be numeric)
//
template<typename T>
requires std::integral<T> || std::floating_point<T>
T max(T x, T y) { return x > y ? x : y; };

// cap(x, min, max)
//
// restricts x to range [min, max]
//
template <typename T>
requires std::integral<T> || std::floating_point<T>
T cap(T x, T min, T max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

// split
//
// split a string at the delimiter specified

template<typename Out>
void split(const std::string &s, const char delim, Out result) {
    const std::string padded = s + delim;
    std::stringstream ss(padded);
    std::string item;

    while (ss) {
        if (!std::getline(ss, item, delim)) break;
        *(result++) = item;
    }
}

// number of utf8 chars in a std::string
//
// no checking of validity is made
//
size_t utf8len(const std::string& str);

#endif
