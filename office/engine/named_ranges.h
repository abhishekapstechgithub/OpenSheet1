#pragma once
#include "cell_range.h"
#include <QString>
#include <QStringList>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>

namespace OpenSheet {

class Sheet;

// Used by Workbook for its public API
struct NamedRange {
    QString   name;
    Sheet    *sheet = nullptr;
    CellRange range;
    QString   comment;
};

// Internal registry used by NamedRanges class (matches named_ranges.cpp)
class NamedRanges {
public:
    NamedRanges() = default;

    void define(const QString &name, int sheetIndex, const CellRange &range);
    void remove(const QString &name);
    bool resolve(const QString &name, int &outSheetIndex, CellRange &outRange) const;
    bool exists(const QString &name) const {
        return m_ranges.contains(name.toUpper());
    }

    QStringList names() const;
    QJsonArray  toJson() const;
    void        fromJson(const QJsonArray &arr);
    void        clear() { m_ranges.clear(); }

private:
    struct Entry {
        int       sheetIndex = 0;
        CellRange range;
    };
    QHash<QString, Entry> m_ranges;
};

} // namespace OpenSheet
