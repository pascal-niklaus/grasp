#include <iostream>

#include <QCryptographicHash>

#include "mainwindow.h"
#include "singletonapplication.h"

int main(int argc, char *argv[]) {
    QString key = QCryptographicHash::hash(argv[0],QCryptographicHash::Md5).toHex();
    SingletonApplication a(argc, argv, key);

    if (a.already_running()) {
      std::cerr << "Already running !\n";
      return 1;
    }

    MainWindow w;
    w.setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
    w.setStyleSheet("background:transparent;");
    w.setAttribute(Qt::WA_TranslucentBackground);
    w.setWindowFlags(Qt::FramelessWindowHint);
    w.show();
    w.setWindowOpacity(0.5);

    return a.exec();
}
