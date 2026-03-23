#include "chart_base.h"
#include "../engine/sheet.h"
#include "../engine/cell_range.h"
#include <QPainter>
#include <QPainterPath>

#include <QImageWriter>
#include <cmath>

namespace OpenSheet {

const QVector<QColor> ChartBase::kPalette = {
    QColor(0x18, 0x5F, 0xA5),   // blue
    QColor(0x3B, 0x6D, 0x11),   // green
    QColor(0xBA, 0x75, 0x17),   // amber
    QColor(0x99, 0x35, 0x56),   // pink
    QColor(0x53, 0x4A, 0xB7),   // purple
    QColor(0xD8, 0x5A, 0x30),   // coral
    QColor(0x0F, 0x6E, 0x56),   // teal
    QColor(0xE2, 0x4B, 0x4A),   // red
};

ChartBase::ChartBase(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(300, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ChartBase::setConfig(const ChartConfig &cfg)
{
    m_config = cfg;
    update();
    emit configChanged();
}

void ChartBase::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // Background
    p.fillRect(rect(), m_config.background);

    QRect plot = computePlotArea(rect());
    drawTitle(p, rect());
    drawAxes(p, plot);
    drawGridLines(p, plot, seriesMax());
    drawChart(p, plot);
    drawLegend(p, rect());
}

void ChartBase::drawTitle(QPainter &p, const QRect &area)
{
    if (m_config.title.isEmpty()) return;
    QFont f = m_config.titleFont;
    if (f.pixelSize() < 1) f.setPointSize(13);
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::black);
    p.drawText(QRect(area.x(), area.y() + 6, area.width(), 28),
               Qt::AlignHCenter | Qt::AlignVCenter, m_config.title);
}

void ChartBase::drawLegend(QPainter &p, const QRect &area)
{
    if (m_config.legend == LegendPos::None || m_config.series.isEmpty()) return;
    int swatchSize = 12, spacing = 8, padding = 8;
    int totalW = 0;
    QFont f; f.setPointSize(9);
    p.setFont(f);
    for (const auto &s : m_config.series)
        totalW += swatchSize + spacing + p.fontMetrics().horizontalAdvance(s.name) + padding;

    int lx = area.center().x() - totalW / 2;
    int ly = area.bottom() - 24;
    for (int i = 0; i < m_config.series.size(); ++i) {
        QColor col = m_config.series[i].color.isValid()
                   ? m_config.series[i].color
                   : defaultColor(i);
        p.fillRect(lx, ly, swatchSize, swatchSize, col);
        p.setPen(Qt::black);
        lx += swatchSize + 4;
        p.drawText(lx, ly + swatchSize - 2, m_config.series[i].name);
        lx += p.fontMetrics().horizontalAdvance(m_config.series[i].name) + padding;
    }
}

void ChartBase::drawAxes(QPainter &p, const QRect &plot)
{
    p.setPen(QPen(QColor(0x44,0x44,0x41), 1));
    // Y axis
    p.drawLine(plot.left(), plot.top(), plot.left(), plot.bottom());
    // X axis
    p.drawLine(plot.left(), plot.bottom(), plot.right(), plot.bottom());

    // Y axis labels
    double maxV = seriesMax();
    if (maxV <= 0) return;
    QFont f; f.setPointSize(8); p.setFont(f);
    p.setPen(QColor(0x88,0x87,0x80));
    for (int i = 0; i <= 5; ++i) {
        double val = maxV * i / 5.0;
        int y = mapY(val, plot, 0, maxV);
        QString lbl;
        if (val >= 1000) lbl = QString::number(val / 1000.0, 'f', 1) + "k";
        else             lbl = QString::number((int)val);
        p.drawText(0, y + 5, plot.left() - 4, 14, Qt::AlignRight | Qt::AlignVCenter, lbl);
    }
}

void ChartBase::drawGridLines(QPainter &p, const QRect &plot, double yMax)
{
    if (yMax <= 0) return;
    QPen pen(QColor(0xE8, 0xE7, 0xE2), 1, Qt::DotLine);
    p.setPen(pen);
    for (int i = 1; i <= 5; ++i) {
        int y = mapY(yMax * i / 5.0, plot, 0, yMax);
        p.drawLine(plot.left(), y, plot.right(), y);
    }
}

QRect ChartBase::computePlotArea(const QRect &w) const
{
    int top    = m_config.title.isEmpty() ? 20 : 44;
    int bottom = (m_config.legend == LegendPos::None) ? 20 : 36;
    int left   = 54;
    int right  = 20;
    return w.adjusted(left, top, -right, -bottom);
}

double ChartBase::mapX(double val, const QRect &plot, double min, double max) const
{
    if (qFuzzyCompare(max, min)) return plot.left();
    return plot.left() + (val - min) / (max - min) * plot.width();
}

double ChartBase::mapY(double val, const QRect &plot, double min, double max) const
{
    if (qFuzzyCompare(max, min)) return plot.bottom();
    return plot.bottom() - (val - min) / (max - min) * plot.height();
}

double ChartBase::seriesMax() const
{
    double m = 0;
    for (const auto &s : m_config.series)
        for (double v : s.yValues) m = std::max(m, v);
    return m * 1.1;
}

double ChartBase::seriesMin() const
{
    double m = 0;
    for (const auto &s : m_config.series)
        for (double v : s.yValues) m = std::min(m, v);
    return std::min(m * 1.1, 0.0);
}

QColor ChartBase::defaultColor(int index)
{
    return kPalette[index % kPalette.size()];
}

QImage ChartBase::toImage(int w, int h) const
{
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    auto *self = const_cast<ChartBase*>(this);
    auto oldSz = self->size();
    self->resize(w, h);
    self->render(&p);
    self->resize(oldSz);
    return img;
}

bool ChartBase::exportPng(const QString &path, int w, int h) const
{
    return toImage(w, h).save(path, "PNG");
}

// ---- BarChart ----

void BarChart::drawChart(QPainter &p, const QRect &plot)
{
    if (m_config.series.isEmpty()) return;
    double maxV = seriesMax();
    if (maxV <= 0) return;

    int nGroups  = 0;
    for (const auto &s : m_config.series)
        nGroups = std::max(nGroups, (int)s.yValues.size());

    int nSeries  = m_config.series.size();
    double gapFrac = 0.25;
    double groupW  = (double)plot.width() / nGroups;
    double barW    = groupW * (1.0 - gapFrac) / nSeries;

    QFont f; f.setPointSize(8); p.setFont(f);

    for (int g = 0; g < nGroups; ++g) {
        double gx = plot.left() + g * groupW + groupW * gapFrac / 2.0;
        for (int s = 0; s < nSeries; ++s) {
            if (g >= m_config.series[s].yValues.size()) continue;
            double val = m_config.series[s].yValues[g];
            QColor col = m_config.series[s].color.isValid()
                       ? m_config.series[s].color : defaultColor(s);
            int bx  = (int)(gx + s * barW);
            int bh  = (int)((val / maxV) * plot.height());
            int by  = plot.bottom() - bh;
            QRect br(bx, by, (int)barW - 1, bh);
            p.fillRect(br, col);

            // X labels below bars (first series only)
            if (s == 0 && g < m_config.series[0].labels.size()) {
                p.setPen(QColor(0x88,0x87,0x80));
                p.drawText(bx, plot.bottom() + 3, (int)(groupW), 16,
                           Qt::AlignHCenter, m_config.series[0].labels[g]);
            }
            // Data labels on top
            if (m_config.showDataLabels) {
                p.setPen(Qt::black);
                p.drawText(br, Qt::AlignHCenter | Qt::AlignTop,
                           QString::number((int)val));
            }
        }
    }
}

// ---- LineChart ----

void LineChart::drawChart(QPainter &p, const QRect &plot)
{
    double maxV = seriesMax(), minV = seriesMin();
    if (qFuzzyCompare(maxV, minV)) return;

    for (int si = 0; si < m_config.series.size(); ++si) {
        const auto &s = m_config.series[si];
        if (s.yValues.isEmpty()) continue;
        QColor col = s.color.isValid() ? s.color : defaultColor(si);

        QPainterPath path;
        for (int i = 0; i < s.yValues.size(); ++i) {
            int n = s.yValues.size();
            double x = mapX((double)i / std::max(n - 1, 1), plot, 0.0, 1.0);
            double y = mapY(s.yValues[i], plot, minV, maxV);
            if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
        }
        QPen pen(col, 2);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);

        // Data points
        p.setBrush(col);
        for (int i = 0; i < s.yValues.size(); ++i) {
            int n = s.yValues.size();
            double x = mapX((double)i / std::max(n - 1, 1), plot, 0.0, 1.0);
            double y = mapY(s.yValues[i], plot, minV, maxV);
            p.drawEllipse(QPointF(x, y), 4, 4);
        }
    }
}

