// SPDX-License-Identifier: GPL-3.0-or-later
// Unit tests for LUMEN core logic (animation engine, search ranking, parsers).
#include <QtTest>
#include <QSignalSpy>

#include "spring.h"
#include "motion.h"
#include "appsmodel.h"
#include "filesearch.h"
#include "sysstats.h"
#include "glass.h"

using namespace Lumen;

class TestCore : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void springSettlesWithoutOvershootWhenCritical()
    {
        Spring s;
        s.setPreset(Spring::Critical);
        s.componentComplete();
        s.snap(0);
        s.setTo(1);
        qreal maxV = 0;
        for (int i = 0; i < 400 && !s.step(1.0 / 120.0); ++i)
            maxV = qMax(maxV, s.value());
        QCOMPARE(s.value(), 1.0);
        QVERIFY2(maxV <= 1.001, "critically damped spring must not overshoot");
    }
    void springBouncyOvershoots()
    {
        Spring s;
        s.setPreset(Spring::Bouncy);
        s.componentComplete();
        s.snap(0);
        s.setTo(1);
        qreal maxV = 0;
        for (int i = 0; i < 600 && !s.step(1.0 / 60.0); ++i)
            maxV = qMax(maxV, s.value());
        QVERIFY(maxV > 1.02);
        QCOMPARE(s.value(), 1.0);
    }
    void springIsFrameRateIndependent()
    {
        auto run = [](qreal hz) {
            Spring s; s.setPreset(Spring::Smooth); s.componentComplete(); s.snap(0); s.setTo(100);
            const int frames = int(0.1 * hz);
            for (int i = 0; i < frames; ++i) s.step(1.0 / hz);
            return s.value();
        };
        QVERIFY(qAbs(run(60) - run(240)) < 0.5); // same trajectory at 60 and 240 Hz
    }
    void springRetargetKeepsVelocity()
    {
        Spring s; s.setPreset(Spring::Smooth); s.componentComplete(); s.snap(0); s.setTo(1);
        for (int i = 0; i < 6; ++i) s.step(1.0 / 60.0);
        const qreal v = s.velocity();
        QVERIFY(v > 0);
        s.setTo(2);
        QCOMPARE(s.velocity(), v);
    }
    void reducedMotionCollapsesDurations()
    {
        Motion::instance()->setReduceMotion(true);
        QVERIFY(Motion::instance()->windowOpen() <= 120);
        Motion::instance()->setReduceMotion(false);
        QCOMPARE(Motion::instance()->windowOpen(), 280);
    }
    void rubberBandIsBounded()
    {
        QVERIFY(Motion::rubberBand(10000, 100) < 100);
        QCOMPARE(Motion::rubberBand(0, 100), 0.0);
        QVERIFY(Motion::rubberBand(-50, 100) < 0);
    }
    void appMatchScoring()
    {
        const int exact = AppsModel::matchScore("finder", "Finder", "", {}, "lumen-finder");
        const int prefix = AppsModel::matchScore("fin", "Finder", "", {}, "");
        const int initials = AppsModel::matchScore("am", "Activity Monitor", "", {}, "");
        const int generic = AppsModel::matchScore("browser", "Firefox", "Web Browser", {}, "");
        const int none = AppsModel::matchScore("zzz", "Finder", "File Manager", {}, "");
        QVERIFY(exact > prefix);
        QVERIFY(prefix > initials);
        QVERIFY(initials > 0 && generic > 0);
        QCOMPARE(none, 0);
    }
    void fileRanking()
    {
        const QStringList paths{"/h/Docs/report-final.pdf", "/h/Docs/Q3 report.csv", "/h/Docs/report.txt", "/h/x/myreport"};
        const QVariantList r = FileSearch::rank(paths, "report", 10);
        QCOMPARE(r.size(), 4);
        QCOMPARE(r.first().toMap().value("name").toString(), QStringLiteral("report.txt"));
        QCOMPARE(r.last().toMap().value("name").toString(), QStringLiteral("myreport"));
    }
    void cpuLineParsing()
    {
        quint64 idle = 0, total = 0;
        QVERIFY(SysStats::parseCpu("cpu  100 0 50 800 50 0 0 0 0 0", idle, total));
        QCOMPARE(idle, quint64(850));
        QCOMPARE(total, quint64(1000));
        QVERIFY(!SysStats::parseCpu("intr 1 2 3", idle, total));
    }
    void byteFormatting()
    {
        QCOMPARE(SysStats::formatBytes(999), QStringLiteral("999 B"));
        QCOMPARE(SysStats::formatBytes(1500000), QStringLiteral("1.5 MB"));
    }
    void glassMaterialsResolve()
    {
        Glass g;
        const auto light = g.material(Glass::Regular, false);
        const auto dark = g.material(Glass::Regular, true);
        QVERIFY(light.contains("tint") && dark.contains("tint"));
        QVERIFY(dark.value("shadow").toReal() > light.value("shadow").toReal());
        const auto active = g.material(Glass::Active, false, QColor(255, 0, 0));
        QCOMPARE(active.value("tint").value<QVector4D>().x(), 1.0f);
    }
};

QTEST_GUILESS_MAIN(TestCore)
#include "tst_core.moc"
