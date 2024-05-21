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

#ifndef SELECTKEY_H
#define SELECTKEY_H

#include <QKeySequenceEdit>

class SelectKey : public QKeySequenceEdit {
    Q_OBJECT
  public:
    SelectKey(Qt::KeyboardModifiers allowed_mods = Qt::NoModifier,
              QWidget* parent = nullptr);
    SelectKey(std::uint32_t allowed_mods = Qt::NoModifier,
              QWidget* parent = nullptr);

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
    void setup(Qt::KeyboardModifiers mods);

  private:
    QKeyCombination _combi;
    Qt::KeyboardModifiers _mods;

    QKeyCombination _delete_key;
    QKeyCombination _prev_combi;
};

#endif
