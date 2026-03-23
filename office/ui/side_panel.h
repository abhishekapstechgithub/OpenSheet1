#pragma once
#include <QLabel>
#include <QWidget>

namespace OpenSheet {
class Workbook;

class SidePanel : public QWidget {
    Q_OBJECT
public:
    explicit SidePanel(QWidget *parent = nullptr);

    void setWorkbook(Workbook *wb);
    void updateCellInfo(int row, int col);

private:
    void buildCellInfoSection();
    void buildSheetStatsSection();
    void buildNamedRangesSection();
    void buildQuickActionsSection();

    Workbook *m_workbook = nullptr;
    QLabel *m_refLabel   = nullptr;
    QLabel *m_typeLabel  = nullptr;
    QLabel *m_valueLabel = nullptr;
    QLabel *m_formulaLabel = nullptr;
};

} // namespace OpenSheet
