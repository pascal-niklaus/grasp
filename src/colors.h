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

#ifndef COLORS_H
#define COLORS_H

#include <QColor>
#include <QKeyCombination>
#include <QString>

#include <cstdint>

class Color {
  public:
    explicit Color(const QColor& color);
    explicit Color(std::uint32_t value);
    Color(std::uint32_t value,
          Qt::Key k1,
          Qt::KeyboardModifiers m1,
          Qt::Key k2,
          Qt::KeyboardModifiers m2);
    Color(std::uint32_t value,
          char k1,
          Qt::KeyboardModifiers m1,
          char k2,
          Qt::KeyboardModifiers m2);

    static QString qcolor_as_hex(const QColor& color);
    static QColor qcolor_from_hex(const QString& color);

    static std::uint32_t uint32_from_qcolor(const QColor& color);
    static QColor qcolor_from_uint32(std::uint32_t color);

    QColor color() const { return _color; }
    QKeyCombination key1() const { return _key1; }
    QKeyCombination key2() const { return _key2; }

    void set_color(const QColor& color) { _color = color; }
    void set_key1(const QKeyCombination& kc) { _key1 = kc; }
    void set_key2(const QKeyCombination& kc) { _key2 = kc; }

  private:
    // members
    QKeyCombination _key1;
    QKeyCombination _key2;
    QColor _color;
};

#endif
