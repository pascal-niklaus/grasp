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

#include "selectfont.h"
#include "utility.h"

#include <QFontDialog>

SelectFont::SelectFont(QWidget* parent) : QPushButton(parent) {
    connect(this, &SelectFont::clicked, this, &SelectFont::slot_chose_font);
}

void SelectFont::set_font(const QFont &font) {
    _font = font;
    QFont tmp = _font;
    tmp.setPointSizeF(cap<double>(tmp.pointSizeF(),_min_display_size, _max_display_size));
    setFont(tmp);
    setText(QString("%1 %2").arg(font.family()).arg(font.pointSize()));
}

void SelectFont::slot_chose_font() {
    bool ok;
    auto font = QFontDialog::getFont(&ok, _font, nullptr, tr("Chose font"));
    if (ok) { set_font(font); }
}

void SelectFont::set_display_size_range(double min, double max) {
    _min_display_size = min;
    _max_display_size = max;
}
