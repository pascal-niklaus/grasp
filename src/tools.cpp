#include "tools.h"

#include "kromitline.h"
#include "kromitpath.h"
#include "kromitsmooth.h"

int Tools::generate_ID(void) {
  static int id = 0;
  return ++id;
}

QGraphicsItem *Tools::create_tool(const Tools::Settings& settings) {
    switch (settings.tool) {
        case TOOL_SMOOTH: {
            auto *pen = new KromitSmooth(nullptr,
                                         settings.color,
                                         settings.width,
                                         settings.arrow_type,
                                         settings.arrow_size,
                                         settings.simplify,
                                         settings.snapdist);
            pen->setData(KromitItem::KeyID, generate_ID());
            return pen;
        }

        case TOOL_PEN: {
            auto *pen = new KromitPath(nullptr,
                                       settings.color,
                                       settings.width,
                                       settings.arrow_type,
                                       settings.arrow_size);
            pen->setData(KromitItem::KeyID, generate_ID());
            return pen;
        }

        case TOOL_LINE: {
            auto *line = new KromitLine(nullptr,
                                        settings.color,
                                        settings.width,
                                        settings.arrow_type,
                                        settings.arrow_size);
            line->setData(KromitItem::KeyID, generate_ID());
            return line;
        }

            // case TOOL_LINE: {
            //     auto *line = new KromitLine(parent);
            //     return line;
            // }

        default:
            return nullptr;
    }
}
