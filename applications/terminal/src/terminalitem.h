// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Terminal — a VT terminal (libvterm) rendered in Qt Quick.
#pragma once

#include <QColor>
#include <QFont>
#include <QPointer>
#include <QQuickPaintedItem>
#include <QSocketNotifier>
#include <QtQml/qqmlregistration.h>
#include <deque>
#include <vector>

extern "C" {
#include <vterm.h>
}

namespace Lumen {

class TerminalItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontChanged)
    Q_PROPERTY(qreal fontSize READ fontSize WRITE setFontSize NOTIFY fontChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QColor background READ background NOTIFY themeChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int columns READ columns NOTIFY sizeChanged)
    Q_PROPERTY(int rows READ rows NOTIFY sizeChanged)
    Q_PROPERTY(int scrollOffset READ scrollOffset WRITE setScrollOffset NOTIFY scrollChanged)
    Q_PROPERTY(int scrollbackLines READ scrollbackLines NOTIFY scrollChanged)
    Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory NOTIFY workingDirectoryChanged)
    Q_PROPERTY(QStringList command READ command WRITE setCommand)

public:
    explicit TerminalItem(QQuickItem *parent = nullptr);
    ~TerminalItem() override;

    QString fontFamily() const { return m_font.family(); }
    void setFontFamily(const QString &f);
    qreal fontSize() const { return m_font.pointSizeF() > 0 ? m_font.pointSizeF() : m_font.pixelSize(); }
    void setFontSize(qreal s);
    QString theme() const { return m_theme; }
    void setTheme(const QString &t);
    QColor background() const { return m_bg; }
    QString title() const { return m_title; }
    bool running() const { return m_pid > 0; }
    int columns() const { return m_cols; }
    int rows() const { return m_rows; }
    int scrollOffset() const { return m_scroll; }
    void setScrollOffset(int o);
    int scrollbackLines() const { return int(m_scrollback.size()); }
    QString workingDirectory() const { return m_cwd; }
    void setWorkingDirectory(const QString &d) { m_cwd = d; Q_EMIT workingDirectoryChanged(); }
    QStringList command() const { return m_command; }
    void setCommand(const QStringList &c) { m_command = c; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void copySelection();
    Q_INVOKABLE void paste();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearScrollback();
    Q_INVOKABLE void sendText(const QString &text);
    Q_INVOKABLE QString selectedText() const;
    Q_INVOKABLE QString currentDirectory() const;
    static QStringList themes();

    void paint(QPainter *painter) override;

    // libvterm callbacks
    int onDamage(VTermRect rect);
    int onMoveCursor(VTermPos pos, int visible);
    int onSetTermProp(VTermProp prop, VTermValue *val);
    int onBell();
    int onPushLine(int cols, const VTermScreenCell *cells);
    int onPopLine(int cols, VTermScreenCell *cells);

Q_SIGNALS:
    void fontChanged();
    void themeChanged();
    void titleChanged();
    void runningChanged();
    void sizeChanged();
    void scrollChanged();
    void workingDirectoryChanged();
    void finished(int exitCode);
    void bell();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void keyPressEvent(QKeyEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void focusInEvent(QFocusEvent *) override { m_focused = true; update(); }
    void focusOutEvent(QFocusEvent *) override { m_focused = false; update(); }

private:
    void readPty();
    void flushOutput();
    void writePty(const char *data, size_t len);
    void resizeTerminal();
    void updateMetrics();
    QColor toQColor(VTermColor c, bool fg) const;
    bool cellAt(int row, int col, VTermScreenCell &cell) const; // row relative to view
    QPoint cellFromPos(const QPointF &p) const;
    bool isSelected(int absRow, int col) const;
    void applyPalette();
    void childExited();

    VTerm *m_vt = nullptr;
    VTermScreen *m_screen = nullptr;
    int m_fd = -1;
    pid_t m_pid = -1;
    QSocketNotifier *m_notifier = nullptr;
    QFont m_font;
    qreal m_cellW = 8, m_cellH = 16, m_ascent = 12;
    int m_cols = 80, m_rows = 24;
    VTermPos m_cursor{0, 0};
    bool m_cursorVisible = true;
    int m_cursorShape = 1;
    bool m_focused = false;
    QString m_title = QStringLiteral("Terminal");
    QByteArray m_titleBuf;
    QString m_theme = QStringLiteral("Lumen Dark");
    QColor m_fg, m_bg, m_cursorColor, m_selection;
    QList<QColor> m_palette;
    std::deque<std::vector<VTermScreenCell>> m_scrollback;
    int m_scroll = 0;
    // selection in absolute rows (scrollback rows are negative)
    bool m_selecting = false, m_hasSel = false;
    QPoint m_selA, m_selB;
    QString m_cwd;
    QStringList m_command;
    QByteArray m_out;
};

} // namespace Lumen
