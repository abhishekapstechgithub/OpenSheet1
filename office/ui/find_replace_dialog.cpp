#include "find_replace_dialog.h"
#include "../engine/workbook.h"
#include "../engine/sheet.h"
#include "../engine/cell.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpression>

namespace OpenSheet {

FindReplaceDialog::FindReplaceDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Find and Replace");
    setMinimumWidth(420);
    setModal(false);

    auto *vl = new QVBoxLayout(this);
    vl->setSpacing(10);

    // Search field
    auto *findRow = new QHBoxLayout;
    findRow->addWidget(new QLabel("Find:", this));
    m_findEdit = new QLineEdit(this);
    m_findEdit->setPlaceholderText("Search text or formula…");
    findRow->addWidget(m_findEdit, 1);
    vl->addLayout(findRow);

    // Replace field
    auto *repRow = new QHBoxLayout;
    repRow->addWidget(new QLabel("Replace:", this));
    m_replaceEdit = new QLineEdit(this);
    m_replaceEdit->setPlaceholderText("Replacement text…");
    repRow->addWidget(m_replaceEdit, 1);
    m_replaceRow = new QWidget(this);
    m_replaceRow->setLayout(repRow);
    vl->addWidget(m_replaceRow);

    // Options
    auto *optBox = new QGroupBox("Options", this);
    auto *optGrid = new QGridLayout(optBox);
    m_caseSensitive = new QCheckBox("Match case", this);
    m_wholeCell     = new QCheckBox("Entire cell contents", this);
    m_useRegex      = new QCheckBox("Regular expression", this);
    m_allSheets     = new QCheckBox("Search all sheets", this);
    optGrid->addWidget(m_caseSensitive, 0, 0);
    optGrid->addWidget(m_wholeCell,     0, 1);
    optGrid->addWidget(m_useRegex,      1, 0);
    optGrid->addWidget(m_allSheets,     1, 1);
    vl->addWidget(optBox);

    // Status label
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("color: #888780; font-size: 11px;");
    vl->addWidget(m_statusLabel);

    // Buttons
    auto *btnRow = new QHBoxLayout;
    auto *prevBtn    = new QPushButton("← Previous", this);
    auto *nextBtn    = new QPushButton("Find Next →", this);
    auto *replBtn    = new QPushButton("Replace", this);
    auto *replAllBtn = new QPushButton("Replace All", this);
    auto *closeBtn   = new QPushButton("Close", this);
    nextBtn->setDefault(true);
    btnRow->addWidget(prevBtn);
    btnRow->addWidget(nextBtn);
    btnRow->addStretch();
    btnRow->addWidget(replBtn);
    btnRow->addWidget(replAllBtn);
    btnRow->addWidget(closeBtn);
    vl->addLayout(btnRow);

    connect(nextBtn,    &QPushButton::clicked, this, &FindReplaceDialog::onFindNext);
    connect(prevBtn,    &QPushButton::clicked, this, &FindReplaceDialog::onFindPrev);
    connect(replBtn,    &QPushButton::clicked, this, &FindReplaceDialog::onReplace);
    connect(replAllBtn, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAll);
    connect(closeBtn,   &QPushButton::clicked, this, &FindReplaceDialog::onClose);
    connect(m_findEdit, &QLineEdit::textChanged, this, [this]{ m_matchIdx = 0; });
}

void FindReplaceDialog::setWorkbook(Workbook *wb) { m_workbook = wb; }
void FindReplaceDialog::setCurrentSheet(Sheet *s)  { m_sheet = s; }

void FindReplaceDialog::openFind()
{
    m_replaceRow->hide();
    setWindowTitle("Find");
    adjustSize();
    show();
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void FindReplaceDialog::openFindReplace()
{
    m_replaceRow->show();
    setWindowTitle("Find and Replace");
    adjustSize();
    show();
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void FindReplaceDialog::onFindNext()
{
    if (m_findEdit->text().isEmpty()) return;
    auto matches = findAllMatches();
    if (matches.isEmpty()) {
        m_statusLabel->setText("No matches found.");
        return;
    }
    m_matchIdx = (m_matchIdx + 1) % matches.size();
    auto &m = matches[m_matchIdx];
    m_statusLabel->setText(QString("Match %1 of %2").arg(m_matchIdx + 1).arg(matches.size()));
    emit cellFound(m.row, m.col);
}

void FindReplaceDialog::onFindPrev()
{
    if (m_findEdit->text().isEmpty()) return;
    auto matches = findAllMatches();
    if (matches.isEmpty()) { m_statusLabel->setText("No matches found."); return; }
    m_matchIdx = (m_matchIdx - 1 + matches.size()) % matches.size();
    auto &m = matches[m_matchIdx];
    m_statusLabel->setText(QString("Match %1 of %2").arg(m_matchIdx + 1).arg(matches.size()));
    emit cellFound(m.row, m.col);
}

void FindReplaceDialog::onReplace()
{
    auto matches = findAllMatches();
    if (matches.isEmpty() || m_matchIdx >= matches.size()) { onFindNext(); return; }
    auto &match = matches[m_matchIdx];
    if (!m_sheet) return;
    Cell &cell = m_sheet->cell(match.row, match.col);
    QString newRaw = cell.raw().replace(m_findEdit->text(),
                                        m_replaceEdit->text(),
                                        m_caseSensitive->isChecked()
                                            ? Qt::CaseSensitive
                                            : Qt::CaseInsensitive);
    m_sheet->setCell(match.row, match.col, newRaw);
    emit replacementsMade(1);
    onFindNext();
}

void FindReplaceDialog::onReplaceAll()
{
    if (!m_sheet || m_findEdit->text().isEmpty()) return;
    auto matches = findAllMatches();
    int count = 0;
    for (const auto &m : matches) {
        Cell &cell = m_sheet->cell(m.row, m.col);
        QString newRaw = cell.raw().replace(m_findEdit->text(),
                                            m_replaceEdit->text(),
                                            m_caseSensitive->isChecked()
                                                ? Qt::CaseSensitive
                                                : Qt::CaseInsensitive);
        m_sheet->setCell(m.row, m.col, newRaw);
        ++count;
    }
    m_statusLabel->setText(QString("%1 replacement(s) made.").arg(count));
    emit replacementsMade(count);
}

void FindReplaceDialog::onClose() { hide(); }

QVector<FindReplaceDialog::Match> FindReplaceDialog::findAllMatches()
{
    QVector<Match> results;
    if (!m_sheet) return results;

    m_sheet->forEachCell([&](int r, int c, Cell &cell) {
        if (matchesQuery(cell.displayText()) || matchesQuery(cell.raw()))
            results.append({r, c});
    });
    return results;
}

bool FindReplaceDialog::matchesQuery(const QString &text) const
{
    QString query = m_findEdit->text();
    if (query.isEmpty()) return false;

    if (m_useRegex->isChecked()) {
        QRegularExpression re(query,
            m_caseSensitive->isChecked()
                ? QRegularExpression::NoPatternOption
                : QRegularExpression::CaseInsensitiveOption);
        return re.match(text).hasMatch();
    }

    Qt::CaseSensitivity cs = m_caseSensitive->isChecked()
        ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (m_wholeCell->isChecked())
        return text.compare(query, cs) == 0;

    return text.contains(query, cs);
}

} // namespace OpenSheet
