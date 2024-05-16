#ifndef _DBUS_H
#define _DBUS_H

#include <qstringconverter.h>
#include <qtmetamacros.h>
#include <QObject>

#include <iostream>

class Message : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.pascalniklaus.grasp")

  public:
    Message(QObject* parent);
    ~Message();

    void set_verbose(bool v) { _verbose = v; }
    bool verbose() const { return _verbose; }
    void log(const std::string& msg) const;

    static QString service_name() { return "org.pascalniklaus.grasp"; }
    static QString service_path() { return "/exec"; }

  signals:
    void quit_received();

    void toggle_received();
    void show_received();
    void hide_received();

    void grab_received();
    void grabnew_received();

    void color_received(int index);
    void arrow_received(const QString&);
    void tool_received(const QString&);
    void width_received(int);

  public Q_SLOTS:
    Q_SCRIPTABLE void quit();

    Q_SCRIPTABLE void toggle();
    Q_SCRIPTABLE void show();
    Q_SCRIPTABLE void hide();

    Q_SCRIPTABLE void grab();
    Q_SCRIPTABLE void grabnew();

    Q_SCRIPTABLE void tool(const QString& tool);
    Q_SCRIPTABLE void arrow(const QString& arrow);
    Q_SCRIPTABLE void color(int index);
    Q_SCRIPTABLE void width(int width);

  private:
    bool _verbose = false;
};

#endif
