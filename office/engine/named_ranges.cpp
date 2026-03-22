#include "named_ranges.h"
#include "cell_range.h"
#include "formula_parser.h"
#include <QJsonObject>
#include <QJsonArray>

namespace OpenSheet {

void NamedRanges::define(const QString &name, int sheetIndex, const CellRange &range)
{
    m_ranges[name.toUpper()] = {sheetIndex, range};
}

bool NamedRanges::resolve(const QString &name,
                           int &outSheetIndex,
                           CellRange &outRange) const
{
    auto it = m_ranges.constFind(name.toUpper());
    if (it == m_ranges.constEnd()) return false;
    outSheetIndex = it->sheetIndex;
    outRange      = it->range;
    return true;
}

void NamedRanges::remove(const QString &name)
{
    m_ranges.remove(name.toUpper());
}

QStringList NamedRanges::names() const
{
    return m_ranges.keys();
}

QJsonArray NamedRanges::toJson() const
{
    QJsonArray arr;
    for (auto it = m_ranges.constBegin(); it != m_ranges.constEnd(); ++it) {
        QJsonObject obj;
        obj["name"]       = it.key();
        obj["sheetIndex"] = it->sheetIndex;
        obj["range"]      = it->range.toString();
        arr.append(obj);
    }
    return arr;
}

void NamedRanges::fromJson(const QJsonArray &arr)
{
    m_ranges.clear();
    for (const auto &v : arr) {
        auto obj = v.toObject();
        Entry e;
        e.sheetIndex = obj["sheetIndex"].toInt();
        e.range      = CellRange::fromString(obj["range"].toString());
        m_ranges[obj["name"].toString()] = e;
    }
}

} // namespace OpenSheet
