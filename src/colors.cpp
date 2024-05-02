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

std::uint32_t Color::get_color(std::uint32_t key, std::uint32_t mod)  {
  for (const auto& c : color_table) {
    if (c.key1 == key && (mod==0 || (c.mod1 & mod)))
      return c.numeric;
    if (c.key2 == key && (mod==0 || (c.mod2 & mod)))
      return c.numeric;
  }
  return 0;
}
