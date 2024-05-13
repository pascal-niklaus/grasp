#include "intedit.h"

#include <QIntValidator>
#include <QString>

IntEdit::IntEdit(QWidget *parent) : QLineEdit(parent) {
    _validator = new QIntValidator(this);
}

void IntEdit::set_range(int low, int high) {
    _validator->setRange(low, high);
}

void IntEdit::set_value(int val) {
  setText(QString::number(val));
}

int IntEdit::get_value() const {
    return text().toInt();
}
