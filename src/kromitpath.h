#ifndef KROMITPATH_H
#define KROMITPATH_H

#include "kromit.h"
#include "arrowitem.h"
#include "path.h"

#include <QPen>

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

    // ----- KromitItem methods

    // - undo system
    virtual KromitUndoItem* create_undo_item() const override;

    // - transform system
    virtual void stash() override;
    virtual void apply(const QTransform& m) override;
    virtual QRectF selectionBox() const override {
        return QRectF(_path.boundingRect());
    }
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_ROTATE |
                CAP_DELETE);
    }

    // - build system
    virtual void start(const QPointF& p, double pressure = 1.0) override;
    virtual void move(const QPointF& p, double pressure = 1.0) override;
    virtual void stop(const QPointF& p, double pressure = 1.0) override;

    // - debugging
    virtual void dump(std::ostream& os) const override;

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual QPainterPath shape() const override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

  public:
    // ----- item accessors
    QPen pen() const { return _pen; }
    ArrowType arrow_type() const { return _arrow_type; }
    double arrow_size() const { return _arrow_size; }
    const Path& path() const { return _xy_path; }
    void set_path(const Path& path);

  private:
    // ----- internal helpers
    void repos_arrows();
    QPainterPath outline(double dist) const;

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

#endif  // KROMITPATH_H
