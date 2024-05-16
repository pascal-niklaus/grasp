#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

#include "tools.h"
#include "colors.h"
#include "selectcolor.h"

class DoubleEdit;
class IntEdit;
class SelectKey;
class SelectFont;
class QSettings;
class QStatusBar;
class QLabel;
class QLineEdit;
class QDialogButtonBox;
// ----------------------------- ToolTab -----------------------------

class ToolTab : public QWidget {
    Q_OBJECT
  public:
    ToolTab(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void load_options(const Settings &settings);
    void save_options(Settings &settings) const;
  private slots:
    void slot_status_message(const QString &msg);

  private:
    QStatusBar *_status = nullptr;
    SelectKey *_edit_key;
    SelectKey *_pen_key;
    SelectKey *_line_key;
    SelectKey *_rect_key;
    SelectKey *_smooth_key;
    SelectKey *_ortho_key;
    SelectKey *_highlight_key;
    SelectKey *_fader_key;
    SelectKey *_text_key;
    SelectKey *_grab_key;
    SelectKey *_grab_partial_key;
    SelectKey *_delete_key;
    SelectKey *_delete_all_key;
    SelectKey *_dash_toggle_key;

    DoubleEdit *_smooth_simplify;
    DoubleEdit *_smooth_snap;
    DoubleEdit *_rect_radius;
    DoubleEdit *_ortho_simplify;
    DoubleEdit *_ortho_snap;
    DoubleEdit *_ortho_maxangle;
    DoubleEdit *_ortho_minlen;
    DoubleEdit *_ortho_radius;
    DoubleEdit *_highlight_sustain;
    DoubleEdit *_highlight_fade;
    DoubleEdit *_fader_sustain;
    DoubleEdit *_fader_fade;
    DoubleEdit *_grab_delay;
    SelectFont *_font;
};

// -------------------------- LinewidthTab ---------------------------

class LinewidthTab : public QWidget {
    Q_OBJECT
  public:
    LinewidthTab(QWidget *parent = nullptr,
                 Qt::WindowFlags f = Qt::WindowFlags());
    void load_options(const Settings &settings);
    void save_options(Settings &settings) const;

  private:
    IntEdit *_width[9];
    IntEdit *_highlighter_width[9];
    SelectKey *_width_key[9];
    SelectKey *_arrow_key[4];
    DoubleEdit *_arrowsize;
};

// ----------------------------- AuxTab ------------------------------

class AuxTab : public QWidget {
    Q_OBJECT
  public:
    AuxTab(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void load_options(const Settings &settings);
    void save_options(Settings &settings) const;
  private slots:
    void slot_browse_clicked();

  private:
    void set_path(const QString &path);

  private:
    SelectKey *_quit_key;
    SelectKey *_undo_key;
    SelectKey *_redo_key;
    SelectKey *_toggle_key;
    SelectKey *_opacity_key;

    QString _path;
    QLineEdit *_path_label;
    QPushButton *_browse;
};

// ---------------------------- ColorTab -----------------------------

class ColorTab : public QWidget {
    Q_OBJECT
  public:
    ColorTab(int ncolors,
             QWidget *parent = nullptr,
             Qt::WindowFlags f = Qt::WindowFlags());
    void load_options(const Settings &settings);
    void save_options(Settings &settings) const;
  private slots:
    void slot_status_message(const QString &msg);

  private:
    int _ncolors;
    QStatusBar *_status = nullptr;

    std::vector<SelectColorButton *> _colbtn;
    std::vector<SelectKey *> _keyseq1;
    std::vector<SelectKey *> _keyseq2;
};

// ------------------------- SettingsDialog --------------------------

class SettingsDialog : public QDialog {
    Q_OBJECT
  public:
    SettingsDialog(Settings &settings,
                   QWidget *parent = nullptr,
                   Qt::WindowFlags f = Qt::WindowFlags());
    ~SettingsDialog() = default;

  private slots:
    void slot_accept();
    void slot_reject();
    void slot_clicked(QAbstractButton *button);

  private:
    void load_all();
    void save_all();

  protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

  private:
    QDialogButtonBox *_bbox;
    Settings& _settings;
    ToolTab *_tooltab;
    LinewidthTab *_linewidthtab;
    AuxTab *_auxtab;
    ColorTab *_colortab;
  //    decltype(Settings::color_table) _old_color_tab;
};

#endif
