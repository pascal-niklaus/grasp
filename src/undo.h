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
