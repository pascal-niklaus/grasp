#ifndef KROMIT_H
#define KROMIT_H

#include <QGraphicsItem>

#include <cstdint>

enum SelectType { SEL_NONE = 0, SEL_SINGLE = 1, SEL_TOGGLE = 2, SEL_ADD = 3 };

enum ToolType : std::uint16_t {
    TOOL_PEN = 1,
    TOOL_PENW = 2,
    TOOL_RECT = 3,
    TOOL_LINE = 4,
    TOOL_SMOOTH = 5,
    TOOL_ORTHO = 6,
    TOOL_SELECT = 7
};

enum ArrowType : std::uint16_t {
    ARROW_NONE = 0,
    ARROW_START = 1,
    ARROW_END = 2,
    ARROW_DOUBLE = ARROW_START | ARROW_END
};

enum TransformCaps : std::uint16_t {
    CAP_NONE = 0x00,
    CAP_MOVE = 0x01,
    CAP_ISOSCALE = 0x02,
    CAP_ANISOSCALE = 0x04,
    CAP_ROTATE = 0x08,
    CAP_DELETE = 0x10
};

TransformCaps operator&(const TransformCaps& a, const TransformCaps& b);
TransformCaps operator|(const TransformCaps& a, const TransformCaps& b);

enum GfxItemType : int {
    GFX_ARROW = QGraphicsItem::UserType + 1,
    GFX_ARROWITEM,
    GFX_HANDLE,
    GFX_ROTATEHANDLE,
    GFX_MOVEHANDLE,
    GFX_SELFRAME,
    GFX_SELECTABLE,  // all items after are selectable
    GFX_PATH,
    GFX_PATHW,
    GFX_LINE,
    GFX_RECT
};

#endif
