#include "tools.h"

#include "kromitline.h"
#include "kromitpath.h"
#include "kromitsmooth.h"
#include "kromitortho.h"
#include "kromitrect.h"
#include "kromittext.h"
#include "kromitpixmap.h"
#include "kromitfader.h"
#include "kromithighlight.h"

int Tools::generate_ID(void) {
    static int id = 0;
    return ++id;
}

QGraphicsItem *Tools::create_tool(const Tools::Settings& settings) {
    switch (settings.tool) {
        case TOOL_ORTHO: {
           auto *ortho = new KromitOrtho(nullptr,
                                         settings.color,
                                         settings.width,
                                         settings.arrow_type,
                                         settings.arrow_size,
                                         settings.simplify,
                                         settings.snapdist,
                                         settings.radius,
                                         settings.minlen,
                                         settings.maxangle);
            ortho->setData(KromitItem::KeyID, generate_ID());
            return ortho;
        }

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

        case TOOL_FADER: {
            auto *fader = new KromitFader(nullptr,
                                        settings.color,
                                        settings.width,
                                        settings.arrow_type,
                                        settings.arrow_size,
                                        settings.sustain,
                                        settings.fade);
            fader->setData(KromitItem::KeyID, generate_ID());
            return fader;
        }

        case TOOL_HIGHLIGHT: {
            auto *highlight = new KromitHighlight(nullptr,
                                                  settings.color,
                                                  settings.width,
                                                  settings.arrow_type,
                                                  settings.arrow_size,
                                                  settings.sustain,
                                                  settings.fade);
            highlight->setData(KromitItem::KeyID, generate_ID());
            return highlight;
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

        case TOOL_RECT: {
            auto *rect = new KromitRect(nullptr,
                                        settings.color,
                                        settings.width,
                                        settings.radius);
            rect->setData(KromitItem::KeyID, generate_ID());
            return rect;
        }

        case TOOL_TEXT: {
            auto *text = new KromitText(
                nullptr, settings.color, settings.fontsize);
            text->setData(KromitItem::KeyID, generate_ID());
            return text;
        }

        case TOOL_PIXMAP: {
            auto *image = new KromitPixmap(nullptr, settings.color, 2);
            image->setData(KromitItem::KeyID, generate_ID());
            return image;
        }

        default:
            return nullptr;
    }
}
