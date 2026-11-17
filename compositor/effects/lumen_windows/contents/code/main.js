// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN window motion (KWin scripted effect).
//
// Timings and curves mirror Lumen.Motion (animation-engine/src/motion.h):
//   windowOpen 280 ms, windowClose 200 ms, minimize 420 ms,
//   entrances use an ease-out (emphasized) curve, exits ease-in.
// Minimize animates the window into its Dock icon: lumen-shell publishes
// each Dock icon's geometry through TaskManager, which KWin exposes as
// window.iconGeometry.
"use strict";

const reduced = () => effect.readConfig("ReduceMotion", false);
const dur = ms => animationTime(reduced() ? Math.min(ms, 120) : ms);

function isManaged(w) {
    return w.normalWindow || w.dialog || w.utility;
}

function openWindow(window) {
    if (!isManaged(window) || effects.hasActiveFullScreenEffect) return;
    if (window.windowClass === "lumen-shell lumen-shell") return;
    window.setData(Effect.WindowForceBlurRole, true);
    cancel(window.lumenAnim || []);
    window.lumenAnim = animate({
        window: window,
        duration: dur(280),
        curve: QEasingCurve.OutCubic,
        animations: reduced() ? [{ type: Effect.Opacity, from: 0, to: 1 }] : [
            { type: Effect.Scale, from: 0.92, to: 1.0 },
            { type: Effect.Opacity, from: 0.0, to: 1.0 }
        ]
    });
}

function closeWindow(window) {
    if (!isManaged(window) || !window.visible || effects.hasActiveFullScreenEffect) return;
    cancel(window.lumenAnim || []);
    window.setData(Effect.WindowClosedGrabRole, effect);
    window.lumenAnim = animate({
        window: window,
        duration: dur(200),
        curve: QEasingCurve.InCubic,
        animations: reduced() ? [{ type: Effect.Opacity, to: 0 }] : [
            { type: Effect.Scale, to: 0.94 },
            { type: Effect.Opacity, to: 0.0 }
        ]
    });
}

function minimize(window) {
    const icon = window.iconGeometry;
    if (!icon || icon.width <= 0) return; // no Dock target: KWin default
    const g = window.geometry;
    const sx = icon.width / g.width, sy = icon.height / g.height;
    const dx = icon.x + icon.width / 2 - (g.x + g.width / 2);
    const dy = icon.y + icon.height / 2 - (g.y + g.height / 2);
    cancel(window.lumenAnim || []);
    window.lumenAnim = animate({
        window: window,
        duration: dur(420),
        curve: QEasingCurve.InOutCubic,
        animations: [
            { type: Effect.Size, to: { value1: g.width * sx, value2: g.height * sy } },
            { type: Effect.Translation, to: { value1: dx, value2: dy } },
            { type: Effect.Opacity, to: 0.0 }
        ]
    });
}

function unminimize(window) {
    const icon = window.iconGeometry;
    if (!icon || icon.width <= 0) return;
    const g = window.geometry;
    const dx = icon.x + icon.width / 2 - (g.x + g.width / 2);
    const dy = icon.y + icon.height / 2 - (g.y + g.height / 2);
    cancel(window.lumenAnim || []);
    window.lumenAnim = animate({
        window: window,
        duration: dur(380),
        curve: QEasingCurve.OutCubic,
        animations: [
            { type: Effect.Size, from: { value1: icon.width, value2: icon.height }, to: { value1: g.width, value2: g.height } },
            { type: Effect.Translation, from: { value1: dx, value2: dy }, to: { value1: 0, value2: 0 } },
            { type: Effect.Opacity, from: 0.0, to: 1.0 }
        ]
    });
}

effects.windowAdded.connect(openWindow);
effects.windowClosed.connect(closeWindow);
effects.windowMinimized.connect(minimize);
effects.windowUnminimized.connect(unminimize);
