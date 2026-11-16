// SPDX-License-Identifier: GPL-3.0-or-later
#include "lumendecoration.h"

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationSettings>
#include <KDecoration3/DecorationShadow>
#include <KPluginFactory>
#include <QPainter>
#include <QPainterPath>

K_PLUGIN_FACTORY_WITH_JSON(LumenDecorationFactory, "lumen.json", registerPlugin<Lumen::Decoration>();)

namespace Lumen {

namespace {
// Separable box blur ×3 on an alpha image (≈ Gaussian).
QImage blurred(QImage img, int r)
{
    img = img.convertToFormat(QImage::Format_Alpha8);
    const int w = img.width(), h = img.height();
    QList<uchar> tmp(qMax(w, h));
    for (int pass = 0; pass < 3; ++pass) {
        for (int y = 0; y < h; ++y) {
            uchar *row = img.scanLine(y);
            int sum = 0;
            for (int i = -r; i <= r; ++i) sum += row[qBound(0, i, w - 1)];
            for (int x = 0; x < w; ++x) { tmp[x] = uchar(sum / (2 * r + 1)); sum += row[qMin(x + r + 1, w - 1)] - row[qMax(x - r, 0)]; }
            memcpy(row, tmp.constData(), w);
        }
        for (int x = 0; x < w; ++x) {
            int sum = 0;
            for (int i = -r; i <= r; ++i) sum += img.scanLine(qBound(0, i, h - 1))[x];
            for (int y = 0; y < h; ++y) { tmp[y] = uchar(sum / (2 * r + 1)); sum += img.scanLine(qMin(y + r + 1, h - 1))[x] - img.scanLine(qMax(y - r, 0))[x]; }
            for (int y = 0; y < h; ++y) img.scanLine(y)[x] = tmp[y];
        }
    }
    return img;
}

std::shared_ptr<KDecoration3::DecorationShadow> makeShadow(bool active)
{
    static std::shared_ptr<KDecoration3::DecorationShadow> cache[2];
    auto &c = cache[active ? 1 : 0];
    if (c) return c;
    const int size = active ? 44 : 30, offY = active ? 16 : 10;
    const int pad = size + offY;
    const int core = int(Decoration::kRadius) * 2 + 8;
    const int side = pad * 2 + core;
    QImage canvas(side, side, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);
    auto layer = [&](int blur, int dy, int alpha) {
        QImage m(side, side, QImage::Format_Alpha8);
        m.fill(0);
        QPainter p(&m);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, alpha));
        p.drawRoundedRect(QRectF(pad, pad + dy, core, core), Decoration::kRadius, Decoration::kRadius);
        p.end();
        QPainter cp(&canvas);
        cp.drawImage(0, 0, blurred(m, blur / 3).convertToFormat(QImage::Format_ARGB32_Premultiplied));
    };
    layer(size, offY, active ? 90 : 55);
    layer(8, 3, active ? 60 : 35);
    {
        QPainter p(&canvas);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(0, 0, 0, active ? 64 : 40), 1));
        p.drawRoundedRect(QRectF(pad - 0.5, pad - 0.5, core + 1, core + 1), Decoration::kRadius + 0.5, Decoration::kRadius + 0.5);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.drawRoundedRect(QRectF(pad, pad, core, core), Decoration::kRadius, Decoration::kRadius);
    }
    c = std::make_shared<KDecoration3::DecorationShadow>();
    c->setPadding(QMarginsF(pad, pad, pad, pad));
    c->setInnerShadowRect(QRectF(pad + Decoration::kRadius, pad + Decoration::kRadius, core - 2 * Decoration::kRadius, core - 2 * Decoration::kRadius));
    c->setShadow(canvas);
    return c;
}
} // namespace

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
{
}

Decoration::~Decoration() = default;

bool Decoration::isDark() const
{
    return background().lightnessF() < 0.5;
}

QColor Decoration::background() const
{
    return window()->palette().color(QPalette::Window);
}

QColor Decoration::foreground() const
{
    QColor c = window()->palette().color(QPalette::WindowText);
    if (!window()->isActive()) c.setAlphaF(0.45);
    return c;
}

void Decoration::setButtonHovered(bool h)
{
    m_hoveredButtons = qMax(0, m_hoveredButtons + (h ? 1 : -1));
    update();
}

bool Decoration::init()
{
    auto *w = window();
    m_left = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_left->setSpacing(8);
    // Fixed order: close, minimize, maximize (traffic lights on the left).
    for (auto t : {KDecoration3::DecorationButtonType::Close, KDecoration3::DecorationButtonType::Minimize, KDecoration3::DecorationButtonType::Maximize}) {
        if (auto *b = Button::create(t, this, m_left))
            m_left->addButton(b);
    }
    connect(w, &KDecoration3::DecoratedWindow::activeChanged, this, [this] { updateShadow(); update(); });
    connect(w, &KDecoration3::DecoratedWindow::captionChanged, this, [this] { update(); });
    connect(w, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateLayout);
    connect(w, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateLayout);
    connect(w, &KDecoration3::DecoratedWindow::paletteChanged, this, [this] { update(); });
    updateLayout();
    updateShadow();
    return true;
}

