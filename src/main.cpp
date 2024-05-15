#include "mainwindow.h"
#include "singletonapplication.h"
#include "dbus.h"

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCommandLineParser>
#include <QSettings>

#include "settings.h"

#include <iostream>
#include <memory>

bool send_dbus_command(QDBusConnection& bus,
                       const QString& cmd,
                       const QString& arg1 = QString(),
                       const QString& arg2 = QString()) {
    QDBusMessage message = QDBusMessage::createMethodCall(
        Message::service_name(), Message::service_path(), "", cmd);

    if (!arg1.isEmpty()) message << arg1;
    if (!arg2.isEmpty()) message << arg2;

    return bus.send(message);
}

// ----------------- help text regarding dbus usage ------------------

void print_dbus_help() {
    std::cerr
        << "'" << APP_NAME << "' "
        << "makes additional commands available via DBus, under service name\n"
        << "'" << Message::service_name().toStdString() << "' and service path "
        << "'" << Message::service_path().toStdString() << "'.\n";

    std::cerr << R"END(
One of the easier options to list the availbale DBus commands and invoke these
is to use 'qdbus'. The available commands can be listed like so:

    > qdbus org.pascalniklaus.grasp /exec
    method void org.pascalniklaus.grasp.arrow(QString arrow)
    method void org.pascalniklaus.grasp.color(QString color)
    method void org.pascalniklaus.grasp.grab()
    method void org.pascalniklaus.grasp.grabnew()
    method void org.pascalniklaus.grasp.hide()
    method void org.pascalniklaus.grasp.quit()
    method void org.pascalniklaus.grasp.show()
    method void org.pascalniklaus.grasp.toggle()
    method void org.pascalniklaus.grasp.tool(QString tool)
    method void org.pascalniklaus.grasp.width(int width)

These commands are then invoked like so:

    > qdbus org.pascalniklaus.grasp /exec quit
    > qdbus org.pascalniklaus.grasp /exec arrow "->"

Another alternative is to use 'dbus-send':

    > dbus-send --session --dest=org.pascalniklaus.grasp --type=method_call /exec org.pascalniklaus.grasp.quit
    > dbus-send --session --dest=org.pascalniklaus.grasp --type=method_call /exec org.pascalniklaus.grasp.arrow "->"
)END";
}

// ------------------------------ main -------------------------------

int main(int argc, char* argv[]) {
    QString key =
        QCryptographicHash::hash(argv[0], QCryptographicHash::Md5).toHex();
    SingletonApplication a(argc, argv, key);

    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);

    // setup command line options
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "GRASP -- GRab screen and Sketch during Presentations\n"
        "(C) 2024 by Pascal Niklaus\n\n");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList{"quit", "q"}, "quit application");
    parser.addOption(quitOption);

    QCommandLineOption toggleOption("toggle", "toggle application visibility");
    parser.addOption(toggleOption);

    QCommandLineOption showOption(
        "show", "show application (maximize it and place it on top)");
    parser.addOption(showOption);

    QCommandLineOption hideOption("hide", "hide application (minimize it)");
    parser.addOption(hideOption);

    QCommandLineOption configOption(
        "config", "use given config file for settings", "CONFIG-FILE");
    parser.addOption(configOption);

    QCommandLineOption grabOption(QStringList{"grab", "g"},
                                  "take screenshot and show it as new drawing");
    parser.addOption(grabOption);

    QCommandLineOption dbusOption(
        "help-dbus", "show detailed help about using the dbus interface");
    parser.addOption(dbusOption);

    parser.process(a);

    if (parser.isSet(dbusOption)) {
        print_dbus_help();
        return 0;
    }

    if (a.already_running()) {
        std::cerr << "Already running !\n";

        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            std::cerr << "Failed to connect to the session bus";
            return 1;
        }

        QString cmd;
        if (parser.isSet(quitOption)) { send_dbus_command(bus, "quit"); }
        if (parser.isSet(toggleOption)) { send_dbus_command(bus, "toggle"); }
        if (parser.isSet(showOption)) { send_dbus_command(bus, "show"); }
        if (parser.isSet(hideOption)) { send_dbus_command(bus, "hide"); }
        if (parser.isSet(grabOption)) { send_dbus_command(bus, "grab"); }
        return 0;
    } else {
        QSettings* qs;
        if (parser.isSet(configOption)) {
            qs = new QSettings(parser.value(configOption), QSettings::IniFormat, &a);
        } else {
            qs = new QSettings("org.pascalniklaus", "grasp", &a);
        }

        Message msg(&a);
        MainWindow w(*qs);

        // ------------ connect DBus messages to signales ------------

        // clang-format off
        QObject::connect(&msg, &Message::quit_received,
                         &w, &MainWindow::close);
        QObject::connect(&msg, &Message::show_received,
                         &w, &MainWindow::slot_show);
        QObject::connect(&msg, &Message::hide_received,
                         &w, &MainWindow::slot_hide);
        QObject::connect(&msg, &Message::toggle_received,
                         &w, &MainWindow::slot_toggle);
        QObject::connect(&msg, &Message::grab_received,
                         &w, &MainWindow::slot_grab);
        QObject::connect(&msg,&Message::grabnew_received,
                         &w, &MainWindow::slot_grab_and_new);
        QObject::connect(&msg, &Message::color_received,
                         &w, &MainWindow::slot_change_color);
        QObject::connect(&msg, &Message::tool_received,
                         &w, &MainWindow::slot_change_tool);
        QObject::connect(&msg, &Message::arrow_received,
                         &w, &MainWindow::slot_change_arrow);
        QObject::connect(&msg, &Message::width_received,
                         &w, &MainWindow::slot_change_width);
        // clang-format on

        // -------------------- set window params --------------------

        w.setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
        w.setStyleSheet("background:transparent;");
        w.setWindowFlags(Qt::FramelessWindowHint);
        w.show();
        // w.setWindowOpacity(1.0);
        w.setWindowOpacity(0.5);

        return a.exec();
    }
}
