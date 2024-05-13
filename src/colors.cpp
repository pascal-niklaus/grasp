#include "colors.h"

#include <vector>
#include <iostream>
#include <QKeyCombination>

Color::Color(const QColor& color) {
    this->color = color;
}

Color::Color(std::uint32_t value) {
    this->color = Color::qcolor_from_uint32(value);
}

Color::Color(std::uint32_t value,
             Qt::Key k1,
             Qt::KeyboardModifiers m1,
             Qt::Key k2,
             Qt::KeyboardModifiers m2) {
    color = Color::qcolor_from_uint32(value);
    key1 = QKeyCombination(m1, k1);
    key2 = QKeyCombination(m2, k2);
}

Color::Color(std::uint32_t value,
             char k1,
             Qt::KeyboardModifiers m1,
             char k2,
             Qt::KeyboardModifiers m2) {
    color = Color::qcolor_from_uint32(value);
    key1 = QKeyCombination(m1, static_cast<Qt::Key>(k1));
    key2 = QKeyCombination(m2, static_cast<Qt::Key>(k2));
}

std::uint32_t Color::uint32_from_qcolor(const QColor& color) {
    return (color.red() << 24) + (color.green() << 16) + (color.blue() << 8) +
           color.alpha();
}

QColor Color::qcolor_from_uint32(std::uint32_t color) {
    return QColor(
        color >> 24, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

QString Color::qcolor_as_hex(const QColor& col) {
  if (col.alpha() != 0xff)
    return QString("#%1%2%3%4")
        .arg(col.red(), 2, 16, QChar('0'))
        .arg(col.green(), 2, 16, QChar('0'))
        .arg(col.blue(), 2, 16, QChar('0'))
        .arg(col.alpha(), 2, 16, QChar('0'));
  else
      return QString("#%1%2%3")
          .arg(col.red(), 2, 16, QChar('0'))
          .arg(col.green(), 2, 16, QChar('0'))
          .arg(col.blue(), 2, 16, QChar('0'));
}

QColor Color::qcolor_from_hex(const QString& txt) {
    bool ok;
    std::uint32_t x = txt.mid(1).toUInt(&ok, 16);
    if (ok && txt.length() == 7) {
        return Color::qcolor_from_uint32(x << 8 | 0xff);
    } else if (ok && txt.length() == 9) {
        return Color::qcolor_from_uint32(x);
    } else {
        std::cerr << "Warning: illegal color:" << txt.toStdString() << "\n";
        return Qt::white;
    }
}
