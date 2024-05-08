#ifndef COLORS_H
#define COLORS_H

#include <QColor>

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
  static QColor qcolor_from_uint32(std::uint32_t color);
  static std::uint32_t uint32_from_qcolor(const QColor& color);
  static int qcolor_find_index(std::uint32_t color);

  // lookup table
  static std::vector<Color> color_table;

  // members
  std::string name;
  std::uint32_t key1, key2;
  std::uint32_t mod1, mod2;
  std::uint32_t numeric;

};

#endif
