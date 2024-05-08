#ifndef KROMITTEXT_H
#define KROMITTEXT_H

#include "kromit.h"
#include "arrowitem.h"
#include <QPen>
#include <QFont>

class KromitUndoText;

// KromitText
//
class KromitText : public KromitItem {
  public:
    enum { Type = GFX_TEXT };

  public:
    KromitText(QGraphicsItem* parent = nullptr,
               const QColor& color = Qt::yellow,
               double size = 12);
    ~KromitText();

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
        return (CAP_MOVE | CAP_ANISOSCALE | CAP_ISOSCALE | CAP_DELETE |
                CAP_ROTATE | CAP_KEYSTROKES);
    }

    // - build system
    virtual void start(const QPointF& p, double pressure = 1.0) override;
    virtual void move(const QPointF& p, double pressure = 1.0) override;
    virtual void stop(const QPointF& p, double pressure = 1.0) override;
    virtual KeyResult keystroke(QKeyCombination key,
                                const QString& text) override;

    // - debugging
    virtual void dump(std::ostream& os) const override;

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual QPainterPath shape() const override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override;

    void feed_char(const QChar ch);
    void finalize() { build_path(false); }

    static void dump_fonts(void);

  public:
    // ----- item accessors
    QColor color() const { return _color; }
    double size() const { return _size; }
    const QString& text() const { return _text; }
    const QPainterPath& path() const { return _path; }
    void set_size(double size);
    void set_path(const QPainterPath& path);

  private:
    // ----- internal helpers
    QPainterPath outline(double dist) const;
    void build_path(bool show_cursor);

  private:
    QColor _color;
    QFont _font;
    double _size;
    QString _text;
    int _crsr_pos;

    QPointF _location;
    QPainterPath _path;
    QPainterPath _stashed_path;
};

// KromitUndoText
//
class KromitUndoText : public KromitUndoItem {
  public:
    explicit KromitUndoText(const KromitText* kpath);
    virtual ~KromitUndoText();

    virtual KromitItem* create_from_undo_item() const override;

    QColor color;
    double size;
    QPainterPath path;
};

#endif  // KROMITTEXT_H
