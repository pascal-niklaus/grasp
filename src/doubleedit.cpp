#include "doubleedit.h"

#include <QDoubleValidator>

DoubleEdit::DoubleEdit(QWidget *parent) : QLineEdit(parent) {
    _validator = new QDoubleValidator(this);
}

void DoubleEdit::set_range(double low, double high, int decimals) {
    _validator->setRange(low, high, decimals);
}

void DoubleEdit::set_value(double val) {
    setText(QString::number(val, 'f', _validator->decimals()));
}

double DoubleEdit::get_value() const {
    return text().toDouble();
}
