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

#include "tools.h"

#include "gfxline.h"
#include "gfxpath.h"
#include "gfxsmooth.h"
#include "gfxortho.h"
#include "gfxrect.h"
#include "gfxtext.h"
#include "gfxpixmap.h"
#include "gfxfader.h"
#include "gfxhighlight.h"

#include <QSettings>

int Tools::generate_ID(void) {
    static int id = 0;
    return ++id;
}

QGraphicsItem *Tools::create_tool(const Settings& settings) {
    switch (settings.tool) {
        case TOOL_ORTHO: {
           auto *ortho = new GfxOrtho(nullptr,
                                         settings.color,
                                         settings.width,
                                         settings.arrow_type,
                                         settings.arrow_size,
                                         settings.simplify_ortho,
                                         settings.snapdist_ortho,
                                         settings.radius_ortho,
                                         settings.minlen_ortho,
                                         settings.maxangle_ortho);
            ortho->setData(GfxItem::KeyID, generate_ID());
            return ortho;
        }

        case TOOL_SMOOTH: {
           auto *pen = new GfxSmooth(nullptr,
                                         settings.color,
                                         settings.width,
                                         settings.arrow_type,
                                         settings.arrow_size,
                                         settings.simplify_smooth,
                                         settings.snapdist_smooth);
            pen->setData(GfxItem::KeyID, generate_ID());
            return pen;
        }

        case TOOL_PEN: {
            auto *pen = new GfxPath(nullptr,
                                       settings.color,
                                       settings.width,
                                       settings.arrow_type,
                                       settings.arrow_size);
            pen->setData(GfxItem::KeyID, generate_ID());
            return pen;
        }

        case TOOL_FADER: {
            auto *fader = new GfxFader(nullptr,
                                        settings.color,
                                        settings.width,
                                        settings.arrow_type,
                                        settings.arrow_size,
                                        settings.sustain_fader,
                                        settings.fade_fader);
            fader->setData(GfxItem::KeyID, generate_ID());
            return fader;
        }

        case TOOL_HIGHLIGHT: {
            auto *highlight = new GfxHighlight(nullptr,
                                               settings.color,
                                               settings.highlight_width,
                                               settings.arrow_type,
                                               settings.arrow_size,
                                               settings.sustain_highlight,
                                               settings.fade_highlight);
            highlight->setData(GfxItem::KeyID, generate_ID());
            return highlight;
        }

        case TOOL_LINE: {
            auto *line = new GfxLine(nullptr,
                                     settings.color,
                                     settings.width,
                                     settings.arrow_type,
                                     settings.arrow_size);
            line->setData(GfxItem::KeyID, generate_ID());
            return line;
        }

        case TOOL_RECT: {
            auto *rect = new GfxRect(
                nullptr, settings.color, settings.width, settings.radius_rect);
            rect->setData(GfxItem::KeyID, generate_ID());
            return rect;
        }

        case TOOL_TEXT: {
            auto *text = new GfxText(nullptr, settings.color, settings.font);
            text->setData(GfxItem::KeyID, generate_ID());
            return text;
        }

        case TOOL_PIXMAP: {
            auto *image = new GfxPixmap(nullptr, settings.color, 2);
            image->setData(GfxItem::KeyID, generate_ID());
            return image;
        }

        default:
            return nullptr;
    }
}

// Settings::Settings
//
// set sensible default values
//
Settings::Settings() {
    reset_defaults();

    tool = TOOL_SMOOTH;
    color = Qt::yellow;
    width = 5;
    highlight_width = 30;
    arrow_type = ARROW_END;
    arrow_size = 3;
    minwidth = 1;
    maxwidth = 4;
}

