#ifndef KROMIT_H
#define KROMIT_H

enum ToolType {
    TOOL_PEN = 1,
    TOOL_PENW = 2,
    TOOL_RECT = 3,
    TOOL_LINE = 4,
    TOOL_SMOOTH = 5,
    TOOL_ORTHO = 6
};

enum ArrowType {
    ARROW_NONE = 0,
    ARROW_START = 1,
    ARROW_END = 2,
    ARROW_DOUBLE = ARROW_START | ARROW_END
};

enum GfxItemType : int {
    GFX_ARROW = 1,
    GFX_ARROWITEM,
    GFX_PATH,
    GFX_PATHW,
    GFX_LINE,
    GFX_RECT
};

#endif
