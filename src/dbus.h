#ifndef _DBUS_H
#define _DBUS_H

#include <qstringconverter.h>
#include <qtmetamacros.h>
#include <QObject>

class Message : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.pascal.kromit")

  public:
    Message(QObject* parent);
    ~Message();

    static QString service_name() { return "org.pascal.kromit"; }

 signals:
    void quit_received();
    void toggle_received();
    void color_received(const QString&);
    void screenshot_received(const QString&);

  public Q_SLOTS:
    Q_SCRIPTABLE void quit();
    Q_SCRIPTABLE void toggle();
    Q_SCRIPTABLE void color(const QString& color);
    Q_SCRIPTABLE void screenshot(const QString& geometry);

};

#endif
