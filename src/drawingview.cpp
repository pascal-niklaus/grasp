#include "drawingview.h"
#include <qgraphicsitem.h>
#include "drawingscene.h"

#include <QEvent>
#include <QApplication>
#include <QGraphicsView>

DrawingView::DrawingView(DrawingScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

bool DrawingView::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TabletMove:
            QApplication::sendEvent(scene(), event);
            return true;
        default:
            return QGraphicsView::event(event);
    }
}

void DrawingView::set_background(QPixmap px) {
    if (background_item) scene()->removeItem(background_item);
    background_item = scene()->addPixmap(px);
    background_item->setOffset(0, 0);
    background_item->setZValue(-1000.0);
}
