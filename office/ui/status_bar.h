#pragma once
#include <QLabel>
#include <QSlider>
#include <QStatusBar>

namespace OpenSheet {

class StatusBar : public QStatusBar {
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = nullptr);

    void updateSelection(int row, int col);
    void updateStats(double sum, double avg, int count);
    void setMessage(const QString &msg, int timeoutMs = 3000);
    void setZoom(int pct);

private:
    QLabel *m_cellLabel  = nullptr;
    QLabel *m_sumLabel   = nullptr;
    QLabel *m_avgLabel   = nullptr;
    QLabel *m_countLabel = nullptr;
    QLabel *m_modeLabel  = nullptr;
    QSlider *m_zoom      = nullptr;
};

} // namespace OpenSheet
