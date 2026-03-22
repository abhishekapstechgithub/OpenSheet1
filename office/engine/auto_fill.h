#pragma once
#include "cell_range.h"
#include <QVariant>
#include <QVector>

namespace OpenSheet {

class Sheet;

/**
 * Implements Excel-style Auto Fill:
 *   - Numeric series:  1, 2, 3 → 4, 5, 6
 *   - Date series:     Jan, Feb → Mar, Apr  /  Mon, Tue → Wed
 *   - Text+num:        Item1, Item2 → Item3
 *   - Formula fill:    =A1*2, dragged down → adjusts references
 *   - Copy fill:       non-series content is simply repeated
 */
class AutoFill {
public:
    enum class FillType {
        AutoDetect,
        Copy,
        Series,
        FormulaFill,
        FlashFill   // pattern-based (future)
    };

    AutoFill() = default;

    /**
     * Fill cells in fillRange based on the seed values in seedRange.
     * fillDir: 0=right, 1=down, 2=left, 3=up
     */
    void fill(Sheet *sheet,
              const CellRange &seedRange,
              const CellRange &fillRange,
              FillType type = FillType::AutoDetect);

    // Detect what kind of series the seed values represent
    static FillType detectType(const QVector<QString> &seedValues);

private:
    // Series generators
    QVector<QString> generateNumericSeries(const QVector<double> &seeds, int count) const;
    QVector<QString> generateDateSeries(const QVector<QString> &seeds, int count) const;
    QVector<QString> generateTextSeries(const QVector<QString> &seeds, int count) const;
    QVector<QString> generateFormulaSeries(const QVector<QString> &seeds,
                                            int count, int dr, int dc) const;
    QVector<QString> copySeries(const QVector<QString> &seeds, int count) const;

    // Adjust a formula's cell references by dr rows and dc columns
    static QString shiftFormula(const QString &formula, int dr, int dc);

    // Detect numeric step from seed values
    static double detectStep(const QVector<double> &values);

    static const QStringList kMonthNames;
    static const QStringList kMonthShort;
    static const QStringList kDayNames;
    static const QStringList kDayShort;
};

} // namespace OpenSheet
