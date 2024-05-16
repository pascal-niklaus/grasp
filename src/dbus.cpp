#include "dbus.h"

#include <QDBusConnection>
#include <QDebug>

Message::Message(QObject *parent)
  : QObject(parent){
    QDBusConnection::sessionBus().registerService(service_name());
    QDBusConnection::sessionBus().registerObject(
        service_path(), this, QDBusConnection::ExportScriptableContents);
}

Message::~Message() {}

void Message::quit() {
    emit quit_received();
}

void Message::log(const std::string& msg) const {
    std::cerr << "DBus:" << msg << "\n";
}

// ---------------------------- show/hide ----------------------------

void Message::show() {
    log("'show' received");
    emit show_received();
}

void Message::hide() {
    log("'hide' received");
    emit hide_received();
}

void Message::toggle() {
    log("'toggle' received");
    emit toggle_received();
}

// ---------------------------- grabbing -----------------------------

void Message::grab() {
    log("'grab' received");
    emit grab_received();
}

void Message::grabnew() {
    log("'grabnew' received");
    emit grabnew_received();
}

// ---------------------------- parameter ----------------------------

void Message::color(int index) {
  // human color index starts at 1, internally we start at 0
  log("'color' received: index=" + std::to_string(index));
  emit color_received(index - 1);
}

void Message::arrow(const QString& arrow) {
    log("'arrow' received: " + arrow.toStdString());
    emit arrow_received(arrow);
}

void Message::tool(const QString& tool) {
    log("'tool' received: " + tool.toStdString());
    emit tool_received(tool);
}

void Message::width(int w) {
    log("'width' received: " + std::to_string(w));
    emit width_received(w);
}