// ---- PieChart ----

void PieChart::drawChart(QPainter &p, const QRect &plot)
{
    if (m_config.series.isEmpty()) return;
    const auto &s = m_config.series[0];
    if (s.yValues.isEmpty()) return;

    double total = 0;
    for (double v : s.yValues) total += std::abs(v);
    if (total <= 0) return;

    int cx = plot.center().x(), cy = plot.center().y();
    int r  = std::min(plot.width(), plot.height()) / 2 - 10;
    QRect pieRect(cx - r, cy - r, r * 2, r * 2);

    double angle = -90.0 * 16; // QPainter uses 1/16 degrees
    for (int i = 0; i < s.yValues.size(); ++i) {
        double span  = std::abs(s.yValues[i]) / total * 360.0 * 16;
        QColor col   = defaultColor(i);
        p.setBrush(col);
        p.setPen(QPen(Qt::white, 2));
        p.drawPie(pieRect, (int)angle, (int)span);

        // Label at centroid
        double midAngle = (angle + span / 2.0) / 16.0 * M_PI / 180.0;
        int lx = cx + (int)(r * 0.65 * std::cos(midAngle));
        int ly = cy + (int)(r * 0.65 * std::sin(midAngle));
        p.setPen(Qt::white);
        QFont f; f.setPointSize(8); f.setBold(true); p.setFont(f);
        QString pct = QString::number(std::abs(s.yValues[i]) / total * 100.0, 'f', 1) + "%";
        p.drawText(QRect(lx - 24, ly - 8, 48, 16), Qt::AlignCenter, pct);
        angle += span;
    }
}

