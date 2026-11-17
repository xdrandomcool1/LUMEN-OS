// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN window decoration (server-side) for third-party applications.
#pragma once

#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationButton>
#include <QVariantList>
#include <memory>

namespace KDecoration3 { class DecorationButtonGroup; class DecorationShadow; }

namespace Lumen {

class Decoration : public KDecoration3::Decoration
{
    Q_OBJECT
public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = {});
    ~Decoration() override;

    bool init() override;
    void paint(QPainter *painter, const QRectF &repaintArea) override;

    bool buttonsHovered() const { return m_hoveredButtons > 0; }
    void setButtonHovered(bool h);
    QColor background() const;
    QColor foreground() const;
    bool isDark() const;

    static constexpr qreal kTitleHeight = 32;
    static constexpr qreal kRadius = 12;

private:
    void updateLayout();
    void updateShadow();
    KDecoration3::DecorationButtonGroup *m_left = nullptr;
    int m_hoveredButtons = 0;
};

class Button : public KDecoration3::DecorationButton
{
    Q_OBJECT
public:
    Button(KDecoration3::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);
    void paint(QPainter *painter, const QRectF &repaintArea) override;
    static KDecoration3::DecorationButton *create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent);
};

} // namespace Lumen
