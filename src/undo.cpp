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
