#include "settings.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyCombination>
#include <QKeyEvent>
#include <QLabel>
#include <QSettings>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFileDialog>

#include <iostream>

#include "utility.h"
#include "colors.h"
#include "doubleedit.h"
#include "intedit.h"
#include "selectcolor.h"
#include "selectfont.h"
#include "selectkey.h"
#include "tools.h"

// -------------------- settings dialog and tabs ---------------------

// ========================== LinewidthTab ===========================

LinewidthTab::LinewidthTab(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f) {
    // arrow heads: < - > =
    // line widths: keys 1-9
    // amplification factor for highlight tool
    auto *lay = new QGridLayout;
    int row = 0;

    lay->addWidget(new QLabel("Shortcut"), row, 0);
    lay->addWidget(new QLabel("Line width"), row, 1);
    lay->addWidget(new QLabel("Highlighter width"), row, 2);

    lay->addWidget(new QLabel("Shortcut"), row, 4);
    lay->addWidget(new QLabel("Arrow head"), row, 5);

    row++;

    for (int w = 0; w < 9; w++) {
        _width_key[w] = new SelectKey();
        _width_key[w]->set_allowed_modifiers(Qt::AltModifier | Qt::MetaModifier |
                                      Qt::ShiftModifier | Qt::ControlModifier);
        lay->addWidget(_width_key[w], row + w, 0);

        _width[w] = new IntEdit(nullptr);
        _width[w]->set_range(0, 30);
        lay->addWidget(_width[w], row + w, 1);

        _highlighter_width[w] = new IntEdit(nullptr);
        _highlighter_width[w]->set_range(10, 130);
        lay->addWidget(_highlighter_width[w], row + w, 2);
    }

    QFont fnt = font();
    //    fnt.setPointSize(fnt.pointSize() * 2);
    //    fnt.setBold(true);

    for (int w = 0; w < 4; w++) {
        _arrow_key[w] = new SelectKey();
        _arrow_key[w]->set_allowed_modifiers(Qt::AltModifier | Qt::MetaModifier |
                                      Qt::ShiftModifier | Qt::ControlModifier);
        lay->addWidget(_arrow_key[w], row + w, 4);

        std::string arrow;
        switch (w) {
            case 0:
                arrow = "head at start: \u2190";
                break;
            case 1:
                arrow = "head at both ends: \u2194";
                break;
            case 2:
                arrow = "head at end: \u2192";
                break;
            case 3:
                arrow = "no heads: \u2012";
                break;
        }
        auto *alabel = new QLabel(QString::fromStdString(arrow));
        alabel->setFont(fnt);
        lay->addWidget(alabel, row + w, 5);
    }

    _arrowsize = new DoubleEdit;
    _arrowsize->set_range(1.0, 6.0);
    lay->addWidget(_arrowsize, row + 6, 5);
    lay->addWidget(new QLabel("Arrow size"), row + 6, 4);

    auto *help = new QLabel();
    help->setWordWrap(true);
    help->setText(
        tr("Note: As shortcut keys for arrow widths, I recommend using the "
           "numeric keys 1 to 9. For the highlighter tool, a larger width is "
           "needed. Arrow size specifies the size of the arrow head relative "
           "to the used line width."));
    lay->addWidget(help, 13, 0, 1, 6);

    setLayout(lay);
    lay->setColumnMinimumWidth(3, 20);
    lay->setRowStretch(14, 10);
    lay->setColumnStretch(6, 10);
}

void LinewidthTab::load_options(const Settings &settings) {
    for (int w = 0; w < 9; w++) {
        _width[w]->set_value(settings.line_widths[w]);
        _highlighter_width[w]->set_value(settings.highlight_line_width[w]);
        _width_key[w]->set_key(settings.width_key[w]);
    }

    _arrow_key[0]->set_key(settings.arrow_start_key);
    _arrow_key[1]->set_key(settings.arrow_double_key);
    _arrow_key[2]->set_key(settings.arrow_end_key);
    _arrow_key[3]->set_key(settings.arrow_none_key);

    _arrowsize->set_value(settings.arrow_size);
}

