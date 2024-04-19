#include "drawingview.h"


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
