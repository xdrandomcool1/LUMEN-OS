// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN touchpad gestures for KWin:
//   3-finger swipe up      → Mission Control (overview)
//   3-finger swipe down    → show desktop windows of the current app (App Exposé → window view)
//   3-finger swipe left/right → switch virtual desktop
//   pinch in (4 fingers)   → overview
#pragma once
#include <effect/effect.h>

namespace Lumen {
class GesturesEffect : public KWin::Effect
{
    Q_OBJECT
public:
    GesturesEffect();
    static bool supported() { return true; }
    static bool enabledByDefault() { return true; }
    bool isActive() const override { return false; } // never paints
private:
    static void invokeShortcut(const QString &name);
};
}
