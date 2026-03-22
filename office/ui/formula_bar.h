#pragma once
#include <QWidget>
#include <QString>

class QLineEdit;
class QLabel;
class QComboBox;

namespace OpenSheet {
class Workbook;
class Sheet;

class FormulaBar : public QWidget {
    Q_OBJECT
public:
    explicit FormulaBar(QWidget *parent = nullptr);

    void setWorkbook(Workbook *wb);
    void setSheet(Sheet *sheet);
    void setCellRef(const QString &ref);
    void setFormula(const QString &formula);
    QString formula() const;

signals:
    void formulaEdited(const QString &text);
    void cellRefNavigated(const QString &ref);
    void functionInserted(const QString &funcName);

private slots:
    void onFxClicked();
    void onFormulaReturnPressed();
    void onRefReturnPressed();

private:
    QLabel    *m_fxLabel   = nullptr;
    QLineEdit *m_refEdit   = nullptr;
    QLineEdit *m_formulaEdit = nullptr;
    Workbook  *m_workbook  = nullptr;
    Sheet     *m_sheet     = nullptr;
};

} // namespace OpenSheet
