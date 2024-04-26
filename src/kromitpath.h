#ifndef KROMITPATH_H
#define KROMITPATH_H

#include "kromit.h"
#include "arrowitem.h"
#include "path.h"

#include <QPainterPath>
#include <QPen>
#include <QColor>

class KromitUndoPath;

// KromitPath
//
class KromitPath : public ArrowItem {
  public:
    enum { Type = GFX_PATH };

  public:
    KromitPath(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double width = 1,
               ArrowType atype = ARROW_NONE,
               double arrowsize = 0.0);
    ~KromitPath();

    virtual KromitUndoItem* create_undo_item() const override;

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

    virtual QRectF selectionBox() const override {
        return QRectF(_path.boundingRect());
    }
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_ROTATE |
                CAP_DELETE);
    }

    virtual void stash() override;
    virtual void apply(const QTransform& m) override;

    virtual void start(const QPointF& p, double pressure = 1.0) override;
    virtual void move(const QPointF& p, double pressure = 1.0) override;
    virtual void stop(const QPointF& p, double pressure = 1.0) override;

    virtual void dump(std::ostream& os) const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

    QPainterPath outline(double dist) const;
    virtual QPainterPath shape() const override;

    QPen pen() const { return _pen; }
    ArrowType arrow_type() const { return _arrow_type; }
    double arrow_size() const { return _arrow_size; }
    const Path& path() const { return _xy_path; }
    void set_path(const Path& path);

  private:
    void repos_arrows();

  private:
    QPen _pen;
    ArrowType _arrow_type = ARROW_NONE;
    double _arrow_size = 0.0;
    KromitArrow* _arrow_start = nullptr;
    KromitArrow* _arrow_end = nullptr;

    Path _xy_path;
    QPainterPath _path;

    Path _stashed_xy_path;
};

// KromitUndoPath
//
class KromitUndoPath : public KromitUndoItem {
  public:
    explicit KromitUndoPath(const KromitPath* kpath);

    virtual KromitItem* create_from_undo_item() const override;

    QPen pen;
    ArrowType arrow_type;
    double arrow_size;
    Path xy_path;
};


// KromitPathW
//
// same as KromitPath, but with variable width
//
class KromitPathW : public QGraphicsItem {
  public:
    enum { Type = GFX_PATHW };

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