void Settings::reset_defaults() {
    // colors
    color_table = {
        {0xf0f0f0ff, 'W', Qt::NoModifier, 'W', Qt::ShiftModifier},
        {0x909090ff, 'M', Qt::NoModifier, 'M', Qt::ShiftModifier},
        {0x2f2f2fff, 'N', Qt::NoModifier, 'N', Qt::ShiftModifier},
        {0x1e90ffff, 'B', Qt::NoModifier, 'B', Qt::ShiftModifier},
        {0x00ffffff, 'C', Qt::NoModifier, 'C', Qt::ShiftModifier},
        {0xff0000ff, 'R', Qt::ShiftModifier, 'I', Qt::NoModifier},
        {0xffa500ff, 'O', Qt::ShiftModifier, 'A', Qt::NoModifier},
        {0x32cd32ff, 'G', Qt::NoModifier, 'G', Qt::ShiftModifier},
        {0x185d27ff, 'F', Qt::ShiftModifier, 0, Qt::NoModifier},
        {0xff00ffff, 'V', Qt::NoModifier, 'V', Qt::ShiftModifier},
        {0xffff00ff, 'Y', Qt::NoModifier, 'Y', Qt::ShiftModifier},
        {0x662c15ff, 'K', Qt::NoModifier, 'K', Qt::ShiftModifier},
    };

    // line widths
    double w0 = 1.0;
    double delta = 1.0;
    for (int i = 0; i < 9; i++) {
        width_key[i] = QKeyCombination(Qt::NoModifier,
                                       static_cast<Qt::Key>(Qt::Key_1 + i));
        line_widths[i] = round(w0);
        w0 += delta;
        delta = pow(delta, 1.1) + 1.1;
    }

    w0 = 8;
    delta = 5;
    for (int i = 0; i < 9; i++) {
        highlight_line_width[i] = round(w0);
        w0 += delta;
        delta = pow(delta, 1.02) + 5;
    }

    // tool keys
    edit_key = QKeyCombination(Qt::NoModifier, Qt::Key_E);
    pen_key = QKeyCombination(Qt::NoModifier, Qt::Key_P);
    smooth_key = QKeyCombination(Qt::NoModifier, Qt::Key_S);
    ortho_key = QKeyCombination(Qt::NoModifier, Qt::Key_O);
    line_key = QKeyCombination(Qt::NoModifier, Qt::Key_L);
    rect_key = QKeyCombination(Qt::NoModifier, Qt::Key_R);
    text_key = QKeyCombination(Qt::NoModifier, Qt::Key_T);
    highlight_key = QKeyCombination(Qt::NoModifier, Qt::Key_H);
    fader_key = QKeyCombination(Qt::NoModifier, Qt::Key_F);

    del_key = QKeyCombination(Qt::NoModifier, Qt::Key_Delete);
    del_all_key = QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier,
                                  Qt::Key_Delete);
    dash_key = QKeyCombination(Qt::NoModifier, Qt::Key_F1);
    grab_key = QKeyCombination(Qt::ControlModifier, Qt::Key_G);
    grab_partial_key = QKeyCombination(Qt::ShiftModifier, Qt::Key_G);

    arrow_start_key = QKeyCombination(Qt::NoModifier, Qt::Key_Less);
    arrow_end_key = QKeyCombination(Qt::NoModifier, Qt::Key_Greater);
    arrow_double_key = QKeyCombination(Qt::NoModifier, Qt::Key_Equal);
    arrow_none_key = QKeyCombination(Qt::NoModifier, Qt::Key_Minus);

    // other keys
    quit_key = QKeyCombination(Qt::ControlModifier, Qt::Key_Q);
    undo_key = QKeyCombination(Qt::ControlModifier, Qt::Key_Z);
    redo_key = QKeyCombination(Qt::ControlModifier, Qt::Key_Y);
    opacity_key = QKeyCombination(Qt::ControlModifier, Qt::Key_O);
    toggle_key = QKeyCombination(Qt::ControlModifier, Qt::Key_T);

    // set default tool settings
    radius_rect = 30;
    radius_ortho = 30;
    minlen_ortho = 70;
    simplify_smooth = 15;
    simplify_ortho = 25;
    snapdist_ortho = 20;
    snapdist_smooth = 20;
    maxangle_ortho = 15;
    font = QFont("Noto sans", 12);
    sustain_fader = 5.0;
    fade_fader = 5.0;
    sustain_highlight = 5.0;
    fade_highlight = 5.0;
    grab_delay = 0.25;
}

