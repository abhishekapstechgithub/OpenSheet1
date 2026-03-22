#pragma once
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
    class QLabel *m_refLabel   = nullptr;
    class QLabel *m_typeLabel  = nullptr;
    class QLabel *m_valueLabel = nullptr;
    class QLabel *m_formulaLabel = nullptr;
};

} // namespace OpenSheet
