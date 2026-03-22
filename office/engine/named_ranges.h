#pragma once
#include "cell_range.h"
#include <QString>
#include <QHash>

namespace OpenSheet {

class Sheet;

struct NamedRange {
    QString    name;
    Sheet     *sheet = nullptr;
    CellRange  range;
    QString    comment;
    bool       readOnly = false;
};

class NamedRangeManager {
public:
    NamedRangeManager() = default;

    void   define(const QString &name, Sheet *sheet, const CellRange &range,
                  const QString &comment = {});
    void   remove(const QString &name);
    bool   resolve(const QString &name, Sheet *&outSheet, CellRange &outRange) const;
    bool   exists(const QString &name) const;

    QVector<NamedRange> allRanges() const;
    void                clear();

    // Validate: name must start with letter, no spaces, not a cell address
    static bool isValidName(const QString &name);

private:
    QHash<QString, NamedRange> m_ranges;
};

} // namespace OpenSheet
