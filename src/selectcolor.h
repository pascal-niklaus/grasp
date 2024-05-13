#ifndef SELECTCOLOR_H
#define SELECTCOLOR_H

#include <QPushButton>

class SelectColorButton : public QPushButton {
    Q_OBJECT
public:
    SelectColorButton( QWidget* parent );

    void set_color( const QColor& color );
    const QColor& get_color() const { return _color; }

public slots:
    void update_color();
    void change_color();

private:
    QColor _color;
};

#endif
