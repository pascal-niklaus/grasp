#include "dbus.h"

#include <QDBusConnection>
#include <QDebug>

Message::Message(QObject *parent)
  : QObject(parent){
    QDBusConnection::sessionBus().registerService("org.pascal.grasp");
    QDBusConnection::sessionBus().registerObject("/exec", this, QDBusConnection::ExportScriptableContents);
}

Message::~Message() {}

void Message::quit() {
    emit quit_received();
}

// ---------------------------- show/hide ----------------------------

void Message::show() {
    emit show_received();
}

void Message::hide() {
    emit hide_received();
}

void Message::toggle() {
    emit toggle_received();
}

// ---------------------------- grabbing -----------------------------

void Message::grab() {
    emit grab_received();
}

void Message::grabnew() {
    emit grabnew_received();
}

// ---------------------------- parameter ----------------------------

void Message::color(int index) {
  // human color index starts at 1, internally we start at 0
    emit color_received(index - 1);
}

void Message::arrow(const QString& arrow) {
    emit arrow_received(arrow);
}

void Message::tool(const QString& tool) {
    qDebug() << "Tool received!" << tool;
    emit tool_received(tool);
}

void Message::width(int w) {
    emit width_received(w);
}
