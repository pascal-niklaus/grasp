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
