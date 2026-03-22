#include "cell_range.h"
#include "formula_parser.h"
#include <QRegularExpression>

namespace OpenSheet {

CellRange CellRange::fromString(const QString &s)
{
    // Accept "A1:C5"  or  "A1" (single cell)
    static QRegularExpression re(
        R"(^([A-Za-z]+)(\d+)(?::([A-Za-z]+)(\d+))?$)");
    auto m = re.match(s.trimmed().toUpper());
    if (!m.hasMatch()) return {};

    int c1 = FormulaParser::colLetterToIndex(m.captured(1));
    int r1 = m.captured(2).toInt();

    if (m.captured(3).isEmpty()) {
        // Single cell
        return CellRange(r1, c1, r1, c1);
    }

    int c2 = FormulaParser::colLetterToIndex(m.captured(3));
    int r2 = m.captured(4).toInt();
    return CellRange(std::min(r1,r2), std::min(c1,c2),
                     std::max(r1,r2), std::max(c1,c2));
}

QString CellRange::toString() const
{
    if (!isValid()) return {};
    QString tl = FormulaParser::indexToColLetter(m_left)  + QString::number(m_top);
    QString br = FormulaParser::indexToColLetter(m_right) + QString::number(m_bottom);
    if (tl == br) return tl;
    return tl + ":" + br;
}

} // namespace OpenSheet
