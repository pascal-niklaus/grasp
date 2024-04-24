#ifndef TOOLS_H
#define TOOLS_H

#include <QGraphicsItem>
#include <QPointF>
#include <QColor>

#include "kromit.h"

namespace Tools {

// Tool::Settings
//
// all settings defining the tools
//
class Settings {
  public:
    ToolType tool;
    QColor color;
    double width;
    double minwidth;
    double maxwidth;
    double arrow_size;
    ArrowType arrow_type;
    double radius;
    double minlen;
    double maxangle;
    double simplify;
    double snapdist;
};

    // Tool::create_tool
    //
    // factory function
    //
QGraphicsItem* create_tool(const Tools::Settings& settings);

int generate_ID(void);

};  // namespace Tools

#endif
