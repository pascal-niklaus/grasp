#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include <QGraphicsScene>

class DrawingScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit DrawingScene(QObject *parent = nullptr);

protected:   
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
};

#endif // DRAWINGSCENE_H
