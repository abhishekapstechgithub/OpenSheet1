#pragma once
#include <QString>
#include <QVariant>

namespace OpenSheet {

/**
 * Converts a raw numeric QVariant to a display string using an
 * Excel-style number format pattern.
 *
 * Supported format codes:
 *   General          auto (integer or decimal, no trailing zeros)
 *   0                integer, pad with zeros
 *   0.00             two decimal places
 *   #,##0            thousands separator
 *   #,##0.00         thousands separator + 2 decimals
 *   0%               percentage (value * 100)
 *   0.00%            percentage with decimals
 *   $#,##0.00        currency prefix
 *   dd/MM/yyyy       short date
 *   dd MMM yyyy      medium date
 *   hh:mm:ss         time
 *   @                text passthrough
 *   [Red]0.00        conditional color (returns color name in outColor)
 */
class NumberFormatter {
public:
    NumberFormatter() = default;

    // Format a value. Returns the display string.
    // outColor (optional): set to a color name like "red" if format specifies one.
    static QString format(const QVariant &value,
                          const QString  &formatCode,
                          QString        *outColor = nullptr);

    // Guess a reasonable format code for a raw cell string
    static QString guessFormat(const QString &raw);

    // Returns true if formatCode is a date/time format
    static bool isDateFormat(const QString &formatCode);

    // Returns true if formatCode is a numeric format
    static bool isNumericFormat(const QString &formatCode);

private:
    static QString applyNumericFormat(double value, const QString &code,
                                      QString *outColor);
    static QString applyDateFormat(double serialDate, const QString &code);
    static QString formatInteger(long long value, const QString &code);
    static QString formatWithThousands(double value, int decimals);
    static QString extractColor(const QString &code, QString &strippedCode);
    static int     countDecimals(const QString &code);
    static bool    hasThousandsSep(const QString &code);
};

} // namespace OpenSheet
