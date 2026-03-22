#pragma once
#include <QDialog>
#include "../charts/chart_base.h"

class QListWidget;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QLabel;

namespace OpenSheet {
class Sheet;
struct CellRange;

class ChartDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChartDialog(QWidget *parent = nullptr);

    void        setSheet(Sheet *sheet);
    void        setDataRange(const CellRange &range);
    ChartConfig chartConfig() const { return m_config; }

private slots:
    void onTypeSelected(int row);
    void onRangeChanged(const QString &rangeStr);
    void onTitleChanged(const QString &t);
    void onLegendChanged(int idx);
    void onPreviewUpdate();

private:
    Sheet      *m_sheet = nullptr;
    ChartConfig m_config;

    QListWidget *m_typeList    = nullptr;
    QLineEdit   *m_rangeEdit   = nullptr;
    QLineEdit   *m_titleEdit   = nullptr;
    QComboBox   *m_legendCombo = nullptr;
    QCheckBox   *m_dataLabels  = nullptr;
    ChartBase   *m_preview     = nullptr;
    QLabel      *m_rangeStatus = nullptr;
};

} // namespace OpenSheet
