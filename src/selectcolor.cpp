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