void LinewidthTab::save_options(Settings &settings) const {
    for (int w = 0; w < 9; w++) {
        settings.line_widths[w] = _width[w]->get_value();
        settings.highlight_line_width[w] = _highlighter_width[w]->get_value();
        settings.width_key[w] = _width_key[w]->get_key();
    }

    settings.arrow_start_key = _arrow_key[0]->get_key();
    settings.arrow_double_key = _arrow_key[1]->get_key();
    settings.arrow_end_key = _arrow_key[2]->get_key();
    settings.arrow_none_key = _arrow_key[3]->get_key();
    settings.arrow_size = _arrowsize->get_value();
}

// ----------------------------- AuxTab ------------------------------

AuxTab::AuxTab(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f) {
    auto *lay = new QGridLayout;
    int row = 0;

    lay->addWidget(new QLabel(tr("Default save directory")), row, 0);
    _path_label = new QLineEdit();
    _path_label->setReadOnly(true);
    _path_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    lay->addWidget(_path_label, row, 1);
    _browse = new QPushButton(tr("Browse"));
    lay->addWidget(_browse, row, 2);

    connect(_browse, &QPushButton::clicked, this, &AuxTab::slot_browse_clicked);

    lay->setColumnMinimumWidth(1, 120);
    lay->setRowStretch(row+1, 10);
    lay->setColumnStretch(3, 10);

    setLayout(lay);
}

void AuxTab::slot_browse_clicked() {
    auto dir = QFileDialog::getExistingDirectory(
        this,
        tr("Chose directory"),
        _path,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
          set_path(dir);

    }
}

void AuxTab::set_path(const QString &path) {
    _path_label->setText(path);
    qDebug() << _path_label->sizeHint();

    QFontMetrics fm(_path_label->font());
    int width = cap<int>(fm.horizontalAdvance(path), 100, rect().width()/2);
    _path_label->setMinimumWidth(width+20);
    _path = path;
}

void AuxTab::load_options(const Settings &settings) {
  set_path(settings.save_dir);
}

void AuxTab::save_options(Settings &settings) const {
    settings.save_dir = _path;
}

// ============================= ToolTab =============================

enum ParamCols {
    COL_FONT = 2,
    COL_SNAPDIST,
    COL_SIMPLIFY,
    COL_RADIUS,
    COL_MINLEN,
    COL_MAXANGLE,
    COL_SUSTAIN,
    COL_FADE,
};

