// SPDX-License-Identifier: GPL-3.0-or-later
// Unit tests for app back-end parsers (no system services required).
#include <QtTest>
#include "../../applications/activity-monitor/src/processmodel.h"
#include "../../applications/store/src/flatpak.h"
#include "../../applications/settings/src/backends.h"
#include "../../applications/utilities/texteditor/src/document.h"

using namespace Lumen;

class TestApps : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void procStatWithSpacesInName()
    {
        ProcInfo p;
        const QByteArray stat = "1234 (Web Content (x)) S 1 1234 1234 0 -1 4194560 100 0 0 0 250 50 0 0 20 0 7 0 1000 100000 300 18446744073709551615";
        QVERIFY(ProcessModel::parseStat(stat, p));
        QCOMPARE(p.pid, 1234);
        QCOMPARE(p.name, QStringLiteral("Web Content (x)"));
        QCOMPARE(p.state, 'S');
        QCOMPARE(p.ppid, 1);
        QCOMPARE(p.ticks, quint64(300));
        QCOMPARE(p.threads, 7);
    }
    void flatpakColumns()
    {
        const auto l = Flatpak::parseColumns("Firefox\torg.mozilla.firefox\t140.0\n\nGIMP\torg.gimp.GIMP\t3.0\n",
                                             {"name", "appId", "version"});
        QCOMPARE(l.size(), 2);
        QCOMPARE(l[1].toMap().value("appId").toString(), QStringLiteral("org.gimp.GIMP"));
    }
    void flatpakProgress()
    {
        QCOMPARE(Flatpak::parseProgress("Installing… 12%  …  57%"), 57);
        QCOMPARE(Flatpak::parseProgress("no numbers"), -1);
    }
    void xkbLayoutList()
    {
        const auto l = XkbLayouts::parse("! model\n  pc105 Generic\n\n! layout\n  us              English (US)\n  de              German\n\n! variant\n  dvorak us: English (Dvorak)\n");
        QCOMPARE(l.size(), 2);
        QCOMPARE(l[1].toMap().value("code").toString(), QStringLiteral("de"));
    }
    void passwordHashIsSha512Crypt()
    {
        const QString h = Accounts::hashPassword(QStringLiteral("correct horse"));
        QVERIFY(h.startsWith(QStringLiteral("$6$")));
        QVERIFY(h != Accounts::hashPassword(QStringLiteral("correct horse"))); // random salt
    }
    void wordCount()
    {
        QCOMPARE(Document::countWords(QStringLiteral("  hello   world\nfrom\tLUMEN ")), 4);
        QCOMPARE(Document::countWords(QString()), 0);
    }
};

QTEST_GUILESS_MAIN(TestApps)
#include "tst_apps.moc"
