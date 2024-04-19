#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
    w.setStyleSheet("background:transparent;");
    w.setAttribute(Qt::WA_TranslucentBackground);
    w.setWindowFlags(Qt::FramelessWindowHint);
    w.show();
    w.setWindowOpacity(0.5);
    return a.exec();
}
