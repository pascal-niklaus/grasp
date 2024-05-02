#include "kromit.h"
#include "kromititem.h"
#include "string_format.h"

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

QString KromitItem::type_txt(int item) {
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
        case GFX_SMOOTH:
            return "Smooth";
        case GFX_TEXT:
            return "Text";
        case GFX_PIXMAP:
            return "Pixmap";
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

std::ostream& operator<<(std::ostream& os, const KromitItem* item) {
    os << *item;
    return os;
}

std::ostream& operator<<(std::ostream& os, const KromitItem& item) {
    os << KromitItem::type_txt(item.type()).toStdString() << ":";
    bool ok;
    auto id = item.data(KromitItem::KeyID).toInt(&ok);
    if (ok) os << " id=" << id;
    item.dump(os);
    if (item.isSelected()) os << " selected";
    if (item.pos().x() != 0 || item.pos().y() != 0)
        os << string_format(" pos=(%.0f,%.0f)", item.pos().x(), item.pos().y());

    os << string_format(" bbox=[(%.0f,%.0f) to (%.0f,%.0f)]",
                        item.boundingRect().topLeft().x(),
                        item.boundingRect().topLeft().y(),
                        item.boundingRect().bottomRight().x(),
                        item.boundingRect().bottomRight().y());
    return os;
}

// ============================ undo item ============================

KromitUndoItem::KromitUndoItem(const KromitItem * item) {
    type = item->type();
    id = item->data(KromitItem::KeyID).toInt();
}

std::ostream& operator<<(std::ostream& os, const KromitUndoItem& item) {
    os << KromitItem::type_txt(item.type).toStdString() << ":";
    os << " id=" << item.id;
    item.dump(os);
    return os;
}
