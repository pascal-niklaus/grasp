#ifndef COLORS_H
#define COLORS_H

#include <cstdint>
#include <string>
#include <vector>

class Color {
  public:
    Color(const std::string& name,
          std::uint32_t hex,
          std::uint32_t key1 = 0,
          std::uint32_t mod1 = 0,
          std::uint32_t key2 = 0,
          std::uint32_t mod2 = 0);

  static std::uint32_t get_color(std::uint32_t key, std::uint32_t mod);

  // lookup table
  static std::vector<Color> color_table;

  // members
  std::string name;
  std::uint32_t key1, key2;
  std::uint32_t mod1, mod2;
  std::uint32_t numeric;

};

#endif
