#ifndef KROMITRECT_H
#define KROMITRECT_H

#include "kromit.h"
#include "arrowitem.h"

#include <QPen>

class KromitUndoRect;

// KromitRect
//
class KromitRect : public KromitItem {
  public:
    enum { Type = GFX_RECT };

  public:
    KromitRect(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double width = 1,
               double radius = 0);
    ~KromitRect();

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
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_DELETE);
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
    double radius() const { return _radius; }
    const QRectF& rect() const { return _rect; }
    void set_rect(const QRectF& rect);

  private:
    // ----- internal helpers
    QPainterPath outline(double dist) const;
    void build_path();

  private:
    QPen _pen;
    double _radius;
    QRectF _rect;

    QPainterPath _path;

    QRectF _stashed_rect;
};

// KromitUndoRect
//
class KromitUndoRect : public KromitUndoItem {
  public:
    explicit KromitUndoRect(const KromitRect* kpath);

    virtual KromitItem* create_from_undo_item() const override;

    QPen pen;
    double radius;
    QRectF rect;
};

#endif  // KROMITRECT_H
