#include "side_panel.h"
#include "../engine/workbook.h"
#include "../engine/sheet.h"
#include "../engine/cell.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>

namespace OpenSheet {

SidePanel::SidePanel(QWidget *parent) : QWidget(parent)
{
    setFixedWidth(200);
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scroll);
    auto *vl = new QVBoxLayout(container);
    vl->setSpacing(6); vl->setContentsMargins(6,6,6,6);
    scroll->setWidget(container);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(scroll);

    // ── Cell Info ──────────────────────────────────────────────────────
    auto *cellGroup = new QGroupBox("Cell Info", container);
    auto *cgl = new QVBoxLayout(cellGroup);
    cgl->setSpacing(3);

    auto addRow = [&](const QString &label, QLabel *&valueLabel) {
        auto *hl = new QHBoxLayout;
        hl->addWidget(new QLabel(label + ":", cellGroup));
        valueLabel = new QLabel("—", cellGroup);
        valueLabel->setStyleSheet("color: #185FA5; font-weight: 500;");
        valueLabel->setWordWrap(true);
        hl->addWidget(valueLabel, 1);
        cgl->addLayout(hl);
    };

    addRow("Ref",     m_refLabel);
    addRow("Type",    m_typeLabel);
    addRow("Value",   m_valueLabel);
    addRow("Formula", m_formulaLabel);
    vl->addWidget(cellGroup);

    // ── Quick Actions ──────────────────────────────────────────────────
    auto *actGroup = new QGroupBox("Quick Actions", container);
    auto *agl = new QVBoxLayout(actGroup);
    auto makeBtn = [&](const QString &label, const QString &tooltip) {
        auto *btn = new QPushButton(label, actGroup);
        btn->setToolTip(tooltip);
        btn->setFixedHeight(26);
        agl->addWidget(btn);
        return btn;
    };
    makeBtn("Σ  AutoSum",  "Insert SUM formula");
    makeBtn("↕  Sort A→Z", "Sort selection ascending");
    makeBtn("📊  Chart",    "Insert chart");
    vl->addWidget(actGroup);

    vl->addStretch();
}

void SidePanel::setWorkbook(Workbook *wb)
{
    m_workbook = wb;
}

void SidePanel::updateCellInfo(int row, int col)
{
    if (!m_workbook) return;
    Sheet *sheet = m_workbook->activeSheet();
    if (!sheet) return;

    // Reference
    QString colStr;
    int c = col;
    while (c > 0) { --c; colStr.prepend(QChar('A' + c % 26)); c /= 26; }
    m_refLabel->setText(colStr + QString::number(row));

    const Cell &cell = sheet->cell(row, col);
    switch (cell.type()) {
    case CellType::Empty:   m_typeLabel->setText("Empty");   break;
    case CellType::Number:  m_typeLabel->setText("Number");  break;
    case CellType::Text:    m_typeLabel->setText("Text");    break;
    case CellType::Boolean: m_typeLabel->setText("Boolean"); break;
    case CellType::Date:    m_typeLabel->setText("Date");    break;
    case CellType::Formula: m_typeLabel->setText("Formula"); break;
    case CellType::Error:   m_typeLabel->setText("Error");   break;
    }

    m_valueLabel->setText(cell.displayText().left(24));
    m_formulaLabel->setText(cell.hasFormula() ? cell.raw().left(24) : "—");
}

} // namespace OpenSheet
