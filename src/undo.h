#ifndef UNDO_H
#define UNDO_H

#include <vector>

// UndoRecord
//
// when undoing, 'ids_to_remove' are moved from the scene to the redo
// store.

class UndoRecord {
  public:
    std::vector<int> ids_to_remove;
    std::vector<int> ids_to_add;
};

#endif