// ---- ScatterChart ----

void ScatterChart::drawChart(QPainter &p, const QRect &plot)
{
    double maxV = seriesMax(), minV = seriesMin();
    if (m_config.series.isEmpty()) return;

    for (int si = 0; si < m_config.series.size(); ++si) {
        const auto &s = m_config.series[si];
        QColor col = s.color.isValid() ? s.color : defaultColor(si);
        p.setBrush(col);
        p.setPen(QPen(col.darker(130), 1));
        for (int i = 0; i < s.yValues.size(); ++i) {
            double xv = (i < s.xValues.size()) ? s.xValues[i] : i;
            double yv = s.yValues[i];
            double x  = mapX(xv, plot, 0, (double)s.yValues.size() - 1);
            double y  = mapY(yv, plot, minV, maxV);
            p.drawEllipse(QPointF(x, y), 5, 5);
        }
    }
}

// ---- AreaChart ----

void AreaChart::drawChart(QPainter &p, const QRect &plot)
{
    double maxV = seriesMax(), minV = seriesMin();

    for (int si = 0; si < m_config.series.size(); ++si) {
        const auto &s = m_config.series[si];
        if (s.yValues.isEmpty()) continue;
        QColor col = s.color.isValid() ? s.color : defaultColor(si);

        QPainterPath path;
        int n = s.yValues.size();
        path.moveTo(mapX(0, plot, 0, std::max(n-1,1)), plot.bottom());
        for (int i = 0; i < n; ++i) {
            double x = mapX(i, plot, 0, std::max(n-1,1));
            double y = mapY(s.yValues[i], plot, minV, maxV);
            path.lineTo(x, y);
        }
        path.lineTo(mapX(n-1, plot, 0, std::max(n-1,1)), plot.bottom());
        path.closeSubpath();

        QColor fill = col;
        fill.setAlphaF(0.35);
        p.setBrush(fill);
        p.setPen(QPen(col, 2));
        p.drawPath(path);
    }
}

// ---- ChartRenderer ----

ChartBase *ChartRenderer::create(ChartType type, QWidget *parent)
{
    switch (type) {
    case ChartType::Bar:     return new BarChart(parent);
    case ChartType::Line:    return new LineChart(parent);
    case ChartType::Pie:     return new PieChart(parent);
    case ChartType::Scatter: return new ScatterChart(parent);
    case ChartType::Area:    return new AreaChart(parent);
    default:                 return new BarChart(parent);
    }
}

} // namespace OpenSheet
