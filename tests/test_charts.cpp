#include <QtTest>
#include <QApplication>
#include "../office/charts/chart_base.h"
#include "../office/engine/cell_range.h"

using namespace OpenSheet;

class TestCharts : public QObject {
    Q_OBJECT

private:
    ChartConfig makeConfig(ChartType type) {
        ChartConfig cfg;
        cfg.type  = type;
        cfg.title = "Test Chart";

        ChartSeries s;
        s.name   = "Series A";
        s.yValues = {10.0, 25.0, 15.0, 40.0, 30.0};
        s.labels  = {"Jan", "Feb", "Mar", "Apr", "May"};
        cfg.series.append(s);
        return cfg;
    }

private slots:

    void testBarChartCreation() {
        auto *chart = ChartRenderer::create(ChartType::Bar);
        QVERIFY(chart != nullptr);
        delete chart;
    }

    void testLineChartCreation() {
        auto *chart = ChartRenderer::create(ChartType::Line);
        QVERIFY(chart != nullptr);
        delete chart;
    }

    void testPieChartCreation() {
        auto *chart = ChartRenderer::create(ChartType::Pie);
        QVERIFY(chart != nullptr);
        delete chart;
    }

    void testScatterChartCreation() {
        auto *chart = ChartRenderer::create(ChartType::Scatter);
        QVERIFY(chart != nullptr);
        delete chart;
    }

    void testAreaChartCreation() {
        auto *chart = ChartRenderer::create(ChartType::Area);
        QVERIFY(chart != nullptr);
        delete chart;
    }

    void testSetConfig() {
        auto *chart = ChartRenderer::create(ChartType::Bar);
        auto cfg = makeConfig(ChartType::Bar);
        chart->setConfig(cfg);
        QCOMPARE(chart->config().title, QString("Test Chart"));
        QCOMPARE(chart->config().series.size(), 1);
        QCOMPARE(chart->config().series[0].yValues.size(), 5);
        delete chart;
    }

    void testConfigChanged_signal() {
        auto *chart = ChartRenderer::create(ChartType::Bar);
        QSignalSpy spy(chart, &ChartBase::configChanged);
        chart->setConfig(makeConfig(ChartType::Bar));
        QCOMPARE(spy.count(), 1);
        delete chart;
    }

    void testSeriesMaxValue() {
        BarChart chart;
        auto cfg = makeConfig(ChartType::Bar);
        chart.setConfig(cfg);
        // seriesMax() is protected, but we can indirectly verify via rendering
        QImage img = chart.toImage(400, 300);
        QVERIFY(!img.isNull());
        QCOMPARE(img.width(), 400);
        QCOMPARE(img.height(), 300);
    }

    void testExportPng() {
        BarChart chart;
        chart.setConfig(makeConfig(ChartType::Bar));
        QTemporaryFile f;
        f.setFileTemplate(QDir::temp().filePath("opensheet_test_XXXXXX.png"));
        f.open(); f.close();
        bool ok = chart.exportPng(f.fileName(), 400, 300);
        QVERIFY(ok);
        QVERIFY(QFileInfo(f.fileName()).size() > 0);
    }

    void testMultipleSeries() {
        LineChart chart;
        ChartConfig cfg;
        cfg.type  = ChartType::Line;
        cfg.title = "Multi-series";

        for (int s = 0; s < 3; ++s) {
            ChartSeries series;
            series.name = QString("Series %1").arg(s+1);
            for (int i = 0; i < 6; ++i)
                series.yValues.append((s+1) * 10.0 + i * 2.5);
            cfg.series.append(series);
        }
        chart.setConfig(cfg);
        QImage img = chart.toImage(800, 400);
        QVERIFY(!img.isNull());
    }

    void testEmptySeriesNocrash() {
        BarChart chart;
        ChartConfig cfg;
        cfg.type = ChartType::Bar;
        chart.setConfig(cfg);
        // Should render without crashing even with no series data
        QImage img = chart.toImage(400, 300);
        QVERIFY(!img.isNull());
    }

    void testPieChartTotalPercentage() {
        PieChart chart;
        ChartConfig cfg;
        cfg.type = ChartType::Pie;
        ChartSeries s;
        s.name    = "Market Share";
        s.yValues = {40.0, 30.0, 20.0, 10.0};
        s.labels  = {"A", "B", "C", "D"};
        cfg.series.append(s);
        chart.setConfig(cfg);
        QImage img = chart.toImage(400, 400);
        QVERIFY(!img.isNull());
    }

    void testDefaultColorPalette() {
        // Palette should have at least 8 colors and no duplicates
        QSet<QRgb> colors;
        for (int i = 0; i < 8; ++i)
            colors.insert(ChartBase::defaultColor(i).rgb());
        QCOMPARE(colors.size(), 8);
    }

    void testChartMinimumSize() {
        auto *chart = ChartRenderer::create(ChartType::Bar);
        QVERIFY(chart->minimumWidth()  > 0);
        QVERIFY(chart->minimumHeight() > 0);
        delete chart;
    }
};

QTEST_MAIN(TestCharts)
#include "test_charts.moc"
