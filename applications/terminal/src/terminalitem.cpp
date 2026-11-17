// SPDX-License-Identifier: GPL-3.0-or-later
#include "terminalitem.h"

#include <QClipboard>
#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include <csignal>
#include <fcntl.h>
#include <pty.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

namespace Lumen {

static constexpr size_t kMaxScrollback = 10000;

namespace {
struct Theme { const char *name; const char *fg; const char *bg; const char *cursor; const char *sel; const char *pal[16]; };
const Theme kThemes[] = {
    {"Lumen Dark", "#E6E6EA", "#1A1B1F", "#E6E6EA", "#3A5A8C",
     {"#1E1F24", "#FF6B6B", "#5AD07A", "#F2C94C", "#5AA9FF", "#C792EA", "#4FD1C5", "#C8C8CE",
      "#5C5F66", "#FF8787", "#7BE495", "#FFD866", "#82BFFF", "#D7A9F5", "#76E4D9", "#FFFFFF"}},
    {"Lumen Light", "#1D1D1F", "#FFFFFF", "#1D1D1F", "#B4D5FE",
     {"#1D1D1F", "#C4262E", "#1F8A3B", "#9A6A00", "#1F5FD1", "#8A3FB8", "#127C87", "#8E8E93",
      "#5C5C61", "#E0484F", "#2BA34B", "#B98400", "#3A7BEB", "#A55AD6", "#1A97A3", "#C7C7CC"}},
    {"Midnight", "#C9D1D9", "#0D1117", "#58A6FF", "#1F3A5F",
     {"#484F58", "#FF7B72", "#3FB950", "#D29922", "#58A6FF", "#BC8CFF", "#39C5CF", "#B1BAC4",
      "#6E7681", "#FFA198", "#56D364", "#E3B341", "#79C0FF", "#D2A8FF", "#56D4DD", "#FFFFFF"}},
    {"Solar", "#839496", "#002B36", "#93A1A1", "#073642",
     {"#073642", "#DC322F", "#859900", "#B58900", "#268BD2", "#D33682", "#2AA198", "#EEE8D5",
      "#002B36", "#CB4B16", "#586E75", "#657B83", "#839496", "#6C71C4", "#93A1A1", "#FDF6E3"}},
};

int cbDamage(VTermRect r, void *u) { return static_cast<TerminalItem *>(u)->onDamage(r); }
int cbMoveRect(VTermRect, VTermRect, void *u) { static_cast<TerminalItem *>(u)->update(); return 1; }
int cbMoveCursor(VTermPos p, VTermPos, int vis, void *u) { return static_cast<TerminalItem *>(u)->onMoveCursor(p, vis); }
int cbTermProp(VTermProp prop, VTermValue *v, void *u) { return static_cast<TerminalItem *>(u)->onSetTermProp(prop, v); }
int cbBell(void *u) { return static_cast<TerminalItem *>(u)->onBell(); }
int cbPush(int cols, const VTermScreenCell *c, void *u) { return static_cast<TerminalItem *>(u)->onPushLine(cols, c); }
int cbPop(int cols, VTermScreenCell *c, void *u) { return static_cast<TerminalItem *>(u)->onPopLine(cols, c); }
const VTermScreenCallbacks kCallbacks = {cbDamage, cbMoveRect, cbMoveCursor, cbTermProp, cbBell, nullptr, cbPush, cbPop, nullptr};

void outputCb(const char *s, size_t len, void *user)
{
    static_cast<QByteArray *>(user)->append(s, qsizetype(len));
}
} // namespace

QStringList TerminalItem::themes()
{
    QStringList l;
    for (const auto &t : kThemes) l << QLatin1String(t.name);
    return l;
}

TerminalItem::TerminalItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setFlag(ItemAcceptsInputMethod, true);
    setFlag(ItemIsFocusScope, true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton);
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::IBeamCursor));
    setOpaquePainting(true);
    m_font = QFont(QStringLiteral("JetBrains Mono"));
    m_font.setStyleHint(QFont::Monospace);
    m_font.setPixelSize(13);
    m_font.setKerning(false);
    m_vt = vterm_new(m_rows, m_cols);
    vterm_set_utf8(m_vt, 1);
    vterm_output_set_callback(m_vt, outputCb, &m_out);
    m_screen = vterm_obtain_screen(m_vt);
    vterm_screen_set_callbacks(m_screen, &kCallbacks, this);
    vterm_screen_set_damage_merge(m_screen, VTERM_DAMAGE_SCROLL);
    vterm_screen_enable_altscreen(m_screen, 1);
    vterm_screen_reset(m_screen, 1);
    setTheme(m_theme);
    updateMetrics();
}

