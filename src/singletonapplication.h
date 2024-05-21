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

#ifndef _SINGLETONAPPLICATION_H
#define _SINGLETONAPPLICATION_H

#include <QApplication>
#include <QSharedMemory>

class SingletonApplication : public QApplication {
    Q_OBJECT

  public:
    SingletonApplication(int &argc, char **argv, const QString &mem_id) :
        QApplication(argc, argv, true) {
        _singleton = new QSharedMemory(mem_id, this);
    }

    ~SingletonApplication() {
        if (_singleton->isAttached()) _singleton->detach();
    };

    bool already_running() {
        if (_singleton->attach(QSharedMemory::ReadOnly)) {
            _singleton->detach();
        } else {
            if (_singleton->create(1)) return false;
        }
        return true;
    }

  private:
    QSharedMemory *_singleton = nullptr;
};

#endif
