#pragma once
#include "../engine/cell_range.h"
#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>
#include <QFont>
#include <QPainter>

namespace OpenSheet {

class Sheet;

enum class ChartType { Bar, Line, Pie, Scatter, Area, Column, Donut };
enum class LegendPos { Top, Bottom, Left, Right, None };

struct ChartSeries {
    QString        name;
    CellRange      xRange;
    CellRange      yRange;
    QColor         color;
    QVector<double> xValues;
    QVector<double> yValues;
    QVector<QString> labels;
};

struct ChartAxis {
    QString title;
    double  min = 0, max = 0;
    bool    autoRange = true;
    int     gridLines = 5;
    QString numberFormat = "General";
};

struct ChartConfig {
    ChartType  type      = ChartType::Bar;
    QString    title;
    LegendPos  legend    = LegendPos::Bottom;
    ChartAxis  xAxis;
    ChartAxis  yAxis;
    bool       showDataLabels = false;
    bool       show3D         = false;
    bool       stacked        = false;
    QColor     background     = Qt::white;
    QFont      titleFont;
    QVector<ChartSeries> series;
};

/**
 * Base chart rendering class.
 * Subclasses override drawChart() for specific chart types.
 * Can render to a QWidget or export to an image.
 */
class ChartBase : public QWidget {
    Q_OBJECT
public:
    explicit ChartBase(QWidget *parent = nullptr);

    void        setConfig(const ChartConfig &cfg);
    ChartConfig config() const { return m_config; }

    // Populate series from sheet ranges
    void        loadFromSheet(Sheet *sheet);

    // Export
    QImage      toImage(int width = 800, int height = 500) const;
    bool        exportPng(const QString &path, int w = 800, int h = 500) const;
    bool exportSvg(const QString &, int = 800, int = 500) const { return false; } // SVG requires Qt6::Svg

signals:
    void configChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

    // Override in subclasses:
    virtual void drawChart(QPainter &p, const QRect &plotArea) = 0;

    // Shared utilities
    void drawTitle(QPainter &p, const QRect &area);
    void drawLegend(QPainter &p, const QRect &area);
    void drawAxes(QPainter &p, const QRect &plotArea);
    void drawGridLines(QPainter &p, const QRect &plotArea, double yMax);

    QRect computePlotArea(const QRect &widgetRect) const;
    double mapX(double val, const QRect &plot, double min, double max) const;
    double mapY(double val, const QRect &plot, double min, double max) const;
    double seriesMax() const;
    double seriesMin() const;

    static QColor defaultColor(int index);

    ChartConfig m_config;

    static const QVector<QColor> kPalette;
};

// ---- Concrete chart types ----

class BarChart : public ChartBase {
public:
    explicit BarChart(QWidget *p = nullptr) : ChartBase(p) {}
protected:
    void drawChart(QPainter &p, const QRect &plotArea) override;
};

class LineChart : public ChartBase {
public:
    explicit LineChart(QWidget *p = nullptr) : ChartBase(p) {}
protected:
    void drawChart(QPainter &p, const QRect &plotArea) override;
};

class PieChart : public ChartBase {
public:
    explicit PieChart(QWidget *p = nullptr) : ChartBase(p) {}
protected:
    void drawChart(QPainter &p, const QRect &plotArea) override;
};

class ScatterChart : public ChartBase {
public:
    explicit ScatterChart(QWidget *p = nullptr) : ChartBase(p) {}
protected:
    void drawChart(QPainter &p, const QRect &plotArea) override;
};

class AreaChart : public ChartBase {
public:
    explicit AreaChart(QWidget *p = nullptr) : ChartBase(p) {}
protected:
    void drawChart(QPainter &p, const QRect &plotArea) override;
};

/**
 * Factory: creates the correct ChartBase subclass for a given ChartType.
 */
class ChartRenderer {
public:
    static ChartBase *create(ChartType type, QWidget *parent = nullptr);
};

} // namespace OpenSheet