TerminalItem::~TerminalItem()
{
    if (m_pid > 0) {
        ::kill(m_pid, SIGHUP);
        ::waitpid(m_pid, nullptr, WNOHANG);
    }
    if (m_fd >= 0)
        ::close(m_fd);
    vterm_free(m_vt);
}

void TerminalItem::setFontFamily(const QString &f)
{
    m_font.setFamily(f);
    updateMetrics();
    Q_EMIT fontChanged();
}

void TerminalItem::setFontSize(qreal s)
{
    m_font.setPixelSize(qMax(6, qRound(s)));
    updateMetrics();
    Q_EMIT fontChanged();
}

void TerminalItem::updateMetrics()
{
    const QFontMetricsF fm(m_font);
    m_cellW = fm.horizontalAdvance(QLatin1Char('M'));
    m_cellH = qCeil(fm.height() * 1.12);
    m_ascent = fm.ascent() + (m_cellH - fm.height()) / 2;
    resizeTerminal();
    update();
}

void TerminalItem::setTheme(const QString &name)
{
    const Theme *t = &kThemes[0];
    for (const auto &th : kThemes)
        if (name == QLatin1String(th.name)) t = &th;
    m_theme = QLatin1String(t->name);
    m_fg = QColor(QLatin1String(t->fg));
    m_bg = QColor(QLatin1String(t->bg));
    m_cursorColor = QColor(QLatin1String(t->cursor));
    m_selection = QColor(QLatin1String(t->sel));
    m_palette.clear();
    for (const char *c : t->pal) m_palette << QColor(QLatin1String(c));
    applyPalette();
    setFillColor(m_bg);
    Q_EMIT themeChanged();
    update();
}

void TerminalItem::applyPalette()
{
    VTermState *state = vterm_obtain_state(m_vt);
    for (int i = 0; i < 16; ++i) {
        VTermColor c;
        vterm_color_rgb(&c, m_palette[i].red(), m_palette[i].green(), m_palette[i].blue());
        vterm_state_set_palette_color(state, i, &c);
    }
    VTermColor fg, bg;
    vterm_color_rgb(&fg, m_fg.red(), m_fg.green(), m_fg.blue());
    vterm_color_rgb(&bg, m_bg.red(), m_bg.green(), m_bg.blue());
    vterm_state_set_default_colors(state, &fg, &bg);
}

void TerminalItem::start()
{
    if (m_pid > 0)
        return;
    resizeTerminal();
    struct winsize ws{};
    ws.ws_row = m_rows;
    ws.ws_col = m_cols;
    int master = -1;
    const pid_t pid = ::forkpty(&master, nullptr, nullptr, &ws);
    if (pid < 0)
        return;
    if (pid == 0) {
        // Child: become the shell.
        ::setenv("TERM", "xterm-256color", 1);
        ::setenv("COLORTERM", "truecolor", 1);
        ::setenv("TERM_PROGRAM", "Lumen Terminal", 1);
        ::unsetenv("QT_QPA_PLATFORM_THEME");
        if (!m_cwd.isEmpty())
            if (::chdir(QFile::encodeName(m_cwd).constData()) != 0) {}
        if (!m_command.isEmpty()) {
            std::vector<QByteArray> args;
            for (const auto &a : m_command) args.push_back(a.toLocal8Bit());
            std::vector<char *> argv;
            for (auto &a : args) argv.push_back(a.data());
            argv.push_back(nullptr);
            ::execvp(argv[0], argv.data());
            ::_exit(127);
        }
        const char *shell = ::getenv("SHELL");
        if (!shell || !*shell) {
            if (passwd *pw = ::getpwuid(::getuid())) shell = pw->pw_shell;
        }
        if (!shell || !*shell) shell = "/bin/bash";
        const QByteArray login = QByteArray("-") + QByteArray(shell).mid(QByteArray(shell).lastIndexOf('/') + 1);
        ::execl(shell, login.constData(), static_cast<char *>(nullptr));
        ::_exit(127);
    }
    m_pid = pid;
    m_fd = master;
    ::fcntl(m_fd, F_SETFL, ::fcntl(m_fd, F_GETFL) | O_NONBLOCK);
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &TerminalItem::readPty);
    Q_EMIT runningChanged();
}

