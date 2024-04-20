#ifndef _SINGLETONAPPLICATION_H
#define _SINGLETONAPPLICATION_H

#include <QApplication>
#include <QSharedMemory>

class SingletonApplication : public QApplication {
    Q_OBJECT

  public:
    SingletonApplication(int &argc, char **argv, const QString &mem_id) :
        QApplication(argc, argv, true) {
        _singleton = new QSharedMemory(mem_id, this);
    }

    ~SingletonApplication() {
        if (_singleton->isAttached()) _singleton->detach();
    };

    bool already_running() {
        if (_singleton->attach(QSharedMemory::ReadOnly)) {
            _singleton->detach();
        } else {
            if (_singleton->create(1)) return false;
        }
        return true;
    }

  private:
    QSharedMemory *_singleton = nullptr;
};

#endif
