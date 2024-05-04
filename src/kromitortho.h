#ifndef KROMITORTHO_H
#define KROMITORTHO_H

#include <QPen>

#include "arrowitem.h"
#include "kromit.h"
#include "path.h"

class KromitUndoOrtho;

// KromitOrtho
//
class KromitOrtho : public ArrowItem {
  public:
    enum { Type = GFX_ORTHO };

  public:
    explicit KromitOrtho(QGraphicsItem* parent,
                         const QColor& color,
                         double width,
                         ArrowType atype,
                         double arrowsize,
                         double simplify,
                         double snap,
                         double radius,
                         double minlen,
                         double maxangle);
    ~KromitOrtho();

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
    void move_internal(const QPointF& p, bool force);
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
    double snap() const { return _snap; }
    double simplify() const { return _simplify; }
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
    double _snap = 0.0;
    double _simplify = 0.0;
    double _radius = 0.0;
    double _minlen = 0.0;
    double _maxangle = 10.0;
    KromitArrow* _arrow_start = nullptr;
    KromitArrow* _arrow_end = nullptr;

    Path _xy_path;
    Path _xy_path_raw;
    QPainterPath _path;
    QPainterPath _raw_path;

    Path _stashed_xy_path;
};

// KromitUndoOrtho
//
class KromitUndoOrtho : public KromitUndoItem {
  public:
    explicit KromitUndoOrtho(const KromitOrtho* kpath);

    virtual KromitItem* create_from_undo_item() const override;

    QPen pen;
    ArrowType arrow_type;
    double arrow_size;
    Path xy_path;
};

#endif  // KROMITORTHO_H
