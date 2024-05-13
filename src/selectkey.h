#ifndef SELECTKEY_H
#define SELECTKEY_H

#include <QKeySequenceEdit>

class SelectKey : public QKeySequenceEdit {
    Q_OBJECT
  public:
    SelectKey(QWidget *parent = nullptr);

    void set_key(const QKeyCombination& keycomb);
    void set_del_key(const QKeyCombination& delcomb);
    void set_allowed_modifiers(Qt::KeyboardModifiers mods);
    QKeyCombination get_key() const { return _combi; }
    QKeyCombination get_canonical() const {
        return SelectKey::canonicalize(_combi);
    }

    static QKeyCombination canonicalize(const QKeyCombination& combi);

  protected:
    virtual bool event(QEvent*) override;

  signals:
    void status_message(const QString& msg);

  private slots:
    void slot_check_key_combi();

  private:
    QKeyCombination _combi;
    Qt::KeyboardModifiers _mods;

    QKeyCombination _delete_key;
    QKeyCombination _prev_combi;
};

#endif
