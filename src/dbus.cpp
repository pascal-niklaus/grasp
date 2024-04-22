#include "dbus.h"

#include <QDBusConnection>
#include <QDebug>

Message::Message(QObject *parent)
  : QObject(parent){
    QDBusConnection::sessionBus().registerService("org.pascal.kromit");
    QDBusConnection::sessionBus().registerObject("/config", this, QDBusConnection::ExportScriptableContents);
}

Message::~Message() {}

void Message::quit() {
  qDebug() << "quit received!";
  emit quit_received();
}

void Message::toggle() {
  qDebug() << "toggle received!";
  emit toggle_received();
}

void Message::color(const QString& color) {
  qDebug() << "Message: color received!" << color;
  emit color_received(color);
}

void Message::screenshot(const QString& geometry) {
  qDebug() << "Message: snapshot received!" << geometry;
  emit screenshot_received(geometry);
}
