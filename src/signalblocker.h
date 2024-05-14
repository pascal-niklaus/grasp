#ifndef SIGNALBLOCKER_H
#define SIGNALBLOCKER_H

#include <QObject>

class SignalBlocker {
  public:
    SignalBlocker(QObject *obj) : _obj(obj), _old(obj->blockSignals(true)) {}
    ~SignalBlocker() { _obj->blockSignals(_old); }

  private:
    QObject *_obj;
    bool _old;
};

#endif // SIGNALBLOCKER_H
