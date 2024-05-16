#include "gfxdash.h"

#include <QPainter>
#include <QPen>

#include "colors.h"
#include "gfxitem.h"
#include "tools.h"

// ============================= NUMBER ==============================
//
// GfxNumber
//
// A marker with a single number indicating the scene

GfxNumber::GfxNumber(QGraphicsItem *parent, int size) : DashIcon(parent) {
    _size = size / 2;

    _poly = new QGraphicsPolygonItem(this);

    QPolygonF rect;
    rect << QPointF(_size, _size) << QPointF(-_size, _size)
         << QPointF(-_size, -_size) << QPointF(_size, -_size);

    _poly->setPolygon(rect);
    _poly->setPen(QPen(Qt::gray, 1));
    _poly->setPos(0, 0);
    _poly->setZValue(999);

    _id = 0;
    _is_active = false;

    _text = new QGraphicsTextItem(this);
    _text->setFont(QFont("Noto Sans", 12, QFont::Black));
    _text->setDefaultTextColor(Qt::gray);
    _text->setZValue(100);

    set_id(0);
}

GfxNumber::~GfxNumber() {}

void GfxNumber::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget) {}

QRectF GfxNumber::selectionBox() const {
    return _poly->polygon().boundingRect().adjusted(-5, -5, 10, 10);
}

void GfxNumber::set_id(int id) {
    _text->setFont(QFont("Noto Sans", 10, QFont::Normal));
    _poly->setPen(QPen(Qt::gray, 1));
    _text->setPlainText(QString("%1").arg(id));
    _text->adjustSize();
    QRectF r = _text->boundingRect();
    _text->setPos(-r.center());

    // store scene ID in item
    setData(GfxItem::KeyDash, DashScene);
    setData(GfxItem::KeyDashAux, id);
}

void GfxNumber::set_selected(bool selected) {
    _is_active = selected;

    if (_is_active) {
        _text->setFont(QFont("Noto Sans", 12, QFont::Black));
        _poly->setPen(QPen(Qt::gray, 3));
    } else {
        _text->setFont(QFont("Noto Sans", 10, QFont::Normal));
        _poly->setPen(QPen(Qt::gray, 1));
    }
    QRectF r = _text->boundingRect();
    _text->setPos(-r.center());
}

// ============================== ICON ===============================
//
// GfxDashIcon
//
// A pixmap

GfxDashIcon::GfxDashIcon(QGraphicsItem *parent,
                         int size,
                         const QPixmap &px,
                         int id) :
    DashIcon(parent) {
    _size = size / 2.0;

    _icon = new QGraphicsPixmapItem(this);
    _icon->setPixmap(px);
    _icon->setPos(-_size, -_size);
    _icon->setScale(static_cast<double>(size) / px.height());

    setData(GfxItem::KeyDash, DashPixmap);
    setData(GfxItem::KeyDashAux, id);
}

GfxDashIcon::~GfxDashIcon() {}

void GfxDashIcon::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget) {}

QRectF GfxDashIcon::selectionBox() const {
    return _icon->boundingRect().adjusted(-25, -25, 25, 25);
}

// ============================= ARROW ==============================
//
// GfxArrowIcon
//
// A marker with a single number