void TerminalItem::readPty()
{
    char buf[1 << 16];
    for (;;) {
        const ssize_t n = ::read(m_fd, buf, sizeof buf);
        if (n > 0) {
            vterm_input_write(m_vt, buf, size_t(n));
            continue;
        }
        if (n == 0 || (n < 0 && errno != EAGAIN && errno != EINTR)) {
            childExited();
            return;
        }
        break;
    }
    vterm_screen_flush_damage(m_screen);
    flushOutput();
    if (m_scroll != 0) { m_scroll = 0; Q_EMIT scrollChanged(); }
    update();
}

void TerminalItem::childExited()
{
    if (m_notifier) { m_notifier->setEnabled(false); m_notifier->deleteLater(); m_notifier = nullptr; }
    int status = 0;
    if (m_pid > 0) ::waitpid(m_pid, &status, 0);
    m_pid = -1;
    Q_EMIT runningChanged();
    Q_EMIT finished(WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}

void TerminalItem::writePty(const char *data, size_t len)
{
    if (m_fd < 0) return;
    while (len > 0) {
        const ssize_t n = ::write(m_fd, data, len);
        if (n < 0) { if (errno == EINTR || errno == EAGAIN) continue; break; }
        data += n; len -= size_t(n);
    }
}

void TerminalItem::flushOutput()
{
    if (!m_out.isEmpty()) {
        writePty(m_out.constData(), size_t(m_out.size()));
        m_out.clear();
    }
}

void TerminalItem::sendText(const QString &text)
{
    const QByteArray u = text.toUtf8();
    writePty(u.constData(), size_t(u.size()));
}

void TerminalItem::resizeTerminal()
{
    const int cols = qMax(10, int((width() - 16) / m_cellW));
    const int rows = qMax(3, int((height() - 8) / m_cellH));
    if (cols == m_cols && rows == m_rows && m_vt)
        return;
    m_cols = cols;
    m_rows = rows;
    vterm_set_size(m_vt, rows, cols);
    vterm_screen_flush_damage(m_screen);
    if (m_fd >= 0) {
        struct winsize ws{};
        ws.ws_row = rows; ws.ws_col = cols;
        ::ioctl(m_fd, TIOCSWINSZ, &ws);
    }
    Q_EMIT sizeChanged();
}

void TerminalItem::geometryChange(const QRectF &n, const QRectF &o)
{
    QQuickPaintedItem::geometryChange(n, o);
    resizeTerminal();
}

int TerminalItem::onDamage(VTermRect) { update(); return 1; }
int TerminalItem::onMoveCursor(VTermPos pos, int visible) { m_cursor = pos; m_cursorVisible = visible; update(); return 1; }
int TerminalItem::onBell() { Q_EMIT bell(); return 1; }

int TerminalItem::onSetTermProp(VTermProp prop, VTermValue *val)
{
    switch (prop) {
    case VTERM_PROP_TITLE:
        if (val->string.initial) m_titleBuf.clear();
        m_titleBuf.append(val->string.str, qsizetype(val->string.len));
        if (val->string.final) { m_title = QString::fromUtf8(m_titleBuf); Q_EMIT titleChanged(); }
        break;
    case VTERM_PROP_CURSORVISIBLE: m_cursorVisible = val->boolean; break;
    case VTERM_PROP_CURSORSHAPE: m_cursorShape = val->number; break;
    default: break;
    }
    update();
    return 1;
}

int TerminalItem::onPushLine(int cols, const VTermScreenCell *cells)
{
    m_scrollback.emplace_back(cells, cells + cols);
    if (m_scrollback.size() > kMaxScrollback) m_scrollback.pop_front();
    Q_EMIT scrollChanged();
    return 1;
}

int TerminalItem::onPopLine(int cols, VTermScreenCell *cells)
{
    if (m_scrollback.empty()) return 0;
    const auto &line = m_scrollback.back();
    VTermScreenCell blank{};
    blank.width = 1;
    for (int i = 0; i < cols; ++i) cells[i] = i < int(line.size()) ? line[size_t(i)] : blank;
    m_scrollback.pop_back();
    Q_EMIT scrollChanged();
    return 1;
}

void TerminalItem::setScrollOffset(int o)
{
    o = qBound(0, o, int(m_scrollback.size()));
    if (o == m_scroll) return;
    m_scroll = o;
    Q_EMIT scrollChanged();
    update();
}

void TerminalItem::clearScrollback()
{
    m_scrollback.clear();
    m_scroll = 0;
    Q_EMIT scrollChanged();
    update();
}

bool TerminalItem::cellAt(int row, int col, VTermScreenCell &cell) const
{
    const int abs = row - m_scroll; // < 0 → scrollback
    if (abs < 0) {
        const int idx = int(m_scrollback.size()) + abs;
        if (idx < 0) return false;
        const auto &line = m_scrollback[size_t(idx)];
        if (col >= int(line.size())) return false;
        cell = line[size_t(col)];
        return true;
    }
    return vterm_screen_get_cell(m_screen, VTermPos{abs, col}, &cell) != 0;
}

QColor TerminalItem::toQColor(VTermColor c, bool fg) const
{
    if ((fg && VTERM_COLOR_IS_DEFAULT_FG(&c)) || (!fg && VTERM_COLOR_IS_DEFAULT_BG(&c)))
        return fg ? m_fg : m_bg;
    vterm_screen_convert_color_to_rgb(m_screen, &c);
    return QColor(c.rgb.red, c.rgb.green, c.rgb.blue);
}

bool TerminalItem::isSelected(int absRow, int col) const
{
    if (!m_hasSel) return false;
    QPoint a = m_selA, b = m_selB;
    if (b.y() < a.y() || (b.y() == a.y() && b.x() < a.x())) std::swap(a, b);
    if (absRow < a.y() || absRow > b.y()) return false;
    if (absRow == a.y() && col < a.x()) return false;
    if (absRow == b.y() && col > b.x()) return false;
    return true;
}

void TerminalItem::paint(QPainter *p)
{
    p->fillRect(boundingRect(), m_bg);
    p->setFont(m_font);
    const qreal ox = 8, oy = 4;
    for (int r = 0; r < m_rows; ++r) {
        const qreal y = oy + r * m_cellH;
        const int absRow = r - m_scroll;
        int c = 0;
        while (c < m_cols) {
            VTermScreenCell cell{};
            if (!cellAt(r, c, cell)) { ++c; continue; }
            const int w = qMax<int>(1, cell.width);
            QColor fg = toQColor(cell.fg, true), bg = toQColor(cell.bg, false);
            if (cell.attrs.reverse) std::swap(fg, bg);
            const bool sel = isSelected(absRow, c);
            if (sel) bg = m_selection;
            if (bg != m_bg) p->fillRect(QRectF(ox + c * m_cellW, y, m_cellW * w, m_cellH), bg);
            if (cell.chars[0] && cell.chars[0] != ' ') {
                QString text;
                for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i]; ++i)
                    text += QString::fromUcs4(reinterpret_cast<const char32_t *>(&cell.chars[i]), 1);
                QFont f = m_font;
                if (cell.attrs.bold) f.setWeight(QFont::Bold);
                if (cell.attrs.italic) f.setItalic(true);
                f.setUnderline(cell.attrs.underline);
                f.setStrikeOut(cell.attrs.strike);
                p->setFont(f);
                if (cell.attrs.bold && !cell.attrs.reverse) fg = fg.lighter(110);
                p->setPen(sel ? m_fg : fg);
                p->drawText(QPointF(ox + c * m_cellW, y + m_ascent), text);
            }
            c += w;
        }
    }
    // Cursor
    if (m_cursorVisible && m_scroll == 0 && m_pid > 0) {
        const QRectF cr(ox + m_cursor.col * m_cellW, oy + m_cursor.row * m_cellH, m_cellW, m_cellH);
        if (!m_focused) {
            p->setPen(QPen(m_cursorColor, 1)); p->setBrush(Qt::NoBrush); p->drawRect(cr.adjusted(0.5, 0.5, -0.5, -0.5));
        } else if (m_cursorShape == VTERM_PROP_CURSORSHAPE_BAR_LEFT) {
            p->fillRect(QRectF(cr.left(), cr.top(), 2, cr.height()), m_cursorColor);
        } else if (m_cursorShape == VTERM_PROP_CURSORSHAPE_UNDERLINE) {
            p->fillRect(QRectF(cr.left(), cr.bottom() - 2, cr.width(), 2), m_cursorColor);
        } else {
            QColor cc = m_cursorColor; cc.setAlphaF(0.85);
            p->fillRect(cr, cc);
            VTermScreenCell cell{};
            if (cellAt(m_cursor.row, m_cursor.col, cell) && cell.chars[0]) {
                p->setPen(m_bg); p->setFont(m_font);
                p->drawText(QPointF(cr.left(), cr.top() + m_ascent), QString::fromUcs4(reinterpret_cast<const char32_t *>(&cell.chars[0]), 1));
            }
        }
    }
}