void Settings::store(QSettings &qs) const {
    qs.beginGroup("Keys");

    qs.setValue("edit-key", Settings::to_string(edit_key));
    qs.setValue("pen-key", Settings::to_string(pen_key));
    qs.setValue("line-key", Settings::to_string(line_key));
    qs.setValue("rect-key", Settings::to_string(rect_key));
    qs.setValue("smooth-key", Settings::to_string(smooth_key));
    qs.setValue("ortho-key", Settings::to_string(ortho_key));
    qs.setValue("highlight-key", Settings::to_string(highlight_key));
    qs.setValue("fader-key", Settings::to_string(fader_key));
    qs.setValue("text-key", Settings::to_string(text_key));

    qs.setValue("delete-key", Settings::to_string(del_key));
    qs.setValue("delete-all-key", Settings::to_string(del_all_key));
    qs.setValue("dash-key", Settings::to_string(dash_key));
    qs.setValue("grab-key", Settings::to_string(grab_key));
    qs.setValue("grab-partial-key", Settings::to_string(grab_partial_key));

    qs.setValue("quit-key", Settings::to_string(quit_key));
    qs.setValue("undo-key", Settings::to_string(undo_key));
    qs.setValue("redo-key", Settings::to_string(redo_key));
    qs.setValue("toggle-key", Settings::to_string(toggle_key));
    qs.setValue("opacity-key", Settings::to_string(opacity_key));

    qs.endGroup();

    // ---------------------------- tools ----------------------------

    qs.beginGroup("Tools");

    qs.beginGroup("Rect");
    qs.setValue("radius", radius_rect);
    qs.endGroup();

    qs.beginGroup("Smooth");
    qs.setValue("simplify", simplify_smooth);
    qs.setValue("snapdist", snapdist_smooth);
    qs.endGroup();

    qs.beginGroup("Ortho");
    qs.setValue("simplify", simplify_ortho);
    qs.setValue("snapdist", snapdist_ortho);
    qs.setValue("radius", radius_ortho);
    qs.setValue("minlen", minlen_ortho);
    qs.setValue("maxangle", maxangle_ortho);
    qs.endGroup();

    qs.beginGroup("Highlight");
    qs.setValue("sustain", sustain_highlight);
    qs.setValue("fade", fade_highlight);
    qs.endGroup();

    qs.beginGroup("Fader");
    qs.setValue("sustain", sustain_fader);
    qs.setValue("fade", fade_fader);
    qs.endGroup();

    qs.beginGroup("Text");
    qs.setValue("font", font.toString());
    qs.endGroup();

    qs.endGroup();

    qs.beginGroup("Colors");
    for (int i = 0; i < static_cast<int>(color_table.size()); i++) {
        const Color &col = color_table[i];
        qs.beginGroup(QString("%1").arg(i, 2, 10, QChar('0')));
        qs.setValue("color", Color::qcolor_as_hex(color_table[i].color));
        qs.setValue("key1", to_string(col.key1));
        qs.setValue("key2", to_string(col.key2));

        qs.endGroup();
    }
    qs.endGroup();

    qs.beginGroup("Linewidths");
    for (int i = 0; i < 9; i++) {
        qs.beginGroup(QString("%1").arg(i, 2, 10, QChar('0')));
        qs.setValue("width", line_widths[i]);
        qs.setValue("highligher-width", highlight_line_width[i]);
        qs.setValue("key", to_string(width_key[i]));
        qs.endGroup();
    }
    qs.endGroup();

    qs.beginGroup("Arrows");
    qs.setValue("arrow-beginning", to_string(arrow_start_key));
    qs.setValue("arrow-end", to_string(arrow_end_key));
    qs.setValue("arrow-double", to_string(arrow_double_key));
    qs.setValue("arrow-none", to_string(arrow_none_key));
    qs.setValue("size", arrow_size);
    qs.endGroup();

    qs.beginGroup("Directories");
    qs.setValue("save-dir", save_dir);
    qs.endGroup();
}

