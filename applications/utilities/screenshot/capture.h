// SPDX-License-Identifier: GPL-3.0-or-later
// Screen capture through KWin's org.kde.KWin.ScreenShot2 interface.
#pragma once
#include <QImage>
#include <QRect>
#include <QString>
#include <QVariantMap>

namespace Lumen::Capture {
enum class Mode { Workspace, Screen, ActiveWindow, Area };
//! Blocking capture. \a error receives a message on failure.
QImage grab(Mode mode, const QRect &area = {}, bool includeCursor = false, QString *error = nullptr);
}
