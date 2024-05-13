
#ifndef DOUBLEEDIT_H
#define DOUBLEEDIT_H

#include <QLineEdit>

class QDoubleValidator;

class DoubleEdit : public QLineEdit {
    Q_OBJECT
  public:
    DoubleEdit(QWidget *parent = nullptr);
    double get_value() const;
    void set_value(double val);
    void set_range(double low, double high, int decimals = 1);

  private:
    QDoubleValidator *_validator;
};

#endif
