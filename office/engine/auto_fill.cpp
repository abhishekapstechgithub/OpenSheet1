#include "auto_fill.h"
#include "sheet.h"
#include "formula_parser.h"
#include <QRegularExpression>

namespace OpenSheet {

const QStringList AutoFill::kMonthNames = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"};
const QStringList AutoFill::kMonthShort = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const QStringList AutoFill::kDayNames   = {
    "Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
const QStringList AutoFill::kDayShort   = {
    "Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

// ─────────────────────────────────────────────────────────────────────────────

void AutoFill::fill(Sheet *sheet,
                     const CellRange &seedRange,
                     const CellRange &fillRange,
                     FillType type)
{
    if (!sheet || !seedRange.isValid() || !fillRange.isValid()) return;

    // Collect seed values (column-major for down fill, row-major for right fill)
    bool fillDown  = fillRange.left() >= seedRange.left() &&
                     fillRange.top()  >  seedRange.bottom();
    bool fillRight = fillRange.top()  >= seedRange.top()  &&
                     fillRange.left() >  seedRange.right();

    QVector<QString> seeds;
    if (fillDown || fillRange.top() > seedRange.top()) {
        // Collect first column of seed range
        for (int r = seedRange.top(); r <= seedRange.bottom(); ++r)
            seeds.append(sheet->cell(r, seedRange.left()).raw());
    } else {
        // Collect first row of seed range
        for (int c = seedRange.left(); c <= seedRange.right(); ++c)
            seeds.append(sheet->cell(seedRange.top(), c).raw());
    }

    if (seeds.isEmpty()) return;
    if (type == FillType::AutoDetect) type = detectType(seeds);

    // Row/col delta for formula shifting
    int dr = fillDown  ? 1 : 0;
    int dc = fillRight ? 1 : 0;

    int count = fillDown
        ? (fillRange.bottom() - fillRange.top() + 1)
        : (fillRange.right()  - fillRange.left() + 1);

    // Generate fill values
    QVector<QString> generated;
    switch (type) {
    case FillType::Series: {
        // Try numeric first
        QVector<double> nums;
        bool allNumeric = true;
        for (const auto &s : seeds) {
            bool ok; double d = s.toDouble(&ok);
            if (!ok) { allNumeric = false; break; }
            nums.append(d);
        }
        if (allNumeric)
            generated = generateNumericSeries(nums, count);
        else
            generated = generateTextSeries(seeds, count);
        break;
    }
    case FillType::FormulaFill:
        generated = generateFormulaSeries(seeds, count, dr, dc);
        break;
    case FillType::Copy:
    default:
        generated = copySeries(seeds, count);
        break;
    }

    // Write back into fillRange
    for (int i = 0; i < generated.size(); ++i) {
        int r = fillRange.top()  + (fillDown  ? i : 0);
        int c = fillRange.left() + (fillRight ? i : 0);
        if (r <= fillRange.bottom() && c <= fillRange.right())
            sheet->setCell(r, c, generated[i]);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

AutoFill::FillType AutoFill::detectType(const QVector<QString> &seeds)
{
    if (seeds.isEmpty()) return FillType::Copy;
    if (seeds.first().startsWith('=')) return FillType::FormulaFill;

    // All numeric? → Series
    bool allNum = true;
    for (const auto &s : seeds) {
        bool ok; s.toDouble(&ok);
        if (!ok) { allNum = false; break; }
    }
    if (allNum && seeds.size() >= 1) return FillType::Series;

    // Month/day names? → Series
    for (const auto &s : seeds) {
        if (kMonthNames.contains(s, Qt::CaseInsensitive)) return FillType::Series;
        if (kMonthShort.contains(s, Qt::CaseInsensitive)) return FillType::Series;
        if (kDayNames.contains(s,   Qt::CaseInsensitive)) return FillType::Series;
        if (kDayShort.contains(s,   Qt::CaseInsensitive)) return FillType::Series;
    }

    // Text + trailing number? (Item1, Item2) → Series
    static QRegularExpression textNumRe(R"(^(.+?)(\d+)$)");
    if (textNumRe.match(seeds.first()).hasMatch()) return FillType::Series;

    return FillType::Copy;
}

QVector<QString> AutoFill::generateNumericSeries(const QVector<double> &seeds, int count) const
{
    double step = detectStep(seeds);
    double last = seeds.isEmpty() ? 0.0 : seeds.last();

    QVector<QString> result;
    result.reserve(count);
    for (int i = 1; i <= count; ++i) {
        double val = last + step * i;
        // Preserve integer display if original was integer
        if (val == std::floor(val) && std::abs(val) < 1e15)
            result.append(QString::number(static_cast<long long>(val)));
        else
            result.append(QString::number(val, 'g', 10));
    }
    return result;
}

QVector<QString> AutoFill::generateTextSeries(const QVector<QString> &seeds, int count) const
{
    // Month names
    auto tryList = [&](const QStringList &list) -> QVector<QString> {
        int idx = -1;
        for (int i = 0; i < list.size(); ++i)
            if (list[i].compare(seeds.last(), Qt::CaseInsensitive) == 0) { idx = i; break; }
        if (idx < 0) return {};
        QVector<QString> r;
        for (int i = 1; i <= count; ++i)
            r.append(list[(idx + i) % list.size()]);
        return r;
    };

    for (const auto &list : {kMonthNames, kMonthShort, kDayNames, kDayShort}) {
        auto r = tryList(list);
        if (!r.isEmpty()) return r;
    }

    // Text + trailing number (Item1, Item2 → Item3…)
    static QRegularExpression re(R"(^(.*?)(\d+)$)");
    auto m = re.match(seeds.last());
    if (m.hasMatch()) {
        QString prefix = m.captured(1);
        int num = m.captured(2).toInt();
        QVector<QString> r;
        for (int i = 1; i <= count; ++i)
            r.append(prefix + QString::number(num + i));
        return r;
    }

    return copySeries(seeds, count);
}

QVector<QString> AutoFill::generateFormulaSeries(const QVector<QString> &seeds,
                                                   int count, int dr, int dc) const
{
    QVector<QString> r;
    r.reserve(count);
    for (int i = 1; i <= count; ++i) {
        const QString &seed = seeds[(i - 1) % seeds.size()];
        r.append(shiftFormula(seed, dr * i, dc * i));
    }
    return r;
}

QVector<QString> AutoFill::copySeries(const QVector<QString> &seeds, int count) const
{
    QVector<QString> r;
    r.reserve(count);
    for (int i = 0; i < count; ++i)
        r.append(seeds[i % seeds.size()]);
    return r;
}

QString AutoFill::shiftFormula(const QString &formula, int dr, int dc)
{
    if (!formula.startsWith('=')) return formula;

    // Replace each cell reference A1 → A(1+dr), B2 → (B+dc)2 etc.
    static QRegularExpression refRe(R"((\$?)([A-Za-z]{1,3})(\$?)(\d{1,7}))");
    QString result = formula;
    int offset = 0;

    auto it = refRe.globalMatch(formula);
    while (it.hasNext()) {
        auto m = it.next();
        bool absCol = m.captured(1) == "$";
        bool absRow = m.captured(3) == "$";
        QString colStr = m.captured(2).toUpper();
        int row = m.captured(4).toInt();

        int newRow = absRow ? row : qMax(1, row + dr);
        int newCol = absCol ? FormulaParser::colLetterToIndex(colStr)
                            : qMax(1, FormulaParser::colLetterToIndex(colStr) + dc);

        QString replacement = (absCol ? "$" : "") +
                              FormulaParser::indexToColLetter(newCol) +
                              (absRow ? "$" : "") +
                              QString::number(newRow);

        result.replace(m.capturedStart() + offset, m.capturedLength(), replacement);
        offset += replacement.length() - m.capturedLength();
    }
    return result;
}

double AutoFill::detectStep(const QVector<double> &values)
{
    if (values.size() < 2) return 1.0;
    double sum = 0;
    for (int i = 1; i < values.size(); ++i)
        sum += values[i] - values[i-1];
    return sum / (values.size() - 1);
}

} // namespace OpenSheet
