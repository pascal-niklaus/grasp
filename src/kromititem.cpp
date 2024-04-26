#include "kromit.h"
#include "kromititem.h"

#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>

bool KromitItem::select() {
    prepareGeometryChange();
    if (!isSelected()) {
        setSelected(true);
        return true;
    }
    return false;
}

bool KromitItem::deselect() {
    prepareGeometryChange();
    if (isSelected()) {
        setSelected(false);
        return true;
    }
    return false;
}

bool KromitItem::toggleselect() {
    prepareGeometryChange();
    setSelected(!isSelected());
    return true;
}

QString KromitItem::type(int item) {
    switch (item) {
        case GFX_ARROWITEM:
            return "ArrowItem";
        case GFX_ARROW:
            return "Arrow";
        case GFX_LINE:
            return "Line";
        case GFX_PATH:
            return "Path";
        case GFX_RECT:
            return "Rect";
        case GFX_SELFRAME:
            return "SelFrame";
        case GFX_HANDLE:
            return "Handle";
        case GFX_ROTATEHANDLE:
            return "RotateHandle";
        case GFX_MOVEHANDLE:
            return "MoveHandle";
        case QGraphicsLineItem::Type:
            return "QLineItem";
        case QGraphicsPathItem::Type:
            return "QPathItem";
        case QGraphicsPolygonItem::Type:
            return "QPolyItem";
        case QGraphicsRectItem::Type:
            return "QRectItem";
        default:
            return QString("Item-%1").arg(item);
    }
}
