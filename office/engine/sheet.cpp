#include "sheet.h"
#include <algorithm>

namespace OpenSheet {

Cell Sheet::s_emptyCell;

Sheet::Sheet(const QString &name, QObject *parent)
    : QObject(parent), m_name(name) {}

Cell &Sheet::cell(int row, int col)
{
    CellAddress addr{row, col};
    return m_cells[addr]; // inserts default Cell if missing
}

const Cell &Sheet::cell(int row, int col) const
{
    CellAddress addr{row, col};
    auto it = m_cells.constFind(addr);
    return (it != m_cells.constEnd()) ? it.value() : s_emptyCell;
}

void Sheet::setCell(int row, int col, const QString &raw)
{
    CellAddress addr{row, col};
    if (raw.isEmpty()) {
        m_cells.remove(addr);
    } else {
        m_cells[addr].setRaw(raw);
    }
    emit cellChanged(row, col);
}

void Sheet::clearCell(int row, int col)
{
    m_cells.remove(CellAddress{row, col});
    emit cellChanged(row, col);
}

bool Sheet::hasCell(int row, int col) const
{
    return m_cells.contains(CellAddress{row, col});
}

void Sheet::clearRange(const CellRange &range)
{
    for (int r = range.top(); r <= range.bottom(); ++r)
        for (int c = range.left(); c <= range.right(); ++c)
            m_cells.remove(CellAddress{r, c});
    emit structureChanged();
}

void Sheet::copyRange(const CellRange &src, const CellAddress &dest)
{
    int dr = dest.row - src.top();
    int dc = dest.col - src.left();
    QHash<CellAddress, Cell> copied;
    for (int r = src.top(); r <= src.bottom(); ++r)
        for (int c = src.left(); c <= src.right(); ++c)
            if (hasCell(r, c))
                copied[CellAddress{r+dr, c+dc}] = m_cells[CellAddress{r,c}];
    for (auto it = copied.begin(); it != copied.end(); ++it)
        m_cells[it.key()] = it.value();
    emit structureChanged();
}

RowProps &Sheet::rowProps(int row) { return m_rowProps[row]; }
ColProps &Sheet::colProps(int col) { return m_colProps[col]; }

void Sheet::insertRow(int before)
{
    QHash<CellAddress, Cell> updated;
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it) {
        CellAddress a = it.key();
        if (a.row >= before) a.row++;
        updated[a] = it.value();
    }
    m_cells = updated;
    emit structureChanged();
}

void Sheet::deleteRow(int row)
{
    QHash<CellAddress, Cell> updated;
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it) {
        CellAddress a = it.key();
        if (a.row == row) continue;
        if (a.row > row) a.row--;
        updated[a] = it.value();
    }
    m_cells = updated;
    emit structureChanged();
}

void Sheet::insertCol(int before)
{
    QHash<CellAddress, Cell> updated;
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it) {
        CellAddress a = it.key();
        if (a.col >= before) a.col++;
        updated[a] = it.value();
    }
    m_cells = updated;
    emit structureChanged();
}

void Sheet::deleteCol(int col)
{
    QHash<CellAddress, Cell> updated;
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it) {
        CellAddress a = it.key();
        if (a.col == col) continue;
        if (a.col > col) a.col--;
        updated[a] = it.value();
    }
    m_cells = updated;
    emit structureChanged();
}

int Sheet::maxRow() const
{
    int m = 0;
    for (auto it = m_cells.constBegin(); it != m_cells.constEnd(); ++it)
        m = std::max(m, it.key().row);
    return m;
}

int Sheet::maxCol() const
{
    int m = 0;
    for (auto it = m_cells.constBegin(); it != m_cells.constEnd(); ++it)
        m = std::max(m, it.key().col);
    return m;
}

QSize Sheet::usedRange() const { return {maxRow(), maxCol()}; }

void Sheet::sortRange(const CellRange &range, int byCol, bool ascending)
{
    // Collect rows as vectors
    QVector<QVector<Cell>> rows;
    for (int r = range.top(); r <= range.bottom(); ++r) {
        QVector<Cell> row;
        for (int c = range.left(); c <= range.right(); ++c)
            row.append(m_cells.value(CellAddress{r, c}));
        rows.append(row);
    }

    int sortColIdx = byCol - range.left();
    std::stable_sort(rows.begin(), rows.end(), [&](const QVector<Cell>&a, const QVector<Cell>&b){
        if (sortColIdx < 0 || sortColIdx >= a.size()) return false;
        const QVariant av = a[sortColIdx].value();
        const QVariant bv = b[sortColIdx].value();
        if (av.typeId() == QMetaType::Double && bv.typeId() == QMetaType::Double)
            return ascending ? av.toDouble() < bv.toDouble() : av.toDouble() > bv.toDouble();
        return ascending ? av.toString() < bv.toString() : av.toString() > bv.toString();
    });

    for (int ri = 0; ri < rows.size(); ++ri)
        for (int ci = 0; ci < rows[ri].size(); ++ci)
            m_cells[CellAddress{range.top()+ri, range.left()+ci}] = rows[ri][ci];

    emit structureChanged();
}

void Sheet::setAutoFilter(const AutoFilter &f) { m_autoFilter = f; emit structureChanged(); }
void Sheet::clearAutoFilter() { m_autoFilter = {}; emit structureChanged(); }

bool Sheet::isRowHidden(int row) const
{
    if (!m_autoFilter.active) return m_rowProps.value(row).hidden;
    if (!hasCell(row, m_autoFilter.col)) return !m_autoFilter.allowedValues.isEmpty();
    QString val = m_cells.value(CellAddress{row, m_autoFilter.col}).displayText();
    return !m_autoFilter.allowedValues.isEmpty() && !m_autoFilter.allowedValues.contains(val);
}

void Sheet::addConditionalRule(const ConditionalRule &rule) { m_condRules.append(rule); }

std::optional<CellFormat> Sheet::evalConditionalFormat(int row, int col) const
{
    for (const auto &rule : m_condRules) {
        if (!rule.range.contains(row, col)) continue;
        const Cell &c = cell(row, col);
        double v = c.value().toDouble();
        bool match = false;
        switch (rule.type) {
        case ConditionalRule::Type::GreaterThan: match = v > rule.value1; break;
        case ConditionalRule::Type::LessThan:    match = v < rule.value1; break;
        case ConditionalRule::Type::Between:     match = v >= rule.value1 && v <= rule.value2; break;
        case ConditionalRule::Type::Equal:       match = qFuzzyCompare(v, rule.value1); break;
        case ConditionalRule::Type::ContainsText:
            match = c.displayText().contains(rule.text, Qt::CaseInsensitive); break;
        default: break;
        }
        if (match) return rule.applyFormat;
    }
    return std::nullopt;
}

void Sheet::forEachCell(std::function<void(int,int,Cell&)> fn)
{
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it)
        fn(it.key().row, it.key().col, it.value());
}

} // namespace OpenSheet