// ---- Input ---------------------------------------------------------------
static VTermModifier mods(Qt::KeyboardModifiers m)
{
    int r = VTERM_MOD_NONE;
    if (m & Qt::ShiftModifier) r |= VTERM_MOD_SHIFT;
    if (m & Qt::AltModifier) r |= VTERM_MOD_ALT;
    if (m & Qt::ControlModifier) r |= VTERM_MOD_CTRL;
    return VTermModifier(r);
}

void TerminalItem::keyPressEvent(QKeyEvent *e)
{
    const auto m = e->modifiers();
    // Clipboard: Ctrl+Shift+C/V (and the Lumen "Super" equivalents).
    if ((m & (Qt::ControlModifier | Qt::ShiftModifier)) == (Qt::ControlModifier | Qt::ShiftModifier) || (m & Qt::MetaModifier)) {
        if (e->key() == Qt::Key_C) { copySelection(); return; }
        if (e->key() == Qt::Key_V) { paste(); return; }
    }
    if (m & Qt::ShiftModifier) {
        if (e->key() == Qt::Key_PageUp) { setScrollOffset(m_scroll + m_rows - 1); return; }
        if (e->key() == Qt::Key_PageDown) { setScrollOffset(m_scroll - m_rows + 1); return; }
    }
    struct Map { int qt; VTermKey vk; };
    static const Map map[] = {
        {Qt::Key_Return, VTERM_KEY_ENTER}, {Qt::Key_Enter, VTERM_KEY_ENTER}, {Qt::Key_Tab, VTERM_KEY_TAB}, {Qt::Key_Backtab, VTERM_KEY_TAB},
        {Qt::Key_Backspace, VTERM_KEY_BACKSPACE}, {Qt::Key_Escape, VTERM_KEY_ESCAPE}, {Qt::Key_Up, VTERM_KEY_UP}, {Qt::Key_Down, VTERM_KEY_DOWN},
        {Qt::Key_Left, VTERM_KEY_LEFT}, {Qt::Key_Right, VTERM_KEY_RIGHT}, {Qt::Key_Insert, VTERM_KEY_INS}, {Qt::Key_Delete, VTERM_KEY_DEL},
        {Qt::Key_Home, VTERM_KEY_HOME}, {Qt::Key_End, VTERM_KEY_END}, {Qt::Key_PageUp, VTERM_KEY_PAGEUP}, {Qt::Key_PageDown, VTERM_KEY_PAGEDOWN},
    };
    m_hasSel = false;
    for (const auto &k : map) {
        if (e->key() == k.qt) {
            vterm_keyboard_key(m_vt, k.vk, mods(e->key() == Qt::Key_Backtab ? (m | Qt::ShiftModifier) : m));
            flushOutput();
            return;
        }
    }
    if (e->key() >= Qt::Key_F1 && e->key() <= Qt::Key_F12) {
        vterm_keyboard_key(m_vt, VTermKey(VTERM_KEY_FUNCTION(e->key() - Qt::Key_F1 + 1)), mods(m));
        flushOutput();
        return;
    }
    if (m & Qt::ControlModifier && e->key() >= Qt::Key_A && e->key() <= Qt::Key_Z) {
        vterm_keyboard_unichar(m_vt, uint32_t('a' + (e->key() - Qt::Key_A)), mods(m & ~Qt::ShiftModifier));
        flushOutput();
        return;
    }
    const QString text = e->text();
    if (!text.isEmpty()) {
        for (char32_t ch : text.toUcs4())
            vterm_keyboard_unichar(m_vt, ch, mods(m & Qt::AltModifier));
        flushOutput();
        return;
    }
    QQuickPaintedItem::keyPressEvent(e);
}

