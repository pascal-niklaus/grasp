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

#ifndef UNDO_H
#define UNDO_H

#include <vector>
#include <iostream>
#include <unordered_set>

// UndoRecord
//
// when undoing, 'ids_to_remove' are moved from the scene to the redo
// store.

class UndoRecord {
  public:
    std::unordered_set<int> ids_to_remove;
    std::unordered_set<int> ids_to_add;

    bool contains(int id) const;
    bool all_ids_in(std::unordered_set<int> &ref) const;
    std::unordered_set<int> all_ids() const;
    friend std::ostream &operator<<(std::ostream &os, const UndoRecord &rec);
};

#endif
