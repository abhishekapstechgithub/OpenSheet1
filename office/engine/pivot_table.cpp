#include "pivot_table.h"
#include "sheet.h"
#include "cell.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace OpenSheet {

PivotTable::PivotTable(QObject *parent) : QObject(parent) {}

void PivotTable::setSourceRange(Sheet *sheet, const CellRange &range)
{
    m_sourceSheet = sheet;
    m_sourceRange = range;
}

void PivotTable::setRowField(const FieldDef &f) { m_rowField = f; }
void PivotTable::setColField(const FieldDef &f) { m_colField = f; }
void PivotTable::addValueField(const ValueField &f) { m_valueFields.append(f); }

bool PivotTable::build(Sheet *destSheet, int destRow, int destCol)
{
    if (!m_sourceSheet || !destSheet) {
        emit buildError("Source or destination sheet is null");
        return false;
    }
    if (!m_sourceRange.isValid()) {
        emit buildError("Source range is invalid");
        return false;
    }
    if (m_valueFields.isEmpty()) {
        emit buildError("No value fields defined");
        return false;
    }

    // ── Extract raw rows ─────────────────────────────────────────────────────
    QVector<Row> rows = extractData();
    if (rows.isEmpty()) {
        emit buildError("Source range contains no data");
        return false;
    }

    // ── Collect distinct row/col keys ────────────────────────────────────────
    QStringList rowKeys, colKeys;
    for (const Row &row : rows) {
        if (m_rowField.sourceCol > 0 && m_rowField.sourceCol <= row.size()) {
            QString k = row[m_rowField.sourceCol - 1].toString();
            if (!rowKeys.contains(k)) rowKeys.append(k);
        }
        if (m_colField.sourceCol > 0 && m_colField.sourceCol <= row.size()) {
            QString k = row[m_colField.sourceCol - 1].toString();
            if (!colKeys.contains(k)) colKeys.append(k);
        }
    }
    rowKeys.sort();
    colKeys.sort();

    // ── Aggregate values ─────────────────────────────────────────────────────
    const ValueField &vf = m_valueFields.first();

    // raw[rowKey][colKey] = list of values
    QMap<QString, QMap<QString, QVector<double>>> raw;
    for (const Row &row : rows) {
        QString rk = (m_rowField.sourceCol > 0 && m_rowField.sourceCol <= row.size())
                     ? row[m_rowField.sourceCol - 1].toString()
                     : "";
        QString ck = (m_colField.sourceCol > 0 && m_colField.sourceCol <= row.size())
                     ? row[m_colField.sourceCol - 1].toString()
                     : "";
        if (vf.sourceCol > 0 && vf.sourceCol <= row.size()) {
            bool ok;
            double d = row[vf.sourceCol - 1].toDouble(&ok);
            if (ok) raw[rk][ck].append(d);
        }
    }

    m_result.rowKeys = rowKeys;
    m_result.colKeys = colKeys;
    for (const QString &rk : rowKeys)
        for (const QString &ck : colKeys)
            if (raw.contains(rk) && raw[rk].contains(ck))
                m_result.data[rk][ck] = aggregate(raw[rk][ck], vf.func);

    // ── Write output ──────────────────────────────────────────────────────────
    int r = destRow, c = destCol;

    // Header row
    destSheet->setCell(r, c, m_rowField.label.isEmpty() ? "Row" : m_rowField.label);
    for (int ci = 0; ci < colKeys.size(); ++ci)
        destSheet->setCell(r, c + 1 + ci, colKeys[ci]);
    if (m_colTotal) destSheet->setCell(r, c + 1 + colKeys.size(), "Total");
    ++r;

    // Data rows
    for (const QString &rk : rowKeys) {
        destSheet->setCell(r, c, rk);
        double rowSum = 0;
        for (int ci = 0; ci < colKeys.size(); ++ci) {
            if (m_result.data.contains(rk) && m_result.data[rk].contains(colKeys[ci])) {
                double val = m_result.data[rk][colKeys[ci]];
                rowSum += val;
                destSheet->setCell(r, c + 1 + ci,
                    QString::number(val, 'f', val == std::floor(val) ? 0 : 2));
            }
        }
        if (m_colTotal)
            destSheet->setCell(r, c + 1 + colKeys.size(),
                QString::number(rowSum, 'f', rowSum == std::floor(rowSum) ? 0 : 2));
        ++r;
    }

    // Grand total row
    if (m_rowTotal) {
        destSheet->setCell(r, c, "Grand Total");
        double grandTotal = 0;
        for (int ci = 0; ci < colKeys.size(); ++ci) {
            double colSum = 0;
            for (const QString &rk : rowKeys)
                if (m_result.data.contains(rk) && m_result.data[rk].contains(colKeys[ci]))
                    colSum += m_result.data[rk][colKeys[ci]];
            grandTotal += colSum;
            destSheet->setCell(r, c + 1 + ci,
                QString::number(colSum, 'f', colSum == std::floor(colSum) ? 0 : 2));
        }
        if (m_colTotal)
            destSheet->setCell(r, c + 1 + colKeys.size(),
                QString::number(grandTotal, 'f', grandTotal == std::floor(grandTotal) ? 0 : 2));
    }

    emit buildComplete();
    return true;
}

QVector<PivotTable::Row> PivotTable::extractData()
{
    QVector<Row> rows;
    int startRow = m_sourceRange.top() + (m_hasHeader ? 1 : 0);

    for (int r = startRow; r <= m_sourceRange.bottom(); ++r) {
        Row row;
        bool hasData = false;
        for (int c = m_sourceRange.left(); c <= m_sourceRange.right(); ++c) {
            const Cell &cell = m_sourceSheet->cell(r, c);
            row.append(cell.value().isValid() ? cell.value() : QVariant(cell.raw()));
            if (!cell.isEmpty()) hasData = true;
        }
        if (hasData) rows.append(row);
    }
    return rows;
}

double PivotTable::aggregate(const QVector<double> &values, AggFunc func)
{
    if (values.isEmpty()) return 0.0;
    switch (func) {
    case AggFunc::Sum:
        return std::accumulate(values.begin(), values.end(), 0.0);
    case AggFunc::Count:
        return values.size();
    case AggFunc::Average:
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    case AggFunc::Min:
        return *std::min_element(values.begin(), values.end());
    case AggFunc::Max:
        return *std::max_element(values.begin(), values.end());
    case AggFunc::CountA:
        return values.size();
    }
    return 0.0;
}

} // namespace OpenSheet