void TerminalItem::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        for (char32_t ch : e->commitString().toUcs4())
            vterm_keyboard_unichar(m_vt, ch, VTERM_MOD_NONE);
        flushOutput();
    }
    e->accept();
}

QVariant TerminalItem::inputMethodQuery(Qt::InputMethodQuery q) const
{
    if (q == Qt::ImCursorRectangle)
        return QRectF(8 + m_cursor.col * m_cellW, 4 + m_cursor.row * m_cellH, m_cellW, m_cellH);
    return QQuickPaintedItem::inputMethodQuery(q);
}

QPoint TerminalItem::cellFromPos(const QPointF &p) const
{
    const int col = qBound(0, int((p.x() - 8) / m_cellW), m_cols - 1);
    const int row = qBound(0, int((p.y() - 4) / m_cellH), m_rows - 1);
    return QPoint(col, row - m_scroll);
}

void TerminalItem::mousePressEvent(QMouseEvent *e)
{
    forceActiveFocus();
    if (e->button() == Qt::MiddleButton) {
        const QString t = QGuiApplication::clipboard()->text(QClipboard::Selection);
        if (!t.isEmpty()) sendText(t);
        return;
    }
    m_selecting = true;
    m_hasSel = false;
    m_selA = m_selB = cellFromPos(e->position());
    update();
}

