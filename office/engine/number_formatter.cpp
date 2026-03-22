#include "number_formatter.h"
#include <QLocale>
#include <QDate>
#include <QDateTime>
#include <QRegularExpression>
#include <cmath>

namespace OpenSheet {

QString NumberFormatter::format(const QVariant &value,
                                 const QString  &formatCode,
                                 QString        *outColor)
{
    if (outColor) outColor->clear();
    if (formatCode.isEmpty() || formatCode == "General") {
        // General: auto-format
        if (value.typeId() == QMetaType::Double || value.typeId() == QMetaType::Float) {
            double d = value.toDouble();
            if (d == std::floor(d) && std::abs(d) < 1e15)
                return QLocale::system().toString(static_cast<long long>(d));
            return QLocale::system().toString(d, 'g', 10);
        }
        return value.toString();
    }

    // Text passthrough
    if (formatCode == "@") return value.toString();

    // Date formats
    if (isDateFormat(formatCode)) {
        double serial = value.toDouble();
        return applyDateFormat(serial, formatCode);
    }

    // Numeric
    if (isNumericFormat(formatCode)) {
        bool ok;
        double d = value.toDouble(&ok);
        if (!ok) return value.toString();
        return applyNumericFormat(d, formatCode, outColor);
    }

    return value.toString();
}

QString NumberFormatter::applyNumericFormat(double value,
                                             const QString &code,
                                             QString *outColor)
{
    // Extract optional color like [Red] or [Blue]
    QString stripped = code;
    QString color = extractColor(code, stripped);
    if (outColor && !color.isEmpty()) *outColor = color;

    // Percentage
    if (stripped.contains('%')) {
        value *= 100.0;
        int dec = countDecimals(stripped);
        return QString::number(value, 'f', dec) + "%";
    }

    // Currency prefix — strip symbol, apply numeric format, re-add prefix
    QString prefix;
    static QRegularExpression currRe(R"(^\$|^€|^£|^¥)");
    auto cm = currRe.match(stripped);
    if (cm.hasMatch()) {
        prefix  = cm.captured(0);
        stripped = stripped.mid(prefix.length());
    }

    int  dec      = countDecimals(stripped);
    bool thousands = hasThousandsSep(stripped);

    QString result;
    if (thousands)
        result = formatWithThousands(value, dec);
    else
        result = QString::number(value, 'f', dec);

    return prefix + result;
}

QString NumberFormatter::applyDateFormat(double serialDate, const QString &code)
{
    // Excel serial: days since 1899-12-30 (accounting for the 1900 leap-year bug)
    int days = static_cast<int>(serialDate);
    if (days > 59) --days;
    QDate base(1899, 12, 30);
    QDate date = base.addDays(days);
    if (!date.isValid()) return QString::number(serialDate);

    // Fractional part = time
    double frac = serialDate - std::floor(serialDate);
    int totalSec = static_cast<int>(std::round(frac * 86400));
    QTime time = QTime(totalSec / 3600, (totalSec % 3600) / 60, totalSec % 60);

    QDateTime dt(date, time);

    // Map format code tokens to Qt format tokens
    QString qtFmt = code;
    qtFmt.replace("yyyy", "yyyy").replace("yy", "yy")
         .replace("MMMM","MMMM").replace("MMM","MMM")
         .replace("MM","MM").replace("dd","dd").replace("d","d")
         .replace("hh","hh").replace("h","h")
         .replace("mm","mm").replace("ss","ss");

    return dt.toString(qtFmt);
}

QString NumberFormatter::formatWithThousands(double value, int decimals)
{
    // Handle negative
    bool neg = value < 0;
    if (neg) value = -value;

    QString numStr = QString::number(value, 'f', decimals);
    int dotPos = numStr.indexOf('.');
    QString intPart  = (dotPos >= 0) ? numStr.left(dotPos) : numStr;
    QString fracPart = (dotPos >= 0) ? numStr.mid(dotPos)  : "";

    // Insert thousands separators
    QString result;
    int start = intPart.length() % 3;
    if (start > 0) result = intPart.left(start);
    for (int i = start; i < intPart.length(); i += 3) {
        if (!result.isEmpty()) result += ',';
        result += intPart.mid(i, 3);
    }
    return (neg ? "-" : "") + result + fracPart;
}

QString NumberFormatter::extractColor(const QString &code, QString &stripped)
{
    static QRegularExpression colorRe(R"(\[(\w+)\])", QRegularExpression::CaseInsensitiveOption);
    auto m = colorRe.match(code);
    if (!m.hasMatch()) return {};
    stripped = code;
    stripped.remove(m.capturedStart(), m.capturedLength());
    stripped = stripped.trimmed();
    return m.captured(1).toLower();
}

int NumberFormatter::countDecimals(const QString &code)
{
    int dot = code.indexOf('.');
    if (dot < 0) return 0;
    int count = 0;
    for (int i = dot + 1; i < code.length(); ++i) {
        if (code[i] == '0' || code[i] == '#') ++count;
        else break;
    }
    return count;
}

bool NumberFormatter::hasThousandsSep(const QString &code)
{
    return code.contains(',');
}

bool NumberFormatter::isDateFormat(const QString &code)
{
    static QRegularExpression dateRe(R"(\b(d{1,4}|M{1,4}|y{2,4}|h{1,2}|s{1,2})\b)",
                                     QRegularExpression::CaseInsensitiveOption);
    return dateRe.match(code).hasMatch();
}

bool NumberFormatter::isNumericFormat(const QString &code)
{
    return code.contains('0') || code.contains('#') || code.contains('%');
}

QString NumberFormatter::guessFormat(const QString &raw)
{
    if (raw.isEmpty()) return "General";
    if (raw.contains('%')) return "0.00%";

    bool ok;
    double d = raw.toDouble(&ok);
    if (ok) {
        if (d == std::floor(d))          return "General";
        if (raw.contains('.'))           return "0.00";
        if (std::abs(d) >= 1000)         return "#,##0";
    }

    // Date-like
    static QRegularExpression dateRe(R"(\d{1,4}[\/\-\.]\d{1,2}[\/\-\.]\d{2,4})");
    if (dateRe.match(raw).hasMatch())    return "dd/MM/yyyy";

    return "@";  // text
}

} // namespace OpenSheet