ToolTab::ToolTab(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f) {
    auto *lay = new QGridLayout;
    int row = 0;

    lay->addWidget(new QLabel("Tool"), row, 0);
    lay->addWidget(new QLabel("Shortcut"), row, 1);
    lay->addWidget(new QLabel("Tool Options"), row, 2, 1, -1);
    row++;

    lay->addWidget(new QLabel("Font"), row, COL_FONT);
    lay->addWidget(new QLabel("Radius"), row, COL_RADIUS);
    lay->addWidget(new QLabel("Snap"), row, COL_SNAPDIST);
    lay->addWidget(new QLabel("Simplify"), row, COL_SIMPLIFY);
    lay->addWidget(new QLabel("Max. angle"), row, COL_MAXANGLE);
    lay->addWidget(new QLabel("Min. length"), row, COL_MINLEN);
    lay->addWidget(new QLabel("Sustain"), row, COL_SUSTAIN);
    lay->addWidget(new QLabel("Fade"), row, COL_FADE);
    row++;

    // edit
    lay->addWidget(new QLabel("Select tool"), row, 0);
    _edit_key = new SelectKey();
    _edit_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_edit_key, row, 1);
    row++;

    // pen
    lay->addWidget(new QLabel("Pen tool"), row, 0);
    _pen_key = new SelectKey();
    _pen_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_pen_key, row, 1);
    row++;

    // line
    lay->addWidget(new QLabel("Line tool"), row, 0);
    _line_key = new SelectKey();
    _line_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_line_key, row, 1);
    row++;

    // rect
    lay->addWidget(new QLabel("Rect tool"), row, 0);
    _rect_key = new SelectKey();
    _rect_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_rect_key, row, 1);

    _rect_radius = new DoubleEdit(nullptr);
    _rect_radius->set_range(0, 30, 1);
    lay->addWidget(_rect_radius, row, COL_RADIUS);

    row++;

    // smooth
    lay->addWidget(new QLabel("Smooth tool"), row, 0);
    _smooth_key = new SelectKey();
    _smooth_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_smooth_key, row, 1);

    _smooth_simplify = new DoubleEdit(nullptr);
    _smooth_simplify->set_range(0.1, 50, 1);
    lay->addWidget(_smooth_simplify, row, COL_SIMPLIFY);

    _smooth_snap = new DoubleEdit(nullptr);
    _smooth_snap->set_range(2.0, 50.0, 1);
    lay->addWidget(_smooth_snap, row, COL_SNAPDIST);

    row++;

    // ortho
    lay->addWidget(new QLabel("Orthogonal tool"), row, 0);
    _ortho_key = new SelectKey();
    _ortho_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_ortho_key, row, 1);

    _ortho_simplify = new DoubleEdit(nullptr);
    _ortho_simplify->set_range(0.1, 50, 1);
    lay->addWidget(_ortho_simplify, row, COL_SIMPLIFY);

    _ortho_snap = new DoubleEdit(nullptr);
    _ortho_snap->set_range(2.0, 50.0, 1);
    lay->addWidget(_ortho_snap, row, COL_SNAPDIST);

    _ortho_maxangle = new DoubleEdit(nullptr);
    _ortho_maxangle->set_range(0.1, 50, 1);
    lay->addWidget(_ortho_maxangle, row, COL_MAXANGLE);

    _ortho_minlen = new DoubleEdit(nullptr);
    _ortho_minlen->set_range(2.0, 50.0, 1);
    lay->addWidget(_ortho_minlen, row, COL_MINLEN);

    _ortho_radius = new DoubleEdit(nullptr);
    _ortho_radius->set_range(2.0, 50.0, 1);
    lay->addWidget(_ortho_radius, row, COL_RADIUS);

    row++;

    // highlight
    lay->addWidget(new QLabel("Highlight tool"), row, 0);
    _highlight_key = new SelectKey();
    _highlight_key->set_allowed_modifiers(Qt::ShiftModifier |
                                          Qt::ControlModifier);
    lay->addWidget(_highlight_key, row, 1);

    _highlight_sustain = new DoubleEdit(nullptr);
    _highlight_sustain->set_range(0.1, 50, 1);
    lay->addWidget(_highlight_sustain, row, COL_SUSTAIN);

    _highlight_fade = new DoubleEdit(nullptr);
    _highlight_fade->set_range(2.0, 50.0, 1);
    lay->addWidget(_highlight_fade, row, COL_FADE);

    row++;

    // fader
    lay->addWidget(new QLabel("Fader tool"), row, 0);
    _fader_key = new SelectKey();
    _fader_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_fader_key, row, 1);

    _fader_sustain = new DoubleEdit(nullptr);
    _fader_sustain->set_range(0.1, 50, 1);
    lay->addWidget(_fader_sustain, row, COL_SUSTAIN);

    _fader_fade = new DoubleEdit(nullptr);
    _fader_fade->set_range(2.0, 50.0, 1);
    lay->addWidget(_fader_fade, row, COL_FADE);

    row++;

    // text
    lay->addWidget(new QLabel("Text tool"), row, 0);
    _text_key = new SelectKey();
    _text_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_text_key, row, 1);

    _font = new SelectFont();
    const auto default_size = _font->font().pointSizeF();
    _font->set_display_size_range(0.7 * default_size, 2 * default_size);
    lay->addWidget(_font, row, COL_FONT);
    row++;

    // grab
    lay->addWidget(new QLabel("Grab screen section"), row, 0);
    _grab_partial_key = new SelectKey();
    _grab_partial_key->set_allowed_modifiers(Qt::ShiftModifier |
                                             Qt::ControlModifier);
    lay->addWidget(_grab_partial_key, row, 1);
    row++;

    // grab screen
    lay->addWidget(new QLabel("Grab entire screen"), row, 0);
    _grab_key = new SelectKey();
    _grab_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_grab_key, row, 1);
    row++;

    // delete
    lay->addWidget(new QLabel("Delete selection"), row, 0);
    _delete_key = new SelectKey();
    _delete_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_delete_key, row, 1);
    row++;

    // delete
    lay->addWidget(new QLabel("Delete scene"), row, 0);
    _delete_all_key = new SelectKey();
    _delete_all_key->set_allowed_modifiers(Qt::ShiftModifier | Qt::ControlModifier);
    lay->addWidget(_delete_all_key, row, 1);
    row++;

    // toggle dash
    lay->addWidget(new QLabel("Toggle dash"), row, 0);
    _dash_toggle_key = new SelectKey();
    _dash_toggle_key->set_allowed_modifiers(Qt::ShiftModifier |
                                            Qt::ControlModifier);
    lay->addWidget(_dash_toggle_key, row, 1);
    row++;

    _status = new QStatusBar();
    const auto px = _status->font().pointSizeF();
    _status->setSizeGripEnabled(false);
    _status->setStyleSheet(
        QString(
            "padding-left:0px; color: red; font-weight:bold; font-size: %1pt;")
            .arg(static_cast<int>(1.5 * px)));

    lay->addWidget(_status, row++, 0, 1, -1);

    auto *help = new QLabel();
    help->setWordWrap(true);
    help->setTextFormat(Qt::MarkdownText);
    help->setText(tr(
        R"END(<table cellspacing=5>
<thead>
  <tr>
    <th align='left'><B>Parameter</B></th>
    <th align='left'><B>Description</B></th>
  </tr>
</thead>
<tbody>
<tr><td>radius</td>
    <td>radius of arcs connecting straight sections</td></tr>
<tr><td>simplify</td>
    <td>during line simplification, the maximum distance of points from straight line approximation</td></tr>
<tr><td>snap</td>
    <td>distance between ends below which ends are joined</td></tr>
<tr><td>max. angle</td>
    <td>maximum angular deviation in degrees within which a path section is still recognized as orthogonal</td></tr>
<tr><td>minlen</td>
    <td>minimum length of orthogonal section; must exceed 2x radius</td></tr>
<tr><td>sustain</td>
    <td>time in seconds before an element starts fading away</td></tr>
<tr><td>fade</td>
    <td>time in seconds an element takes to fade away</td></tr>
</tbody>
</table>
)END"));
    lay->addWidget(help, row++, 0, 1, 12);

    lay->setRowStretch(row, 10);
    lay->setColumnStretch(12, 10);
    // tools -> shortcuts, some defaults (e.g. timings, magnifiers, font)
    setLayout(lay);
}

