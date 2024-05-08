#ifndef KROMITDASH_H
#define KROMITDASH_H

#include <set>

/*
 * The dashboard contains:
 *
 * - little numbers in boxes for the different scenes
 * - a new scene is started with Ctrl-Shift-N (continuously  increasing ID)
 * - the CONTENT of the current scene is deleted with Ctrl-Shift-Del
 * - RMB on non-active scene, with menu can be used to delete the stored scene
 * - RMB save ??

 * - color icons
 * - arrow icons
 * - tool icons
 *
 * - setup icon (cogwheel)
 *
 */

#include "kromit.h"

class DashIcon : public QGraphicsItem {
  public:
    DashIcon(QGraphicsItem* parent = nullptr) : QGraphicsItem(parent) {}
    enum : std::uint32_t {
        DashCol = 0x10000000,
        DashArrow = 0x20000000,
        DashTool = 0x30000000,
        DashScene = 0x40000000,
        DashSetup = 0x50000000,
        DashPixmap = 0x60000000,
        DashMask = 0xf0000000
    };
    virtual QRectF selectionBox() const = 0;

    virtual bool contains(const QPointF& point) const override {
        return selectionBox().contains(point);
    };
};

// ============================= NUMBER ==============================

class KromitDashIcon : public DashIcon {
  public:
    enum { Type = GFX_DASHICON };

  public:
    KromitDashIcon(QGraphicsItem* parent, int size, const QPixmap& px, int id);
    ~KromitDashIcon();

    void set_pixmap(int id, const QPixmap& pixmap);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const { return QRectF(); };
    virtual QRectF selectionBox() const;

  private:
    double _size;
    QGraphicsPixmapItem* _icon = nullptr;
};

// ============================= NUMBER ==============================

class KromitScene : public DashIcon {
  public:
    enum { Type = GFX_SCENE };

  public:
    KromitScene(QGraphicsItem* parent = nullptr, int size = 30);
    ~KromitScene();

    void set_id(int id);
    void set_selected(bool selected);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const { return QRectF(); };
    virtual QRectF selectionBox() const;

  private:
    QGraphicsTextItem* _text = nullptr;
    QGraphicsPolygonItem* _poly = nullptr;
    int _id;
    bool _is_active;
    int _size;
};

// ============================= COLOR ==============================

class KromitColor : public DashIcon {
  public:
    enum { Type = GFX_COLOR };

  public:
    KromitColor(QGraphicsItem* parent = nullptr, int size = 30);
    ~KromitColor();

    void set_color(std::uint32_t color, const QString& shortcut);
    void set_selected(bool selected);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const { return QRectF(); };
    virtual QRectF selectionBox() const;

  private:
    QGraphicsTextItem* _text = nullptr;
    QGraphicsPolygonItem* _poly = nullptr;
    std::uint32_t _color;
};

// ============================= ARROW ==============================

class KromitArrowIcon : public DashIcon {
  public:
    enum { Type = GFX_ARROWICON };

  public:
    KromitArrowIcon(QGraphicsItem* parent = nullptr, int size = 30);
    ~KromitArrowIcon();

    void set_type(ArrowType type, bool selected = false);
    void set_selected(bool selected);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const { return QRectF(); };
    virtual QRectF selectionBox() const;

  private:
    QGraphicsPolygonItem* _left = nullptr;
    QGraphicsPolygonItem* _right = nullptr;
    QGraphicsPolygonItem* _bar = nullptr;
    QGraphicsPolygonItem* _box = nullptr;

    ArrowType _type;
    bool _is_active;
};

// ============================== DASH ===============================

class KromitDash : public QGraphicsItem {
  public:
    enum { Type = GFX_DASH };

  public:
    KromitDash(QGraphicsItem* parent, int width, int height);
    ~KromitDash();

    void select_color(const QColor& color);
    void select_arrow(ArrowType type);
    void select_tool(ToolType type);
    void select_scene(int n);

    void add_scene(int id);
    void remove_scene(int id);
    void clear_scenes();

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const { return QRectF(); };

  private:
    void reposition_scenes();

  private:
    int _height;
    int _width;
    int _active_scene = 0;

    std::map<int, KromitScene*> _scenes;
};

#endif