void TerminalItem::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_selecting) return;
    m_selB = cellFromPos(e->position());
    m_hasSel = m_selA != m_selB;
    update();
}

void TerminalItem::mouseReleaseEvent(QMouseEvent *)
{
    m_selecting = false;
    if (m_hasSel)
        QGuiApplication::clipboard()->setText(selectedText(), QClipboard::Selection);
}

void TerminalItem::mouseDoubleClickEvent(QMouseEvent *e)
{
    const QPoint c = cellFromPos(e->position());
    auto isWord = [&](int col) {
        VTermScreenCell cell{};
        if (!cellAt(c.y() + m_scroll, col, cell) || !cell.chars[0]) return false;
        const QChar ch(char16_t(cell.chars[0]));
        return ch.isLetterOrNumber() || QStringLiteral("-_./~:@").contains(ch);
    };
    int a = c.x(), b = c.x();
    while (a > 0 && isWord(a - 1)) --a;
    while (b < m_cols - 1 && isWord(b + 1)) ++b;
    m_selA = QPoint(a, c.y());
    m_selB = QPoint(b, c.y());
    m_hasSel = true;
    QGuiApplication::clipboard()->setText(selectedText(), QClipboard::Selection);
    update();
}

void TerminalItem::wheelEvent(QWheelEvent *e)
{
    const int lines = e->angleDelta().y() / 40;
    setScrollOffset(m_scroll + lines);
}

QString TerminalItem::selectedText() const
{
    if (!m_hasSel) return {};
    QPoint a = m_selA, b = m_selB;
    if (b.y() < a.y() || (b.y() == a.y() && b.x() < a.x())) std::swap(a, b);
    QString out;
    for (int row = a.y(); row <= b.y(); ++row) {
        QString line;
        const int c0 = row == a.y() ? a.x() : 0, c1 = row == b.y() ? b.x() : m_cols - 1;
        for (int c = c0; c <= c1; ++c) {
            VTermScreenCell cell{};
            if (!cellAt(row + m_scroll, c, cell)) continue;
            line += cell.chars[0] ? QString::fromUcs4(reinterpret_cast<const char32_t *>(&cell.chars[0]), 1) : QStringLiteral(" ");
        }
        while (line.endsWith(QLatin1Char(' '))) line.chop(1);
        out += line;
        if (row != b.y()) out += QLatin1Char('\n');
    }
    return out;
}

void TerminalItem::copySelection()
{
    const QString t = selectedText();
    if (!t.isEmpty()) QGuiApplication::clipboard()->setText(t);
}

void TerminalItem::paste()
{
    QString t = QGuiApplication::clipboard()->text();
    if (t.isEmpty()) return;
    t.replace(QStringLiteral("\r\n"), QStringLiteral("\r")).replace(QLatin1Char('\n'), QLatin1Char('\r'));
    // Bracketed paste lets shells avoid executing pasted newlines blindly.
    sendText(QStringLiteral("\x1b[200~") + t + QStringLiteral("\x1b[201~"));
}

void TerminalItem::selectAll()
{
    m_selA = QPoint(0, -int(m_scrollback.size()));
    m_selB = QPoint(m_cols - 1, m_rows - 1);
    m_hasSel = true;
    update();
}

QString TerminalItem::currentDirectory() const
{
    if (m_pid <= 0) return m_cwd;
    // Foreground process group of the PTY → its cwd (follows `cd`).
    const pid_t fg = ::tcgetpgrp(m_fd);
    const QString link = QStringLiteral("/proc/%1/cwd").arg(fg > 0 ? fg : m_pid);
    return QFile::symLinkTarget(link);
}

} // namespace Lumen
