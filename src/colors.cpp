#include "colors.h"

#include <vector>
#include <iostream>
#include <QKeyCombination>

std::vector<Color> Color::color_table = {
    {"white"  , 0xf0f0f0ff  , 'W' , Qt::NoModifier    , 'W' , Qt::ShiftModifier } ,
    {"grey"   , 0x909090ff  , 'M' , Qt::NoModifier    , 'M' , Qt::ShiftModifier } ,
    {"black"  , 0x2f2f2fff  , 'N' , Qt::NoModifier    , 'N' , Qt::ShiftModifier } ,
    {"blue"   , 0x1e90ffff  , 'B' , Qt::NoModifier    , 'B' , Qt::ShiftModifier } ,
    {"cyan"   , 0x00ffffff  , 'C' , Qt::NoModifier    , 'C' , Qt::ShiftModifier } ,
    {"red"    , 0xff0000ff  , 'R' , Qt::ShiftModifier , 'I' , Qt::NoModifier }    ,
    {"orange" , 0xffa500ff  , 'O' , Qt::ShiftModifier , 'A' , Qt::NoModifier }    ,
    {"green"  , 0x32cd32ff  , 'G' , Qt::NoModifier    , 'G' , Qt::ShiftModifier } ,
    {"forest" , 0x185d27ff  , 'F' , Qt::ShiftModifier , 0   , Qt::NoModifier }    ,
    {"violet" , 0xff00ffff  , 'V' , Qt::NoModifier    , 'V' , Qt::ShiftModifier } ,
    {"yellow" , 0xffff00ff  , 'Y' , Qt::NoModifier    , 'Y' , Qt::ShiftModifier } ,
    {"brown"  , 0x662c15ff  , 'K' , Qt::NoModifier    , 'K' , Qt::ShiftModifier } ,
};

Color::Color(const std::string& name,
             std::uint32_t hex,
             std::uint32_t key1,
             std::uint32_t mod1,
             std::uint32_t key2,
             std::uint32_t mod2) {
    this->name = name;
    this->key1 = key1;
    this->key2 = key2;
    this->mod1 = mod1;
    this->mod2 = mod2;
    this->numeric = hex;
}

std::uint32_t Color::uint32_from_qcolor(const QColor& color) {
    return (color.red() << 24) + (color.green() << 16) + (color.blue() << 8) +
           color.alpha();
}

QColor Color::qcolor_from_uint32(std::uint32_t color) {
    return QColor(
        color >> 24, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

int Color::qcolor_find_index(std::uint32_t color) {
    const auto it = std::find_if(
        Color::color_table.cbegin(),
        Color::color_table.cend(),
        [&color](const Color& item) { return item.numeric == color; });
    if (it == Color::color_table.cend()) return -1;
    return std::distance(Color::color_table.cbegin(), it);
}

std::uint32_t Color::get_color(std::uint32_t key, std::uint32_t mod)  {
  for (const auto& c : color_table) {
      if (c.key1 == key && ((mod == 0 && c.mod1 == 0) || (c.mod1 & mod)))
          return c.numeric;
      if (c.key2 == key && ((mod == 0 && c.mod2 == 0) || (c.mod2 & mod)))
          return c.numeric;
  }
  return 0;
}
