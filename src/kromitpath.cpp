#include "kromitpath.h"

#include <QDebug>
#include <QPainter>

KromitPath::KromitPath()
{
    qDebug() << "KromitPath";
    original_path.moveTo(0,0);
    original_path.lineTo(100,0);
    original_path.lineTo(100,-150);
    original_path.arcTo(50,-200,50,50,0,90);
    //original_path.closeSubpath();

    setPath(original_path);

    qDebug() << original_path;

}

void KromitPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(QPen(Qt::yellow, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setRenderHint( QPainter::Antialiasing );
    painter->drawPath( path() );
}

