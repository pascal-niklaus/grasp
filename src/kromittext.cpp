#include "kromittext.h"
#include "string_format.h"
#include "utility.h"

#include <QDebug>
#include <QPainter>
#include <QFontDatabase>
#include <QFontMetrics>

// KromitText::KromitText
//
KromitText::KromitText(QGraphicsItem* parent,
                       const QColor& color,
                       double size) :
    KromitItem(parent) {
    _size = size;
    _color = color;
    _crsr_pos = 0;
    _font = QFont("Noto Sans", size);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

KromitText::~KromitText() {}


// KromitText::create_undo_item
//
KromitUndoItem* KromitText::create_undo_item() const {
    auto* undo = new KromitUndoText(this);
    return undo;
}

// KromitText::stash
//
// store pre-transformation state
//
void KromitText::stash() {
    _stashed_path = _path;
}

// KromitText::apply
//
// apply 2D affine transformation
//
void KromitText::apply(const QTransform& m) {
    prepareGeometryChange();

    set_path(m.map(_stashed_path));
}

// KromitText::start
//
void KromitText::start(const QPointF& p, double pressure) {
    _location = p;
    build_path(true);
}

// KromitText::stop
//
void KromitText::stop(const QPointF& p, double pressure) {
    _location = p;
    build_path(true);
}

// KromitText::move
//
void KromitText::move(const QPointF& p, double pressure) {
    _location = p;
    build_path(true);
}

KromitItem::KeyResult KromitText::keystroke(QKeyCombination key,
                                            const QString& text) {
    if (key.key() == Qt::Key_Escape) {
        _text.clear();
        build_path(false);
        return KEY_STOP;
    }

    if (key == QKeyCombination(Qt::NoModifier | Qt::Key_Return)) {
        build_path(false);
        return KEY_STOP;
    }

    if (key.key() == Qt::Key_Plus &&
        key.keyboardModifiers() & Qt::ControlModifier) {
        _font.setPointSizeF(_font.pointSizeF() * 1.333);
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_Minus &&
        key.keyboardModifiers() & Qt::ControlModifier) {
        _font.setPointSizeF(_font.pointSizeF() / 1.333);
        build_path(true);
        return KEY_PROCESSED;
    }

    if (key.key() == Qt::Key_Backspace) {
        if (_crsr_pos > 0) {
            _text.remove(_crsr_pos - 1, 1);
            _crsr_pos--;
        }
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_Home) {
        _crsr_pos = 0;
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_End) {
        _crsr_pos = _text.length();
        build_path(true);
        return KEY_PROCESSED;
    }
    if (key.key() == Qt::Key_Delete) {
        if (_crsr_pos < static_cast<int>(_text.length())) {
            _text.remove(_crsr_pos, 1);
        }
        build_path(true);
        return KEY_PROCESSED;
    }

    if (key.key() == Qt::Key_Left) {
        // move cursor left
        _crsr_pos = cap<int>(_crsr_pos - 1, 0, _text.length());
        build_path(true);
    };
    if (key.key() == Qt::Key_Right) {
        // move cursor right
        _crsr_pos = cap<int>(_crsr_pos + 1, 0, _text.length());
        build_path(true);
    };

    if (text.length() == 1) {
        const auto ch = text.front();
        if (ch > 0 && ch < ' ' && ch != QChar(13)) return KEY_IGNORED;

        _text.insert(_crsr_pos, text);
        _crsr_pos += text.length();

        build_path(true);
        return KEY_PROCESSED;
    }

    return KEY_IGNORED;
}

void KromitText::build_path(bool show_cursor) {
    prepareGeometryChange();

    QStringList lines = _text.split(QChar(13));

    _path.clear();
    QFontMetrics fm(_font);

    int y = _location.y();
    int crsr_in_line = _crsr_pos;
    for (const auto& line : lines) {
        _path.addText(
            _location.x(), y, QFont(_font), line);

        if (show_cursor && crsr_in_line >= 0 && crsr_in_line <= static_cast<int>(line.length())) {
            const int crsrx =
                _location.x() +
              fm.horizontalAdvance(line, crsr_in_line);
            const int crsry0 = y + fm.descent();
            const int crsry1 = y - fm.ascent();

            const int dx = 4;
            const int dy = 2;
            _path.moveTo(crsrx, crsry0);
            _path.lineTo(crsrx + dx, crsry0 + dy);
            _path.lineTo(crsrx, crsry0);
            _path.lineTo(crsrx - dx, crsry0 + dy);
            _path.lineTo(crsrx, crsry0);
            _path.lineTo(crsrx, crsry1);
            _path.lineTo(crsrx - dx, crsry1 - dy);
            _path.lineTo(crsrx, crsry1);
            _path.lineTo(crsrx + dx, crsry1 - dy);
            _path.lineTo(crsrx, crsry1);
        }

        y += fm.height();
        crsr_in_line -= (1 + line.length());
    }

}

// KromitText::outline
//
// create a path outlining the path at given distance
//
QPainterPath KromitText::outline(double dist) const {
    QPainterPathStroker ps;
    ps.setWidth(dist);
    ps.setCapStyle(Qt::RoundCap);
    ps.setJoinStyle(Qt::RoundJoin);
    return ps.createStroke(_path);
}

// KromitText::shape
//
// selectable area
//
QPainterPath KromitText::shape() const {
    return outline(20);
}

// KromitText::boundingRect
//
// this area is re-painted
//
QRectF KromitText::boundingRect() const {
    const double delta = 35;
    QRectF rect = _path.boundingRect();
    rect.adjust(-delta, -delta, delta, delta);
    return rect;
}

// KromitText::paint
//
void KromitText::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget) {
    if (isSelected()) {
        painter->setPen(QPen(Qt::gray, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawPath(outline(15));
    }
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(_color, 0));
    painter->setBrush(_color);
    painter->drawPath(_path);
}

// KromitText::set_path
//
// Set the path, recreating PainterPath
//
void KromitText::set_path(const QPainterPath& path) {
    prepareGeometryChange();
    _path = path;
}

// KromitText::dump
//
void KromitText::dump(std::ostream& os) const {
}

void KromitText::dump_fonts() {
    auto fams = QFontDatabase::families();
    qDebug() << fams;
}

// ============================== undo ===============================

// KromitUndoText::KromitUndoText
//
KromitUndoText::KromitUndoText(const KromitText* ktext) :
    KromitUndoItem(static_cast<const KromitItem*>(ktext)) {
    color = ktext->color();
    size = ktext->size();
    path = ktext->path();
}

KromitUndoText::~KromitUndoText() {}

// KromitUndoText::create_from_undo_item
//
KromitItem* KromitUndoText::create_from_undo_item() const {
    KromitText* ktext =
        new KromitText(nullptr, color, size);
    ktext->setData(KromitText::KeyID, id);
    ktext->set_path(path);
    return ktext;
}