GfxArrowIcon::GfxArrowIcon(QGraphicsItem *parent, int size) : DashIcon(parent) {
    size = size / 2;

    _box = new QGraphicsPolygonItem(this);

    constexpr double kx = 1.4;

    QPolygonF rect;
    rect << QPointF(kx * size, size) << QPointF(-kx * size, size)
         << QPointF(-kx * size, -size) << QPointF(kx * size, -size);

    _box->setPolygon(rect);
    _box->setPen(QPen(Qt::gray, 1));
    _box->setPos(0, 0);
    _box->setZValue(999);

    _type = ARROW_NONE;
    _is_active = false;

    QPolygonF bar;
    const double hsize = size * 1;
    const double vsize = size * 0.17;
    bar << QPointF(-hsize, -vsize) << QPointF(hsize, -vsize)
        << QPointF(hsize, vsize) << QPointF(-hsize, vsize);

    _bar = new QGraphicsPolygonItem(this);
    _bar->setPos(0, 0);
    _bar->setPen(QPen(Qt::black, 0));
    _bar->setBrush(Qt::gray);
    _bar->setZValue(999);
    _bar->setPolygon(bar);

    QPolygonF head;
    head << QPointF(-hsize * 1.2, 0) << QPointF(-hsize * 0.4, -vsize * 4)
         << QPointF(-hsize * 0.5, 0) << QPointF(-hsize * 0.4, vsize * 4);
    _left = new QGraphicsPolygonItem(this);
    _left->setPen(QPen(Qt::black, 0));
    _left->setBrush(Qt::gray);
    _left->setPos(0, 0);
    _left->setZValue(999);
    _left->setPolygon(head);

    _right = new QGraphicsPolygonItem(this);
    _right->setPen(QPen(Qt::black, 0));
    _right->setBrush(Qt::gray);
    _right->setPos(0, 0);
    _right->setRotation(180);
    _right->setZValue(999);
    _right->setPolygon(head);
}

GfxArrowIcon::~GfxArrowIcon() {}

void GfxArrowIcon::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *option,
                         QWidget *widget) {
    // draw scene number, and fill if selected (current scene)
}

QRectF GfxArrowIcon::selectionBox() const {
    return _box->polygon().boundingRect().adjusted(-5, -5, 10, 10);
}

void GfxArrowIcon::set_type(ArrowType type, bool selected) {
    auto colr = selected ? Qt::white : Qt::gray;

    _bar->setBrush(colr);
    _left->setBrush(colr);
    _right->setBrush(colr);
    _box->setPen(QPen(colr, 1));
    if (type & ARROW_START)
        _left->show();
    else
        _left->hide();
    if (type & ARROW_END)
        _right->show();
    else
        _right->hide();

    setData(GfxItem::KeyDash, DashArrow);
    setData(GfxItem::KeyDashAux, static_cast<std::uint32_t>(type));
}

void GfxArrowIcon::set_selected(bool selected) {
    _box->setPen(QPen(selected ? Qt::white : Qt::gray, selected ? 3 : 1));
}

// ============================== COLOR ==============================

// GfxColor
//
// A square with a color

GfxColor::GfxColor(QGraphicsItem *parent, int size) : DashIcon(parent) {
    size /= 2;

    _poly = new QGraphicsPolygonItem(this);

    constexpr double kx = 1.2;

    QPolygonF rect;
    rect << QPointF(kx * size, size) << QPointF(-kx * size, size)
         << QPointF(-kx * size, -size) << QPointF(kx * size, -size);

    _poly->setPolygon(rect);
    _poly->setPen(QPen(Qt::gray, 1));
    _poly->setBrush(Qt::green);
    _poly->setPos(0, 0);
    _poly->setZValue(999);

    _text = new QGraphicsTextItem(this);
    _text->setFont(QFont("Noto Sans", 12, QFont::Black));
    _text->setDefaultTextColor(Qt::black);
    _text->setZValue(1000);
}

GfxColor::~GfxColor() {}

void GfxColor::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {}

QRectF GfxColor::selectionBox() const {
    return _poly->polygon().boundingRect().adjusted(-5, -5, 10, 10);
}

void GfxColor::set_color(const QColor &color, const QString &shortcut) {
    _text->setPlainText(shortcut);
    _text->adjustSize();
    QRectF r = _text->boundingRect();
    _text->setPos(-r.center());
    _poly->setBrush(color);

    setData(GfxItem::KeyDash, DashCol);
    setData(GfxItem::KeyDashAux, Color::uint32_from_qcolor(color));
}

void GfxColor::set_selected(bool selected) {
    _poly->setPen(QPen(selected ? Qt::white : Qt::gray, selected ? 3 : 1));
}

// ============================== DASH ===============================

// GfxDash
//
// The entire dashboard

