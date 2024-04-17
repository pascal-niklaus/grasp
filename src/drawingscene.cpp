#include "drawingscene.h"
#include "kromitpath.h"

DrawingScene::DrawingScene(QObject *parent)
    : QGraphicsScene{parent}
{
    KromitPath *k = new KromitPath;
    addItem(k);
}

void DrawingScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mousePressEvent(event);
}

void DrawingScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
}

void DrawingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
}

/*
 *  addLine
 *  addPath
 *  addRect
 *  addPolygon
 *  addText
 *  addItem(...)
 *
 * setSceneRect()
 */

/* QGraphicsItem
 *
 * implement own:
 * - boundingRect
 * - paint
 * - contains() (as alternative to shape)
 *
 * - grabKeyboard
*
* shape
 *
 *
 */
