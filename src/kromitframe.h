#ifndef KROMITFRAME_H
#define KROMITFRAME_H

#include <qpoint.h>
#include <QGraphicsItem>
#include <QPointF>
#include <QPen>
#include <QColor>

#include "kromit.h"
#include "kromititem.h"
#include "kromithandle.h"
#include "kromitmove.h"
#include "kromitrotate.h"

// KromitFrame
//
// a selection frame
//

class KromitFrame : public KromitItem {
  public:
    enum { Type = GFX_SELFRAME };
    enum HandlePos : std::uint16_t {
        // type of handle
        POS_ISOSCALE = 0x10,
        POS_ANISOSCALE = 0x20,
        POS_ROTATE = 0x40,
        POS_MOVE = 0x80,
        // corner/side designations
        POS_TOPLEFT = 0x01,
        POS_TOPRIGHT = 0x02,
        POS_BOTTOMRIGHT = 0x03,
        POS_BOTTOMLEFT = 0x04,
        POS_TOP = 0x01,
        POS_RIGHT = 0x02,
        POS_BOTTOM = 0x03,
        POS_LEFT = 0x04,
        POS_CENTER = 0x05,
    };

  public:
    KromitFrame(QGraphicsItem* parent = nullptr,
                const QColor& color = Qt::yellow,
                double width = 1);
    ~KromitFrame();

    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

    // these methods are not needed for a frame, but must be implemented
    virtual QRectF selectionBox() const override { return QRectF(); }
    virtual TransformCaps caps() const override { return CAP_NONE; }

    void setCaps(TransformCaps caps);
    void setRect(const QPointF& p1, const QPointF& p2);

    virtual void start(const QPointF& p, double pressure = 1.0) override;
    virtual void move(const QPointF& p, double pressure = 1.0) override;
    virtual void stop(const QPointF& p, double pressure = 1.0) override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

private:
  // handle positions, assuming p1 is topleft and p2 is topright
  QPointF center() {
      return QPointF(0.5 * (_p1.x() + _p2.x()), 0.5 * (_p1.y() + _p2.y()));
  }
  QPointF topleft() { return _p1; }
  QPointF bottomright() { return _p2; }
  QPointF topright() { return QPointF(_p2.x(), _p1.y()); }
  QPointF bottomleft() { return QPointF(_p1.x(), _p2.y()); }
  QPointF midtop() { return QPointF(0.5 * (_p1.x() + _p2.x()), _p1.y()); }
  QPointF midbottom() { return QPointF(0.5 * (_p1.x() + _p2.x()), _p2.y()); }
  QPointF midleft() { return QPointF(_p1.x(), 0.5 * (_p1.y() + _p2.y())); }
  QPointF midright() { return QPointF(_p2.x(), 0.5 * (_p1.y() + _p2.y())); }

private:
  QPen _pen;
  QPointF _p1, _p2;

  KromitMove* _move_handle = nullptr;
  KromitRotHandle* _rot_handle[4] = {nullptr, nullptr, nullptr, nullptr};
  KromitHandle* _iso_handle[4] = {nullptr, nullptr, nullptr, nullptr};
  KromitHandle* _aniso_handle[4] = {nullptr, nullptr, nullptr, nullptr};
};

#endif  // KROMITFRAMEH_H
