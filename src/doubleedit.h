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


#ifndef DOUBLEEDIT_H
#define DOUBLEEDIT_H

#include <QLineEdit>

class QDoubleValidator;

class DoubleEdit : public QLineEdit {
    Q_OBJECT
  public:
    DoubleEdit(QWidget *parent = nullptr);
    double get_value() const;
    void set_value(double val);
    void set_range(double low, double high, int decimals = 1);

  private:
    QDoubleValidator *_validator;
};

#endif
