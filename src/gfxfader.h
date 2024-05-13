#ifndef GFXFADER_H
#define GFXFADER_H

#include "grasp.h"
#include "gfxpath.h"
#include "path.h"

#include <QPen>

class GfxUndoFader;

// GfxFader
//
class GfxFader : public GfxPath {
  public:
    enum { Type = GFX_FADER };

  public:
    GfxFader(QGraphicsItem* parent = nullptr,
                const QColor& color = Qt::yellow,
                double width = 1,
                ArrowType atype = ARROW_NONE,
                double arrowsize = 0.0,
                double sustain = 5.0,
                double fade = 5.0);
    ~GfxFader();

    // ----- GfxItem methods

    // - undo system
    virtual GfxUndoItem* create_undo_item() const override;

    // - transform system
    virtual TransformCaps caps() const override {
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_ROTATE |
                CAP_DYNAMIC | CAP_DELETE);
    }

    // - build system
    virtual void start(const QPointF& p, double pressure = 1.0) override;
    virtual void move(const QPointF& p, double pressure = 1.0) override;
    virtual void stop(const QPointF& p, double pressure = 1.0) override;

    // - debugging
    virtual void dump(std::ostream& os) const override;

    virtual DynResult dynamic() override;

  public:
    // ----- item accessors
    std::chrono::steady_clock::time_point sustained() const {
        return _sustained;
    }
    std::chrono::steady_clock::time_point faded() const {
        return _faded;
    }
    void set_sustained(std::chrono::steady_clock::time_point val) {
        _sustained = val;
    }
    void set_faded(std::chrono::steady_clock::time_point val) { _faded = val; }

  private:
    // ----- internal helpers
    void update_time();

  private:
    double _sustain_secs;
    double _fading_secs;
    std::chrono::steady_clock::time_point _sustained;
    std::chrono::steady_clock::time_point _faded;
};

// GfxUndoFader
//
class GfxUndoFader : public GfxUndoItem {
  public:
    explicit GfxUndoFader(const GfxFader* kpath);

    virtual GfxItem* create_from_undo_item() const override;

    QPen pen;
    ArrowType arrow_type;
    double arrow_size;
    Path xy_path;
    std::chrono::steady_clock::time_point sustained;
    std::chrono::steady_clock::time_point faded;
};

#endif  // GFXFADER_H
