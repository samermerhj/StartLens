/*
 * Copyright 2026 Samer Merhj <mjosak7@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "qhotkey.h"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class QHotkey::Private : public QAbstractNativeEventFilter
{
public:
    Private(QHotkey *qq) : q(qq) {}
    QHotkey *q;
    QKeySequence shortcut;
    xcb_keycode_t keyCode = 0;
    unsigned int modifierMask = 0;
    bool registered = false;

    bool registerShortcut(const QKeySequence &seq) {
        if (registered) unregisterShortcut();
        shortcut = seq;
        Display *display = XOpenDisplay(nullptr);
        if (!display) return false;
        QString keyStr = seq.toString();
        Qt::KeyboardModifiers mods = Qt::NoModifier;
        if (keyStr.contains("Super")) mods |= Qt::MetaModifier;
        if (keyStr.contains("Ctrl"))  mods |= Qt::ControlModifier;
        if (keyStr.contains("Alt"))   mods |= Qt::AltModifier;
        if (keyStr.contains("Shift")) mods |= Qt::ShiftModifier;
        KeySym keysym = XStringToKeysym(keyStr.remove("Super+").remove("Ctrl+").remove("Alt+").remove("Shift+").toUtf8().constData());
        if (keysym == NoSymbol) { XCloseDisplay(display); return false; }
        keyCode = XKeysymToKeycode(display, keysym);
        modifierMask = 0;
        if (mods & Qt::ShiftModifier)   modifierMask |= ShiftMask;
        if (mods & Qt::ControlModifier) modifierMask |= ControlMask;
        if (mods & Qt::AltModifier)     modifierMask |= Mod1Mask;
        if (mods & Qt::MetaModifier)    modifierMask |= Mod4Mask;
        XGrabKey(display, keyCode, modifierMask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
        XCloseDisplay(display);
        QCoreApplication::instance()->eventDispatcher()->installNativeEventFilter(this);
        registered = true;
        return true;
    }

    void unregisterShortcut() {
        if (!registered) return;
        Display *display = XOpenDisplay(nullptr);
        if (display) {
            XUngrabKey(display, keyCode, modifierMask, DefaultRootWindow(display));
            XCloseDisplay(display);
        }
        QCoreApplication::instance()->eventDispatcher()->removeNativeEventFilter(this);
        registered = false;
    }

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override {
        if (eventType == "xcb_generic_event_t") {
            xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
            if (ev->response_type == XCB_KEY_PRESS) {
                xcb_key_press_event_t *kp = (xcb_key_press_event_t *)ev;
                if (kp->detail == keyCode && (kp->state & modifierMask) == modifierMask) {
                    emit q->activated();
                    return true;
                }
            }
        }
        return false;
    }
};

QHotkey::QHotkey(QObject *parent) : QObject(parent), d(new Private(this)) {}
QHotkey::~QHotkey() { delete d; }
bool QHotkey::registerShortcut(const QKeySequence &shortcut) { return d->registerShortcut(shortcut); }
void QHotkey::unregisterShortcut() { d->unregisterShortcut(); }
