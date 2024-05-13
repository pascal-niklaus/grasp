#ifndef INTEDIT_H
#define INTEDIT_H

#include <QLineEdit>

class QIntValidator;

class IntEdit : public QLineEdit {
    Q_OBJECT
  public:
    IntEdit(QWidget *parent = nullptr);
    int get_value() const;
    void set_value(int val);
    void set_range(int low, int high);

  private:
    QIntValidator *_validator;
};

#endif