void Decoration::updateLayout()
{
    const bool max = window()->isMaximized();
    const qreal w = window()->width();
    setBorders(QMarginsF(0, kTitleHeight, 0, 0));
    setResizeOnlyBorders(max ? QMarginsF() : QMarginsF(6, 6, 6, 6));
    setTitleBar(QRectF(0, 0, w, kTitleHeight));
    setBorderRadius(KDecoration3::BorderRadius(max ? 0 : kRadius));
    m_left->setPos(QPointF(14, (kTitleHeight - 12) / 2));
    update();
}

void Decoration::updateShadow()
{
    setShadow(makeShadow(window()->isActive()));
}

void Decoration::paint(QPainter *p, const QRectF &repaintArea)
{
    Q_UNUSED(repaintArea)
    const qreal w = window()->width();
    const qreal r = window()->isMaximized() ? 0 : kRadius;
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.moveTo(0, kTitleHeight);
    path.lineTo(0, r);
    path.quadTo(0, 0, r, 0);
    path.lineTo(w - r, 0);
    path.quadTo(w, 0, w, r);
    path.lineTo(w, kTitleHeight);
    path.closeSubpath();
    p->fillPath(path, background());
    // Hairline highlight along the top edge (glass rim).
    QColor rim = isDark() ? QColor(255, 255, 255, 26) : QColor(255, 255, 255, 140);
    p->setPen(QPen(rim, 1));
    p->drawPath(path.translated(0, 0.5));
    // Title
    QFont f = settings()->font();
    f.setFamily(QStringLiteral("Inter Variable"));
    f.setPixelSize(13);
    f.setWeight(QFont::DemiBold);
    p->setFont(f);
    p->setPen(foreground());
    const QRectF titleRect(80, 0, w - 160, kTitleHeight);
    const QString caption = p->fontMetrics().elidedText(window()->caption(), Qt::ElideMiddle, int(titleRect.width()));
    p->drawText(titleRect, Qt::AlignCenter, caption);
    p->restore();
    m_left->paint(p, repaintArea);
}

// ---------------------------------------------------------------- buttons
Button::Button(KDecoration3::DecorationButtonType type, Decoration *decoration, QObject *parent)
    : KDecoration3::DecorationButton(type, decoration, parent)
{
    setGeometry(QRectF(0, 0, 12, 12));
    connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, [decoration](bool h) { decoration->setButtonHovered(h); });
}

KDecoration3::DecorationButton *Button::create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
{
    auto *d = qobject_cast<Decoration *>(decoration);
    if (!d) return nullptr;
    switch (type) {
    case KDecoration3::DecorationButtonType::Close:
    case KDecoration3::DecorationButtonType::Minimize:
    case KDecoration3::DecorationButtonType::Maximize:
        return new Button(type, d, parent);
    default:
        return nullptr;
    }
}

void Button::paint(QPainter *p, const QRectF &)
{
    auto *d = static_cast<Decoration *>(decoration());
    const bool active = d->window()->isActive() || d->buttonsHovered();
    QColor fill;
    switch (type()) {
    case KDecoration3::DecorationButtonType::Close: fill = QColor(0xFF, 0x5F, 0x57); break;
    case KDecoration3::DecorationButtonType::Minimize: fill = QColor(0xFE, 0xBC, 0x2E); break;
    default: fill = QColor(0x28, 0xC8, 0x40); break;
    }
    if (!isEnabled() || !active) fill = d->isDark() ? QColor(0x4A, 0x4A, 0x4D) : QColor(0xD0, 0xD0, 0xD2);
    if (isPressed()) fill = fill.darker(118);
    const QRectF g = geometry();
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(QPen(QColor(0, 0, 0, d->isDark() ? 64 : 30), 0.5));
    p->setBrush(fill);
    p->drawEllipse(g.adjusted(0.25, 0.25, -0.25, -0.25));
    if (d->buttonsHovered() && isEnabled()) {
        const QColor ink(0, 0, 0, 140);
        p->setPen(QPen(ink, 1.2, Qt::SolidLine, Qt::RoundCap));
        p->setBrush(ink);
        const qreal x = g.x(), y = g.y(), s = g.width();
        if (type() == KDecoration3::DecorationButtonType::Close) {
            p->drawLine(QPointF(x + s * 0.32, y + s * 0.32), QPointF(x + s * 0.68, y + s * 0.68));
            p->drawLine(QPointF(x + s * 0.68, y + s * 0.32), QPointF(x + s * 0.32, y + s * 0.68));
        } else if (type() == KDecoration3::DecorationButtonType::Minimize) {
            p->drawLine(QPointF(x + s * 0.27, y + s * 0.5), QPointF(x + s * 0.73, y + s * 0.5));
        } else {
            p->setPen(Qt::NoPen);
            QPolygonF a{QPointF(x + s * 0.30, y + s * 0.30), QPointF(x + s * 0.62, y + s * 0.30), QPointF(x + s * 0.30, y + s * 0.62)};
            QPolygonF b{QPointF(x + s * 0.70, y + s * 0.70), QPointF(x + s * 0.38, y + s * 0.70), QPointF(x + s * 0.70, y + s * 0.38)};
            p->drawPolygon(a);
            p->drawPolygon(b);
        }
    }
    p->restore();
}

} // namespace Lumen

#include "lumendecoration.moc"
