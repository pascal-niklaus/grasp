#include "utility.h"

size_t utf8len(const std::string& str) {
    size_t len = 0;
    for (const auto& c : str)
        if ((c & 0xc0) != 0x80) len++;
    return len;
}