void ToolTab::load_options(const Settings &settings) {
    _rect_radius->set_value(settings.radius_rect);

    _smooth_simplify->set_value(settings.simplify_smooth);
    _smooth_snap->set_value(settings.snapdist_smooth);

    _ortho_simplify->set_value(settings.simplify_ortho);
    _ortho_snap->set_value(settings.snapdist_ortho);
    _ortho_maxangle->set_value(settings.maxangle_ortho);
    _ortho_minlen->set_value(settings.minlen_ortho);
    _ortho_radius->set_value(settings.radius_ortho);

    _fader_sustain->set_value(settings.sustain_fader);
    _fader_fade->set_value(settings.fade_fader);

    _highlight_sustain->set_value(settings.sustain_highlight);
    _highlight_fade->set_value(settings.fade_highlight);

    _font->set_font(settings.font);

    _edit_key->set_key(settings.edit_key);

    _pen_key->set_key(settings.pen_key);
    _smooth_key->set_key(settings.smooth_key);
    _ortho_key->set_key(settings.ortho_key);
    _line_key->set_key(settings.line_key);
    _rect_key->set_key(settings.rect_key);
    _text_key->set_key(settings.text_key);
    _highlight_key->set_key(settings.highlight_key);
    _fader_key->set_key(settings.fader_key);

    _delete_key->set_key(settings.del_key);
    _delete_all_key->set_key(settings.del_all_key);
    _dash_toggle_key->set_key(settings.dash_key);
    _grab_key->set_key(settings.grab_key);
    _grab_partial_key->set_key(settings.grab_partial_key);
}


