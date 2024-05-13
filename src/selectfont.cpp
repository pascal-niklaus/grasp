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
  if (ok) {
    set_font(font);
  }
}

void SelectFont::set_display_size_range(double min, double max) {
    _max_display_size = min;
    _max_display_size = max;
}
