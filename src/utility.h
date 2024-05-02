#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>
#include <sstream>

template<typename T>
T square(T x) { return x*x; };

template<typename T>
T max(T x, T y) { return x > y ? x : y; };


template<typename T>
T cap(T x, T min, T max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}


////////////////////////////////////////////////////////////
//
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

size_t utf8len(const std::string& str);

#endif
