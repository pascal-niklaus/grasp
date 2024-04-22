#ifndef UTILITY_HPP
#define UTILITY_HPP

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



#endif
