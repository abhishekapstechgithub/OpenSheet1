#pragma once
#include "cell_range.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QVariant>

namespace OpenSheet {

class Sheet;

/**
 * Pivot table engine.
 *
 * Given a source data range on a Sheet, produces aggregated output
 * that can be written to a destination Sheet.
 *
 * Concept:
 *   Source:                           Output:
 *   Region | Product | Sales          Product   East  West  Total
 *   East   | Widget  | 100     →      Widget    250   180   430
 *   West   | Widget  | 180            Gadget    —     220   220
 *   East   | Widget  | 150            Grand T   400   400   800
 */
class PivotTable : public QObject {
    Q_OBJECT
public:
    enum class AggFunc { Sum, Count, Average, Min, Max, CountA };

    struct FieldDef {
        int     sourceCol;  // 1-based column index in source range
        QString label;      // display name
    };

    struct ValueField {
        int      sourceCol;
        QString  label;
        AggFunc  func = AggFunc::Sum;
    };

    explicit PivotTable(QObject *parent = nullptr);

    // Configuration
    void setSourceRange(Sheet *sheet, const CellRange &range);
    void setRowField(const FieldDef &field);
    void setColField(const FieldDef &field);
    void addValueField(const ValueField &field);
    void setHasHeaderRow(bool has) { m_hasHeader = has; }
    void setGrandTotals(bool rows, bool cols) { m_rowTotal = rows; m_colTotal = cols; }

    // Build the pivot and write output to destSheet starting at destCell
    bool build(Sheet *destSheet, int destRow, int destCol);

    // Access computed result
    struct PivotResult {
        QStringList rowKeys;   // distinct row field values
        QStringList colKeys;   // distinct column field values
        // data[rowKey][colKey] = aggregated value
        QMap<QString, QMap<QString, double>> data;
    };

    PivotResult result() const { return m_result; }

signals:
    void buildComplete();
    void buildError(const QString &error);

private:
    using Row = QVector<QVariant>;

    Sheet      *m_sourceSheet = nullptr;
    CellRange   m_sourceRange;
    FieldDef    m_rowField;
    FieldDef    m_colField;
    QVector<ValueField> m_valueFields;
    bool        m_hasHeader = true;
    bool        m_rowTotal  = true;
    bool        m_colTotal  = true;
    PivotResult m_result;

    QVector<Row> extractData();
    double aggregate(const QVector<double> &values, AggFunc func);
};

} // namespace OpenSheet
