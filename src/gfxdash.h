/*
 * Grasp -- a program to GRAb the screen and Sketch on top during Presentations
 *
 * Copyright (C) 2024 Pascal Niklaus <pascal@niklaus.org>
 *
 * This file is part of 'Grasp'.
 *
 * 'Grasp' is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'Grasp' is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'Grasp'. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef GFXDASH_H
#define GFXDASH_H

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

#include "grasp.h"

class Settings;

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

class GfxDashIcon : public DashIcon {
  public:
    enum { Type = GFX_DASHICON };

  public:
    GfxDashIcon(QGraphicsItem* parent, int size, const QPixmap& px, int id);
    ~GfxDashIcon();

    void set_pixmap(int id, const QPixmap& pixmap);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override { return QRectF(); };
    virtual QRectF selectionBox() const override;

  private:
    double _size;
    QGraphicsPixmapItem* _icon = nullptr;
};

// ============================= NUMBER ==============================

class GfxNumber : public DashIcon {
  public:
    enum { Type = GFX_SCENE };

  public:
    GfxNumber(QGraphicsItem* parent = nullptr, int size = 30);
    ~GfxNumber();

    void set_id(int id);
    void set_selected(bool selected);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override { return QRectF(); };
    virtual QRectF selectionBox() const override;

  private:
    QGraphicsTextItem* _text = nullptr;
    QGraphicsPolygonItem* _poly = nullptr;
    int _id;
    bool _is_active;
    int _size;
};

// ============================= COLOR ==============================

class GfxColor : public DashIcon {
  public:
    enum { Type = GFX_COLOR };

  public:
    GfxColor(QGraphicsItem* parent = nullptr, int size = 30);
    ~GfxColor();

    void set_color(const QColor& color, const QString& shortcut);
    void set_selected(bool selected);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override { return QRectF(); };
    virtual QRectF selectionBox() const override;

  private:
    QGraphicsTextItem* _text = nullptr;
    QGraphicsPolygonItem* _poly = nullptr;
    std::uint32_t _color;
};

// ============================= ARROW ==============================

class GfxArrowIcon : public DashIcon {
  public:
    enum { Type = GFX_ARROWICON };

  public:
    GfxArrowIcon(QGraphicsItem* parent = nullptr, int size = 30);
    ~GfxArrowIcon();

    void set_type(ArrowType type, bool selected = false);
    void set_selected(bool selected);

    // ----- QGraphicsItem methods
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;
    virtual int type() const override { return Type; }
    virtual QRectF boundingRect() const override { return QRectF(); };
    virtual QRectF selectionBox() const override;

  private:
    QGraphicsPolygonItem* _left = nullptr;
    QGraphicsPolygonItem* _right = nullptr;
    QGraphicsPolygonItem* _bar = nullptr;
    QGraphicsPolygonItem* _box = nullptr;

    ArrowType _type;
    bool _is_active;
};

// ============================== DASH ===============================

class GfxDash : public QGraphicsItem {
  public:
    enum { Type = GFX_DASH };

  public:
    GfxDash(QGraphicsItem* parent, int width, int height, Settings& settings);
    ~GfxDash();

    void reposition(int width);
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
    virtual QRectF boundingRect() const override { return QRectF(); };

  private:
    void reposition_scenes();

  private:
    int _height;
    int _width;
    int _active_scene = 0;
    Settings& _settings;

    GfxDashIcon* _cogwheel;
    std::vector<GfxColor*> _colors;
    std::vector<GfxArrowIcon*> _arrows;

    std::map<int, GfxNumber*> _scenes;
};

#endif