void ToolTab::save_options(Settings &settings) const {
    settings.radius_rect = _rect_radius->text().toDouble();

    settings.simplify_smooth = _smooth_simplify->text().toDouble();
    settings.snapdist_smooth = _smooth_snap->text().toDouble();

    settings.simplify_ortho = _ortho_simplify->text().toDouble();
    settings.snapdist_ortho = _ortho_snap->text().toDouble();
    settings.maxangle_ortho = _ortho_maxangle->text().toDouble();
    settings.minlen_ortho = _ortho_minlen->text().toDouble();
    settings.radius_ortho = _ortho_radius->text().toDouble();

    settings.sustain_fader = _fader_sustain->text().toDouble();
    settings.fade_fader = _fader_fade->text().toDouble();

    settings.sustain_highlight = _highlight_sustain->text().toDouble();
    settings.fade_highlight = _highlight_fade->text().toDouble();

    settings.font = _font->get_font();

    settings.edit_key = _edit_key->get_key();
    settings.pen_key = _pen_key->get_key();
    settings.smooth_key = _smooth_key->get_key();
    settings.ortho_key = _ortho_key->get_key();
    settings.line_key = _line_key->get_key();
    settings.rect_key = _rect_key->get_key();
    settings.text_key = _text_key->get_key();
    settings.highlight_key = _highlight_key->get_key();
    settings.fader_key = _fader_key->get_key();

    settings.del_key = _delete_key->get_key();
    settings.del_all_key = _delete_all_key->get_key();
    settings.dash_key = _dash_toggle_key->get_key();
    settings.grab_key = _grab_key->get_key();
    settings.grab_partial_key = _grab_partial_key->get_key();
}

void ToolTab::slot_status_message(const QString &msg) {
    _status->showMessage(msg, 4000);
}

// ============================ ColorTab =============================

ColorTab::ColorTab(int ncolors, QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f) {
    _ncolors = ncolors;

    auto *lay = new QGridLayout;
    constexpr int nrow = 4;
    constexpr int colwidth = 6;
    for (int i = 0; i < ncolors; i++) {
        const auto row = i % nrow;
        const auto col = i / nrow;
        if (row == 0) {
            // lay->addWidget(new QLabel("#"), 0, colwidth * col);
            lay->addWidget(new QLabel("Color"), 0, colwidth * col + 1);
            lay->addWidget(new QLabel("Key 1"), 0, colwidth * col + 2);
            lay->addWidget(new QLabel("Key 2"), 0, colwidth * col + 3);
        }

        lay->addWidget(
            new QLabel(QString("#%1").arg(i + 1)), row + 1, colwidth * col);

        auto *btn = new SelectColorButton(nullptr);
        //        colbtn->set_color(Color::qcolor_from_uint32(color.numeric));
        lay->addWidget(btn, row + 1, colwidth * col + 1);
        _colbtn.emplace_back(btn);

        auto *ks1 = new SelectKey();
        ks1->set_allowed_modifiers(Qt::ShiftModifier);
        ks1->set_del_key(Qt::Key_Delete);
        _keyseq1.emplace_back(ks1);

        auto *ks2 = new SelectKey();
        ks2->set_allowed_modifiers(Qt::ShiftModifier);
        ks2->set_del_key(Qt::Key_Delete);
        _keyseq2.emplace_back(ks2);

        // if (color.key1 > 0)
        //     keyseq1->setKeySequence(QKeySequence(
        //         QKeyCombination(static_cast<Qt::KeyboardModifiers>(color.mod1),
        //                         static_cast<Qt::Key>(color.key1))));
        // if (color.key2 > 0)
        //     keyseq2->setKeySequence(QKeySequence(
        //         QKeyCombination(static_cast<Qt::KeyboardModifiers>(color.mod2),
        //                         static_cast<Qt::Key>(color.key2))));

        connect(ks1,
                &SelectKey::status_message,
                this,
                &ColorTab::slot_status_message);
        connect(ks2,
                &SelectKey::status_message,
                this,
                &ColorTab::slot_status_message);

        lay->addWidget(ks1, row + 1, colwidth * col + 2);
        lay->addWidget(ks2, row + 1, colwidth * col + 3);
        lay->setColumnMinimumWidth(col * colwidth + 5, 15);
    }
    auto *help = new QLabel();
    help->setWordWrap(true);
    help->setText(tr(
        "Note: Two different, alternative, key strokes may be "
        "specified. However, only the Shift key is allowed as "
        "modifier. All other modifiers will be ignored. Also, take care that "
        "these keystrokes do not conflict with each other and the tool "
        "seleciton keystrokes. To delete a key stroke, hit the DEL key"));
    lay->addWidget(help, nrow + 2, 0, 1, colwidth * 3 - 2);

    _status = new QStatusBar();
    const auto px = _status->font().pointSizeF();
    _status->setSizeGripEnabled(false);
    _status->setStyleSheet(
        QString(
            "padding-left:0px; color: red; font-weight:bold; font-size: %1pt;")
            .arg(static_cast<int>(1.5 * px)));

    lay->addWidget(_status, nrow + 3, 0, 1, colwidth * 3);

    lay->setRowStretch(nrow + 4, 10);
    lay->setColumnStretch(16, 10);

    setLayout(lay);
}