GfxDash::GfxDash(QGraphicsItem *parent,
                 int width,
                 int height,
                 Settings &settings) :
    QGraphicsItem(parent), _settings(settings) {
    _height = height;
    _width = width;
    int x = width - height;

    // setup (cogwheel)

    auto px = QPixmap(":/icons/icons/settings.png");

    _cogwheel = new GfxDashIcon(this, height, px, 1);
    _cogwheel->setPos(x, 2.0 + static_cast<double>(height) / 2.0);
    _cogwheel->setData(GfxItem::KeyDash, DashIcon::DashSetup);
    _cogwheel->setData(GfxItem::KeyDashAux, 0);

    // add color boxes, right to left
    for (auto it = _settings.color_table.crbegin();
         it != _settings.color_table.crend();
         it++) {
        auto *icon = new GfxColor(this, height);
        _colors.emplace_back(icon);
        icon->setPos(x, 2.0 + static_cast<double>(height) / 2.0);
        QString txt;
        if (it->key1.keyboardModifiers() == Qt::ShiftModifier) {
            txt = QString("\u21e7%1").arg(QChar(it->key1.key()).toUpper());
        } else {
            txt = QString("%1").arg(QChar(it->key1.key()).toUpper());
        }
        icon->set_color(it->color, txt);
    }

    // arrow icons
    for (int i = 0; i < 4; i++) {
        auto *icon = new GfxArrowIcon(this, height);
        _arrows.emplace_back(icon);
        icon->setPos(x, 2.0 + static_cast<double>(height) / 2.0);
        ArrowType type = ARROW_NONE;
        switch (i) {
            case 0:
                type = ARROW_DOUBLE;
                break;
            case 1:
                type = ARROW_END;
                break;
            case 2:
                type = ARROW_NONE;
                break;
            case 3:
                type = ARROW_START;
                break;
        }
        icon->set_type(type);
    }

    reposition(width);
    // scene numbers
}

GfxDash::~GfxDash() {}

void GfxDash::reposition(int width) {
    _width = width;
    int x = _width - _height;
    const int y = 2.0 + static_cast<double>(_height) / 2.0;

    _cogwheel->setPos(x,y);

    x -= _height * 1.5 * 2;
    for (auto &c : _colors) {
        c->setPos(x, y);
        x -= _height * 1.5;
    }

    x -= _height * 1.5;
    for (auto &a : _arrows) {
        a->setPos(x, y);
        x -= _height * 1.5;
    }
}

void GfxDash::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) {}

void GfxDash::select_color(const QColor &color) {
    std::uint32_t numeric_color = Color::uint32_from_qcolor(color);
    for (auto &item : childItems()) {
        if (item->type() == GFX_COLOR) {
            auto *citem = static_cast<GfxColor *>(item);
            citem->set_selected(numeric_color ==
                                item->data(GfxItem::KeyDashAux).toUInt());
        }
    }
};

void GfxDash::select_arrow(ArrowType type) {
    for (auto &item : childItems()) {
        if (item->type() == GFX_ARROWICON) {
            auto *aitem = static_cast<GfxArrowIcon *>(item);
            aitem->set_selected(type ==
                                item->data(GfxItem::KeyDashAux).toUInt());
        }
    }
};

void GfxDash::select_tool(ToolType type){
    // TODO
};

void GfxDash::select_scene(int n){
  _active_scene = n;
  reposition_scenes();
};

void GfxDash::add_scene(int id) {
    if (_scenes.find(id) == _scenes.end()) {
        GfxNumber *new_scene = new GfxNumber(this, _height);
        new_scene->set_id(id);
        _scenes.insert({id, new_scene});
        reposition_scenes();
    }
}

void GfxDash::remove_scene(int id) {
    auto it = _scenes.find(id);
    if (it != _scenes.end()) {
        prepareGeometryChange();
        delete it->second;
        _scenes.erase(id);
        reposition_scenes();
    }
}

void GfxDash::clear_scenes() {
    while (_scenes.size()) {
        prepareGeometryChange();
        delete _scenes.begin()->second;
    }
}

void GfxDash::reposition_scenes() {
    int x = _height * 4;
    for (auto &item : _scenes) {
        item.second->setPos(x, _height);
        item.second->set_selected(item.first == _active_scene);
        x += 1.5 * _height;

        // also maybe highlight active scene???
    }
}
