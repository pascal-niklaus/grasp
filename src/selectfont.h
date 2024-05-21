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

#ifndef SELECTFONT_H
#define SELECTFONT_H

#include <QPushButton>
#include <QFont>

class SelectFont : public QPushButton {
    Q_OBJECT
  public:
    SelectFont(QWidget* parent = nullptr);

    void set_font(const QFont& font);
    const QFont& get_font() const { return _font; }
    void set_display_size_range(double min, double max);

  private slots:
    void slot_chose_font();

  private:
    QFont _font;
    double _min_display_size = 1.0;
    double _max_display_size = 100.0;
};

#endif