void ColorTab::slot_status_message(const QString &msg) {
    _status->showMessage(msg, 4000);
}

void ColorTab::load_options(const Settings &settings) {
    for (int i = 0; i < _ncolors; i++) {
      _colbtn[i]->set_color(settings.color_table[i].color);
      _keyseq1[i]->set_key(settings.color_table[i].key1);
      _keyseq2[i]->set_key(settings.color_table[i].key2);
    }
}

void ColorTab::save_options(Settings &settings) const {
    for (int i = 0; i < _ncolors; i++) {
        settings.color_table[i].color = _colbtn[i]->get_color();
        settings.color_table[i].key1 = _keyseq1[i]->get_key();
        settings.color_table[i].key2 = _keyseq2[i]->get_key();
    }
}

// ========================= SettingsDialog ==========================

SettingsDialog::SettingsDialog(Settings &settings,
                               QWidget *parent,
                               Qt::WindowFlags f) :
    QDialog(parent, f), _settings(settings) {
    //
    auto *vblay = new QVBoxLayout();
    setLayout(vblay);

    auto *tabs = new QTabWidget();
    vblay->addWidget(tabs);

    _tooltab = new ToolTab;
    tabs->addTab(_tooltab, tr("&Tools && Shortcuts"));

    _colortab = new ColorTab(_settings.color_table.size());
    tabs->addTab(_colortab, tr("&Colors"));

    _linewidthtab = new LinewidthTab;
    tabs->addTab(_linewidthtab, tr("Line &widths && Arrows"));

    _auxtab = new AuxTab;
    tabs->addTab(_auxtab, tr("&Auxiliary settings"));

    auto bbox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bbox->button(QDialogButtonBox::Ok)->setDefault(false);

    vblay->addWidget(bbox);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connect(bbox, &QDialogButtonBox::accepted, this, &SettingsDialog::slot_accept);
    connect(bbox, &QDialogButtonBox::rejected, this, &SettingsDialog::slot_reject);

    retrieve_settings();
    _auxtab->load_options(_settings);
    _colortab->load_options(_settings);
    _tooltab->load_options(_settings);
    _linewidthtab->load_options(_settings);
}

void SettingsDialog::keyPressEvent(QKeyEvent *event) {
    int key = event->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter)
        event->ignore();
    else
        QDialog::keyPressEvent(event);
}

void SettingsDialog::slot_accept() {
    _tooltab->save_options(_settings);
    _linewidthtab->save_options(_settings);
    _colortab->save_options(_settings);
    _auxtab->save_options(_settings);
    store_settings();
    accept();
}

void SettingsDialog::slot_reject() {
    reject();
}

void SettingsDialog::store_settings() {
}

void SettingsDialog::retrieve_settings() {
}
