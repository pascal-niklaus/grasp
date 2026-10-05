/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "mainwindow.h"
#include "singletonapplication.h"
#include "dbus.h"
#include "settings.h"
#include "verbosity.h"
#include "utility.h"

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCommandLineParser>
#include <QDebug>
#include <QSettings>

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
    QGuiApplication::setDesktopFileName("grasp");

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

    QCommandLineOption verboseOption("verbose", "print messages, higher 'LEVEL' = more messages", "LEVEL", "1");
    parser.addOption(verboseOption);

    QCommandLineOption dbusOption(
        "help-dbus", "show detailed help about using the dbus interface");
    parser.addOption(dbusOption);

    QCommandLineOption portalOnlyOption(
        "portal-only",
        "for testing: ignores D-Bus toggle/grabnew so only the "
        "GlobalShortcuts-portal-assigned shortcut can trigger them");
    parser.addOption(portalOnlyOption);

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
        bool ok;
        int verbosity = parser.value(verboseOption).toInt(&ok);
        if (ok) {
          Verbosity::set(cap<int>(verbosity,0,10));
        } else {
            std::cerr << "Illegal verbosity value, turning off\n";
        }

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
        if (parser.isSet(portalOnlyOption)) {
            if (QGuiApplication::platformName() == "wayland") {
                qWarning() << "--portal-only: D-Bus toggle/grabnew disabled; use "
                              "the GlobalShortcuts-portal-assigned shortcut instead";
            } else {
                qWarning() << "--portal-only: D-Bus toggle/grabnew disabled, but "
                              "GlobalShortcuts is Wayland-only -- there is no "
                              "replacement trigger on this platform ("
                           << QGuiApplication::platformName() << ")";
            }
        } else {
            QObject::connect(&msg, &Message::toggle_received,
                             &w, &MainWindow::slot_toggle);
            QObject::connect(&msg,&Message::grabnew_received,
                             &w, &MainWindow::slot_grab_and_new);
        }
        QObject::connect(&msg, &Message::grab_received,
                         &w, &MainWindow::slot_grab);
        QObject::connect(&msg, &Message::color_received,
                         &w, &MainWindow::slot_change_color);
        QObject::connect(&msg, &Message::tool_received,
                         &w, &MainWindow::slot_change_tool);
        QObject::connect(&msg, &Message::arrow_received,
                         &w, &MainWindow::slot_change_arrow);
        QObject::connect(&msg, &Message::width_received,
                         &w, &MainWindow::slot_change_width);
        QObject::connect(&msg, &Message::rect_received,
                         &w, &MainWindow::slot_rect);
        // clang-format on

        // -------------------- set window params --------------------

        w.setWindowFlags(w.windowFlags() | Qt::FramelessWindowHint |
                         Qt::WindowStaysOnTopHint);
        w.setStyleSheet("background:transparent;");
        w.setWindowState(Qt::WindowFullScreen | Qt::WindowMaximized);
        w.show();

        return a.exec();
    }
}
