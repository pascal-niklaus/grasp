#ifndef KROMITPIXMAP_H
#define KROMITPIXMAP_H

#include "kromit.h"
#include "arrowitem.h"

#include <QPen>

class KromitUndoPixmap;

// KromitPixmap
//
class KromitPixmap : public KromitItem {
  public:
    enum { Type = GFX_PIXMAP };

  public:
    KromitPixmap(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double width = 1);
    ~KromitPixmap();

    // ----- KromitItem methods

    // - undo system
    virtual KromitUndoItem* create_undo_item() const override;

    // - transform system
    virtual void stash() override;
    virtual void apply(const QTransform& m) override;
    virtual QRectF selectionBox() const override { return _sel_rect; }
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ISOSCALE | CAP_DELETE);
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
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

  public:
    // ----- item accessors
    QPen pen() const { return _pen; }
    const QPixmap& pixmap() const { return _pixmap; }
    const QRectF& sel_rect() const { return _sel_rect; }

    void set_pixmap(const QPixmap& pixmap, const QRectF& place);

  private:
    bool drag_frame;
    QPen _pen;
    QRectF _sel_rect;
    QRectF _stashed_rect;
    QPixmap _pixmap;
};

// KromitUndoPixmap
//
class KromitUndoPixmap : public KromitUndoItem {
  public:
    explicit KromitUndoPixmap(const KromitPixmap* kpath);

    virtual KromitItem* create_from_undo_item() const override;

    QRectF _rect;
    QPixmap _pixmap;
};

#endif  // KROMITPIXMAP_H
