#include "mainwindow.h"
#include "singletonapplication.h"
#include "dbus.h"

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCommandLineParser>


#include "kromittext.h"
#include <iostream>

bool send_dbus_command(QDBusConnection& bus,
                       const QString& cmd,
                       const QString& arg1 = QString(),
                       const QString& arg2 = QString()) {
    QDBusMessage message = QDBusMessage::createMethodCall(
        Message::service_name(), "/config", "", cmd);

    if (! arg1.isEmpty())
      message << arg1;
    if (! arg2.isEmpty())
      message << arg2;

    // Add arguments to the message if needed
    //    message << QVariant::fromValue("Argument1") <<
    //    QVariant::fromValue(42);

    // Send the message
    return bus.send(message);
}

#include "path.h"
#include <regex>

void test() {
    std::string dbtxt =
        "(1002.0,477.0) (1324.0,476.0) (1489.0,550.0) (1512.0,568.0) "
        "(1522.0,728.0)";
    const std::regex xyrx(R"( *\(([0-9\.]+),([0-9\.]+)\))");
    std::sregex_iterator it(dbtxt.begin(), dbtxt.end(), xyrx);
    std::sregex_iterator end;

    Path path;
    while (it != end) {
        std::smatch xyrx_match = *it;
        const double x = std::stof(xyrx_match[1]);
        const double y = std::stof(xyrx_match[2]);
        path.append(x,y);
        ++it;
    }

    Path opath = path;
    opath.orthogonalize(20.0, 70.0);
    opath.round_corners(30, 6, false);

    std::cerr << path;
    std::cerr << opath;

    // plot everything
    QPixmap pixmap(2000, 1000);
    pixmap.fill(Qt::black);
    QPainter painter(&pixmap);

    QPen pen(Qt::yellow);
    pen.setWidth(1);
    painter.setPen(pen);

    QPainterPath pp;
    pp.moveTo(path.first());
    for (int i=1; i<path.size(); i++)
      pp.lineTo(path[i]);
    painter.drawPath(pp);

    pp.clear();
    pen.setColor(Qt::cyan);
    painter.setPen(pen);
    pp.moveTo(opath.first());
    for (int i=1; i<opath.size(); i++)
      pp.lineTo(opath[i]);
    painter.drawPath(pp);

    pixmap.save("debug.png", "PNG");
}

int main(int argc, char *argv[]) {

    QString key = QCryptographicHash::hash(argv[0],QCryptographicHash::Md5).toHex();
    SingletonApplication a(argc, argv, key);

  //     test();
  // return 0;

    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);

    // setup command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Kromit -- a screen annotation tool\n(C) 2024 by Pascal Niklaus");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    QCommandLineOption snapshotOption(
        "s", "take snapshot of screen");
    parser.addOption(snapshotOption);

    QCommandLineOption colorOption(QStringList{"color", "c"}, "set color of tool", "COLOR");
    parser.addOption(colorOption);

    QCommandLineOption toolOption(QStringList{"tool", "t"}, "set tool type", "TOOL");
    parser.addOption(toolOption);

    QCommandLineOption quitOption(QStringList{"quit", "q"}, "quit application");
    parser.addOption(quitOption);

    QCommandLineOption toggleOption("toggle", "toggle application");
    parser.addOption(toggleOption);

    QCommandLineOption screenshotOption(QStringList{"grab", "g"}, "take screenshot", "GEOMETRY", "all");
    parser.addOption(screenshotOption);

    //    parser.addOption(const QCommandLineOption &commandLineOption)
    parser.process(a);

    if (a.already_running()) {
        std::cerr << "Already running !\n";

        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            qDebug() << "Failed to connect to the session bus";
            return 1;
        }

        QString cmd;
        if (parser.isSet(quitOption)) {
          send_dbus_command(bus, "quit");
        }

        if (parser.isSet(colorOption)) {
          QString color = parser.value(colorOption);
          qDebug() << "color=" << color;
          send_dbus_command(bus, "color", color);
        }

        if (parser.isSet(screenshotOption)) {
          QString geom = parser.value(screenshotOption);
          qDebug() << "grab";
          send_dbus_command(bus, "screenshot", "");
        }
        return 0;
    } else {
        Message msg(&a);
        MainWindow w;

        QObject::connect(&msg, &Message::quit_received, &w, &MainWindow::close);
        QObject::connect(&msg, &Message::color_received, &w, &MainWindow::change_color);
        QObject::connect(&msg, &Message::toggle_received, &w, &MainWindow::toggle);
        QObject::connect(&msg, &Message::screenshot_received, &w, &MainWindow::take_screenshot);

        w.setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
        w.setStyleSheet("background:transparent;");
        // w.setAttribute(Qt::WA_TranslucentBackground);
        w.setWindowFlags(Qt::FramelessWindowHint); // TODO: | Qt::WindowStaysOnTopHint);
        w.show();
        w.setWindowOpacity(0.5);
        w.fix_geometry();
        //  w.setWindowOpacity(1);

        return a.exec();
    }
}
