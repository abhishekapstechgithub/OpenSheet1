#pragma once
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
    class QLabel *m_cellLabel  = nullptr;
    class QLabel *m_sumLabel   = nullptr;
    class QLabel *m_avgLabel   = nullptr;
    class QLabel *m_countLabel = nullptr;
    class QLabel *m_modeLabel  = nullptr;
    class QSlider *m_zoom      = nullptr;
};

} // namespace OpenSheet