bool Settings::retrieve(QSettings &qs) {
    if (!qs.contains("Keys/edit-key")) return false;

    qs.beginGroup("Keys");

    edit_key = Settings::keycomb_from_string(qs.value("edit-key").toString());
    pen_key = Settings::keycomb_from_string(qs.value("pen-key").toString());
    line_key = Settings::keycomb_from_string(qs.value("line-key").toString());
    rect_key = Settings::keycomb_from_string(qs.value("rect-key").toString());
    smooth_key =
        Settings::keycomb_from_string(qs.value("smooth-key").toString());
    ortho_key = Settings::keycomb_from_string(qs.value("ortho-key").toString());
    highlight_key =
        Settings::keycomb_from_string(qs.value("highlight-key").toString());
    fader_key = Settings::keycomb_from_string(qs.value("fader-key").toString());
    text_key = Settings::keycomb_from_string(qs.value("text-key").toString());

    del_key = Settings::keycomb_from_string(qs.value("delete-key").toString());
    del_all_key =
        Settings::keycomb_from_string(qs.value("delete-all-key").toString());
    dash_key = Settings::keycomb_from_string(qs.value("dash-key").toString());
    grab_key = Settings::keycomb_from_string(qs.value("grab-key").toString());
    grab_partial_key =
        Settings::keycomb_from_string(qs.value("grab-partial-key").toString());

    quit_key = Settings::keycomb_from_string(qs.value("quit-key").toString());
    undo_key = Settings::keycomb_from_string(qs.value("undo-key").toString());
    redo_key = Settings::keycomb_from_string(qs.value("redo-key").toString());
    toggle_key =
        Settings::keycomb_from_string(qs.value("toggle-key").toString());
    opacity_key =
        Settings::keycomb_from_string(qs.value("opacity-key").toString());

    qs.endGroup();

    // ---------------------------- tools ----------------------------
    qs.beginGroup("Tools");

    qs.beginGroup("Rect");
    radius_rect = qs.value("radius").toDouble();
    qs.endGroup();

    qs.beginGroup("Smooth");
    simplify_smooth = qs.value("simplify").toDouble();
    snapdist_smooth = qs.value("snapdist").toDouble();
    qs.endGroup();

    qs.beginGroup("Ortho");
    simplify_ortho = qs.value("simplify").toDouble();
    snapdist_ortho = qs.value("snapdist").toDouble();
    radius_ortho = qs.value("radius").toDouble();
    minlen_ortho = qs.value("minlen").toDouble();
    maxangle_ortho = qs.value("maxangle").toDouble();
    qs.endGroup();

    qs.beginGroup("Highlight");
    sustain_highlight = qs.value("sustain").toDouble();
    fade_highlight = qs.value("fade").toDouble();
    qs.endGroup();

    qs.beginGroup("Fader");
    sustain_fader = qs.value("sustain").toDouble();
    fade_fader = qs.value("fade").toDouble();
    qs.endGroup();

    qs.beginGroup("Text");
    font.fromString(qs.value("font").toString());
    qs.endGroup();
    qs.endGroup();

    qs.beginGroup("Colors");
    for (int i = 0; i < static_cast<int>(color_table.size()); i++) {
        Color &col = color_table[i];
        qs.beginGroup(QString("%1").arg(i, 2, 10, QChar('0')));
        col.color = Color::qcolor_from_hex(qs.value("color").toString());
        col.key1 = Settings::keycomb_from_string(qs.value("key1").toString());
        col.key2 = Settings::keycomb_from_string(qs.value("key2").toString());
        qs.endGroup();
    }
    qs.endGroup();

    qs.beginGroup("Linewidths");
    for (int i = 0; i < 9; i++) {
        qs.beginGroup(QString("%1").arg(i, 2, 10, QChar('0')));
        line_widths[i] = qs.value("width").toDouble();
        highlight_line_width[i] = qs.value("highligher-width").toDouble();
        width_key[i] =
            Settings::keycomb_from_string(qs.value("key").toString());
        qs.endGroup();
    }
    qs.endGroup();

    qs.beginGroup("Arrows");
    arrow_start_key = Settings::keycomb_from_string(
        qs.value("arrow-beginning").toString());
    arrow_end_key =
        Settings::keycomb_from_string(qs.value("arrow-end").toString());
    arrow_double_key = Settings::keycomb_from_string(
        qs.value("arrow-double").toString());
    arrow_none_key = Settings::keycomb_from_string(
        qs.value("arrow-none").toString());
    arrow_size = qs.value("size").toDouble();
    qs.endGroup();

    qs.beginGroup("Directories");
    save_dir = qs.value("save-dir").toString();
    qs.endGroup();
    return true;
}

QColor Settings::get_color(const QKeyCombination &k) {
    for (const auto &c : color_table) {
        if (c.key1 == k || c.key2 == k) return c.color;
    }
    return QColor();
}

int Settings::qcolor_find_index(const QColor& color) const {
    const auto it = std::find_if(
        color_table.cbegin(),
        color_table.cend(),
        [&color](const Color& item) { return item.color == color; });
    if (it == color_table.cend()) return -1;
    return std::distance(color_table.cbegin(), it);
}
