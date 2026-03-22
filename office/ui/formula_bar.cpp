// formula_bar.cpp
#include "formula_bar.h"
#include "../engine/workbook.h"
#include "../engine/sheet.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>

namespace OpenSheet {

FormulaBar::FormulaBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(30);
    auto *hl = new QHBoxLayout(this);
    hl->setSpacing(0); hl->setContentsMargins(0,0,0,0);

    m_refEdit = new QLineEdit(this);
    m_refEdit->setFixedWidth(70);
    m_refEdit->setPlaceholderText("A1");
    m_refEdit->setStyleSheet("border-right: 1px solid #D3D1C7; border-radius: 0; padding: 2px 6px;");
    connect(m_refEdit, &QLineEdit::returnPressed, this, &FormulaBar::onRefReturnPressed);

    m_fxLabel = new QLabel("fx", this);
    m_fxLabel->setFixedWidth(32);
    m_fxLabel->setAlignment(Qt::AlignCenter);
    m_fxLabel->setStyleSheet(
        "color: #185FA5; font-style: italic; font-weight: bold; "
        "border-right: 1px solid #D3D1C7; cursor: pointer;");
    m_fxLabel->setToolTip("Insert Function");

    m_formulaEdit = new QLineEdit(this);
    m_formulaEdit->setStyleSheet("border: none; border-radius: 0; padding: 2px 8px; font-size: 12px;");
    m_formulaEdit->setPlaceholderText("Enter value or formula…");
    connect(m_formulaEdit, &QLineEdit::textEdited,      this, &FormulaBar::formulaEdited);
    connect(m_formulaEdit, &QLineEdit::returnPressed,   this, &FormulaBar::onFormulaReturnPressed);

    hl->addWidget(m_refEdit);
    hl->addWidget(m_fxLabel);
    hl->addWidget(m_formulaEdit, 1);
}

void FormulaBar::setWorkbook(Workbook *wb) { m_workbook = wb; }
void FormulaBar::setSheet(Sheet *sheet)    { m_sheet    = sheet; }

void FormulaBar::setCellRef(const QString &ref)
{
    if (m_refEdit) m_refEdit->setText(ref);
}

void FormulaBar::setFormula(const QString &formula)
{
    if (m_formulaEdit) m_formulaEdit->setText(formula);
}

QString FormulaBar::formula() const
{
    return m_formulaEdit ? m_formulaEdit->text() : QString();
}

void FormulaBar::onFxClicked() { emit functionInserted(""); }

void FormulaBar::onFormulaReturnPressed()
{
    emit formulaEdited(m_formulaEdit->text());
}

void FormulaBar::onRefReturnPressed()
{
    emit cellRefNavigated(m_refEdit->text().trimmed().toUpper());
}

} // namespace OpenSheet
