#include "selectkey.h"

#include <QDebug>
#include <QEvent>

SelectKey::SelectKey(QWidget *parent) : QKeySequenceEdit(parent) {
    connect(this,
            &SelectKey::editingFinished,
            this,
            &SelectKey::slot_check_key_combi);
}

void SelectKey::set_key(const QKeyCombination &keycomb) {
    _combi = _prev_combi = keycomb;
    setKeySequence(keycomb);
}

void SelectKey::set_del_key(const QKeyCombination &delcomb) {
    _delete_key = delcomb;
}

void SelectKey::set_allowed_modifiers(Qt::KeyboardModifiers mods) {
    _mods = mods;
}

void SelectKey::slot_check_key_combi() {
    auto seq = keySequence();

    if (seq.count() > 1) {
        emit status_message("Using first key stroke only !");
        set_key(seq[0]);
    }

    if (seq.count() == 0 || seq[0] == _delete_key) {
        _combi = QKeyCombination();
        set_key(_combi);
        return;
    }

    const auto m = seq[0].keyboardModifiers();
    if (m && (_mods & m) != m) {
        _combi = QKeyCombination();
        set_key(_combi);
        emit status_message("Illegal modifier key used !");
        return;
    }

    _combi = seq[0];
}

bool SelectKey::event(QEvent *ev) {
    auto t = ev->type();
    switch (t) {
        case QEvent::FocusIn:
            clear();
            break;
        case QEvent::Timer:
            clearFocus();
            break;
        default:
            break;
    }
    return QKeySequenceEdit::event(ev);
}

// QKeyCombination SelectKey::canonicalize
//
// Attempt to work around keyboard layout issues. For example, Shift-0
// and Shift-= and = are the same keycodes on some keyboards, yet they
// are not encoded int the same way in QKeyEvent and QKeySequenceEdit
// results.
//
QKeyCombination SelectKey::canonicalize(const QKeyCombination &combi) {
    const auto key = combi.key();
    auto mods = (combi.keyboardModifiers());

    switch (key) {
        case Qt::Key_Greater:
        case Qt::Key_Equal:
        case Qt::Key_Ampersand:
        case Qt::Key_ParenLeft:
        case Qt::Key_ParenRight:
        case Qt::Key_QuoteDbl:
        case Qt::Key_NumberSign:
        case Qt::Key_Dollar:
        case Qt::Key_Percent:
        case Qt::Key_Asterisk:
        case Qt::Key_Plus:
        case Qt::Key_Comma:
        case Qt::Key_Minus:
        case Qt::Key_Period:
        case Qt::Key_Slash:
        case Qt::Key_Colon:
        case Qt::Key_Semicolon:
        case Qt::Key_Less:
        case Qt::Key_Question:
        case Qt::Key_At:
        case Qt::Key_BracketLeft:
        case Qt::Key_Backslash:
        case Qt::Key_BracketRight:
        case Qt::Key_AsciiCircum:
        case Qt::Key_Underscore:
        case Qt::Key_QuoteLeft:
        case Qt::Key_BraceLeft:
        case Qt::Key_Bar:
        case Qt::Key_BraceRight:
        case Qt::Key_AsciiTilde:
        case Qt::Key_degree:
        case Qt::Key_paragraph:
            mods &= ~Qt::ShiftModifier;
            break;
        default:
            break;
    }
    return QKeyCombination(mods, key);
}
