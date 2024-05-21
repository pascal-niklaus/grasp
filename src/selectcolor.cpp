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

#include "selectcolor.h"

#include <QColorDialog>

SelectColorButton::SelectColorButton( QWidget* parent )
    : QPushButton(parent) {
    connect(this,
            &SelectColorButton::clicked,
            this,
            &SelectColorButton::change_color);
}

void SelectColorButton::update_color() {
    setStyleSheet("background-color: " + _color.name());
}

void SelectColorButton::change_color() {
    QColor new_color = QColorDialog::getColor(_color, parentWidget());
    if (new_color != _color)
        set_color(new_color);
}

void SelectColorButton::set_color( const QColor& color ) {
    _color = color;
    update_color();
}
