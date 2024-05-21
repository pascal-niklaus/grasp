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

#include "undo.h"
#include <unordered_set>

bool UndoRecord::contains(int id) const {
    return ids_to_add.contains(id) || ids_to_remove.contains(id);
}

bool UndoRecord::all_ids_in(std::unordered_set<int> &ref) const {
    for (auto v : ids_to_add)
        if (!ref.contains(v)) return false;
    for (auto v : ids_to_remove)
        if (!ref.contains(v)) return false;
    return true;
}

std::unordered_set<int> UndoRecord::all_ids() const {
    std::unordered_set<int> all;
    all = ids_to_add;
    for (auto v : ids_to_remove) all.insert(v);
    return all;
}

std::ostream &operator<<(std::ostream &os, const UndoRecord &rec) {
    os << "remove:";
    for (auto v : rec.ids_to_remove) os << " " << v;
    os << "\n";
    os << "add:   ";
    for (auto v : rec.ids_to_add) os << " " << v;
    os << "\n";
    return os;
}
