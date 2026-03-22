#include "status_bar.h"
#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>

namespace OpenSheet {

StatusBar::StatusBar(QWidget *parent) : QStatusBar(parent)
{
    setSizeGripEnabled(false);
    setStyleSheet("QStatusBar { font-size: 11px; }");

    m_modeLabel  = new QLabel("Ready", this);
    m_cellLabel  = new QLabel("A1",    this);
    m_sumLabel   = new QLabel("Sum: 0",   this);
    m_avgLabel   = new QLabel("Avg: 0",   this);
    m_countLabel = new QLabel("Count: 0", this);
    m_zoom       = new QSlider(Qt::Horizontal, this);
    m_zoom->setRange(50, 200);
    m_zoom->setValue(100);
    m_zoom->setFixedWidth(90);
    m_zoom->setToolTip("Zoom");

    auto spacer = [this]() -> QLabel* {
        auto *l = new QLabel("|", this);
        l->setStyleSheet("color: #D3D1C7;");
        return l;
    };

    addWidget(m_modeLabel);
    addWidget(spacer());
    addWidget(m_cellLabel);
    addPermanentWidget(m_sumLabel);
    addPermanentWidget(spacer());
    addPermanentWidget(m_avgLabel);
    addPermanentWidget(spacer());
    addPermanentWidget(m_countLabel);
    addPermanentWidget(spacer());
    addPermanentWidget(m_zoom);
}

void StatusBar::updateSelection(int row, int col)
{
    // Column letter
    QString colStr;
    int c = col;
    while (c > 0) { --c; colStr.prepend(QChar('A' + c % 26)); c /= 26; }
    m_cellLabel->setText(colStr + QString::number(row));
}

void StatusBar::updateStats(double sum, double avg, int count)
{
    m_sumLabel->setText("Sum: "   + QString::number(sum,   'f', sum   == (int)sum   ? 0 : 2));
    m_avgLabel->setText("Avg: "   + QString::number(avg,   'f', avg   == (int)avg   ? 0 : 2));
    m_countLabel->setText("Count: " + QString::number(count));
}

void StatusBar::setMessage(const QString &msg, int timeoutMs)
{
    showMessage(msg, timeoutMs);
}

void StatusBar::setZoom(int pct)
{
    m_zoom->setValue(pct);
}

} // namespace OpenSheet
