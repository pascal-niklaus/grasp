#include "drawingscene.h"

#include <qevent.h>
#include <qglobal.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qguiapplication.h>
#include <qnamespace.h>

#include <QApplication>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QScreen>
#include <QTabletEvent>
#include <iostream>

#include "kromitpath.h"

// DrawingScene::DrawingScene
//
DrawingScene::DrawingScene(QObject *parent) : QGraphicsScene{parent} {
    // fix geometry, otherwise scene moves around when items are added
    setSceneRect(QGuiApplication::primaryScreen()->geometry());
}


// forward mouse events to the Scene...

void DrawingScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mousePressEvent(event);
}

void DrawingScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
}

void DrawingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
}

// DrawingScene::mapped
//
// map view coordinates to GraphicsScene coordinates
// this requires that a view already exists !
//
QPointF DrawingScene::mapped(QPointF p) const {
    const QGraphicsView *v = views().first();
    if (v) {
        return v->mapToScene(p.x() + 0.5, p.y() + 0.5);
    } else {
        Q_ASSERT(false);
        return QPointF(0, 0);
    }
}

// DrawingScene::event
//
// here tablet and mouse events are processed. The coordinates for
// these events need to be mapped to the scene coordinates

bool DrawingScene::event(QEvent *event) {
    switch (event->type()) {
            // ------------- tablet press, move, release -------------

        case QEvent::TabletPress: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            current_item = new KromitPath();
            KromitPath *path = static_cast<KromitPath *>(current_item);
            QPointF p = mapped(ev->scenePosition());
            double pressure = ev->pressure();
            path->start(p);
            path->line_to(p);
            addItem(current_item);
            ev->accept();
            return true;
        }

        case QEvent::TabletMove: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            double pressure = ev->pressure();
            if (current_item) {
                KromitPath *path = static_cast<KromitPath *>(current_item);
                path->line_to(mapped(ev->scenePosition()));
            }
            ev->accept();
            return false;
        }

        case QEvent::TabletRelease: {
            QTabletEvent *ev = static_cast<QTabletEvent *>(event);
            double pressure = ev->pressure();
            if (current_item) {
                KromitPath *path = static_cast<KromitPath *>(current_item);
                path->line_to(mapped(ev->scenePosition()));
                current_item = nullptr;
            }
            ev->accept();
            return true;
        }

            // ---------------- key press and release ----------------

        case QEvent::KeyPress:
        case QEvent::KeyRelease:
            return QGraphicsScene::event(event);

            // --------------------- mouse event ---------------------

        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            current_item = new KromitPath();
            KromitPath *path = static_cast<KromitPath *>(current_item);
            QPointF p = mapped(ev->scenePos());
            path->start(p);
            path->line_to(p);
            addItem(current_item);
            ev->accept();
            return true;
        }

        case QEvent::GraphicsSceneMouseMove: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            if (current_item) {
                KromitPath *path = static_cast<KromitPath *>(current_item);
                path->line_to(mapped(ev->scenePos()));
            }
            ev->accept();
            return false;
        }

        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *ev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            if (current_item) {
                KromitPath *path = static_cast<KromitPath *>(current_item);
                path->line_to(mapped(ev->scenePos()));
                current_item = nullptr;
            }
            ev->accept();
            return false;
        }
        default:
            return QGraphicsScene::event(event);
    }
}
