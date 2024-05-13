#ifndef GRASP_H
#define GRASP_H

#include <QGraphicsItem>

#include <cstdint>

enum SelectType { SEL_NONE = 0, SEL_SINGLE = 1, SEL_TOGGLE = 2, SEL_ADD = 3 };

enum ToolType : std::uint16_t {
    TOOL_PEN = 1,
    TOOL_PENW,
    TOOL_RECT,
    TOOL_LINE,
    TOOL_SMOOTH,
    TOOL_ORTHO,
    TOOL_FADER,
    TOOL_HIGHLIGHT,
    TOOL_TEXT,
    TOOL_PIXMAP,
    TOOL_SELECT
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
    CAP_DELETE = 0x10,
    CAP_KEYSTROKES = 0x20,
    CAP_DYNAMIC = 0x40,
};

TransformCaps operator&(const TransformCaps& a, const TransformCaps& b);
TransformCaps operator|(const TransformCaps& a, const TransformCaps& b);
std::ostream &operator<<(std::ostream &os, const TransformCaps &cap);

enum GfxItemType : int {
    GFX_ARROW = QGraphicsItem::UserType + 1,
    GFX_ARROWITEM,
    // --------------------------- dash ----------------------------
    GFX_DASH,
    GFX_DASH_MIN,
    GFX_SCENE,
    GFX_DASHICON,
    GFX_ARROWICON,
    GFX_COLOR,
    GFX_DASH_MAX,
    // ---------------------- selection frame ----------------------
    GFX_HANDLE,
    GFX_ROTATEHANDLE,
    GFX_MOVEHANDLE,
    GFX_SELFRAME,
    // ----------- all items that follow are selectable ------------
    GFX_SELECTABLE,
    GFX_PATH,
    GFX_PATHW,
    GFX_SMOOTH,
    GFX_LINE,
    GFX_RECT,
    GFX_ORTHO,
    GFX_HIGHLIGHT,
    GFX_FADER,
    GFX_TEXT,
    GFX_PIXMAP,
};

#endif
