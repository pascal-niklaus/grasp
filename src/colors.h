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

    // members
    QKeyCombination key1;
    QKeyCombination key2;
    QColor color;
};

#endif
