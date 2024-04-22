#ifndef KROMITPATH_H
#define KROMITPATH_H

#include <qnamespace.h>
#include <QGraphicsItem>
#include <QPointF>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QColor>

#include "kromit.h"
#include "kromitarrow.h"
#include "arrowitem.h"
#include "path.h"

// KromitPath
//
class KromitPath : public ArrowItem {
  public:
    enum { Type = static_cast<int>(UserType) + GFX_PATH };

  public:
    KromitPath(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double width = 1,
               ArrowType atype = ARROW_NONE,
               double arrowsize = 0.0);
    ~KromitPath();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

    void start(const QPointF& p);
    void stop(const QPointF& p);
    void line_to(double x, double y);
    void line_to(const QPointF& p);

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

  private:
    void repos_arrows();

  private:
    QPen _pen;
    ArrowType _arrow_type = ARROW_NONE;
    double _arrow_width = 0.0;
    KromitArrow* _arrow_start = nullptr;
    KromitArrow* _arrow_end = nullptr;

    Path _xy_path;
    QPainterPath _path;
};

// KromitPathW
//
// same as KromitPath, but with variable width
//
class KromitPathW : public QGraphicsItem {
  public:
    enum { Type = static_cast<int>(UserType) + GFX_PATHW };

  public:
    KromitPathW();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

    void start(const QPointF& p, double w);
    void stop(const QPointF& p, double w);
    void line_to(double x, double y, double w);
    void line_to(const QPointF& p, double w);

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

  private:
    QPen _pen;
    PathW _xyw_path;
    QPainterPath _path;
};

#endif  // KROMITPATH_H
