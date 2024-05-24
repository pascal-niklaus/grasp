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

#ifndef TOOLS_H
#define TOOLS_H

#include <QColor>
#include <QFont>

#include "grasp.h"
#include "colors.h"

class QGraphicsItem;
class QSettings;

// Tool::Settings
//
// all settings defining the tools
//
class Settings {
  public:
    Settings();
    // settings that change while the user draws
    ToolType tool;
    QColor color;
    double width;
    double highlight_width;
    double minwidth;
    double maxwidth;
    ArrowType arrow_type;

    // tool parameters that stay constant
    double arrow_size;
    double radius_rect;
    double radius_ortho;
    double minlen_ortho;
    double maxangle_ortho;
    double simplify_smooth;
    double simplify_ortho;
    double snapdist_smooth;
    double snapdist_ortho;
    QFont font;
    double sustain_fader;
    double fade_fader;
    double sustain_highlight;
    double fade_highlight;
    double grab_delay;

    // shortcuts
    QKeyCombination edit_key;
    QKeyCombination pen_key;
    QKeyCombination smooth_key;
    QKeyCombination ortho_key;
    QKeyCombination line_key;
    QKeyCombination rect_key;
    QKeyCombination text_key;
    QKeyCombination highlight_key;
    QKeyCombination fader_key;

    QKeyCombination del_key;
    QKeyCombination del_all_key;
    QKeyCombination dash_key;
    QKeyCombination grab_key;
    QKeyCombination grab_partial_key;

    QKeyCombination arrow_start_key;
    QKeyCombination arrow_end_key;
    QKeyCombination arrow_double_key;
    QKeyCombination arrow_none_key;

    QKeyCombination quit_key;
    QKeyCombination undo_key;
    QKeyCombination redo_key;
    QKeyCombination opacity_key;
    QKeyCombination toggle_key;

    // line widths and shortcut keys
    QKeyCombination width_key[9];
    double line_widths[9];
    double highlight_line_width[9];

    // color table
    std::vector<Color> color_table;

    // directories
    QString save_dir;

  public:
    static QString to_string(const QKeyCombination& kc);
    static QKeyCombination keycomb_from_string(const QString& str);

  public:
    QColor get_color(const QKeyCombination& key);

    [[deprecated("if used, check return value is non-negative")]]
    static int qcolor_find_index(std::uint32_t color);

    QColor qcolor_at_index(int index) const;
    int qcolor_find_index(const QColor& color) const;

    void reset_defaults();
    void store(QSettings& qs) const;
    bool retrieve(QSettings& qs);
};

namespace Tools {

// Tool::create_tool
//
// factory function
//
[[nodiscard("the returned item needs to be deleted later")]]
QGraphicsItem* create_tool(const Settings& settings);

int generate_ID(void);

};  // namespace Tools

#endif
