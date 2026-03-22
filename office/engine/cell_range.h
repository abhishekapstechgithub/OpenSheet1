#pragma once
#include <QString>

namespace OpenSheet {

class CellRange {
public:
    CellRange() = default;
    CellRange(int top, int left, int bottom, int right)
        : m_top(top), m_left(left), m_bottom(bottom), m_right(right) {}

    int  top()    const { return m_top; }
    int  left()   const { return m_left; }
    int  bottom() const { return m_bottom; }
    int  right()  const { return m_right; }
    int  rowCount() const { return m_bottom - m_top + 1; }
    int  colCount() const { return m_right - m_left + 1; }
    bool isValid() const  { return m_top<=m_bottom && m_left<=m_right && m_top>0 && m_left>0; }

    bool contains(int row, int col) const {
        return row>=m_top && row<=m_bottom && col>=m_left && col<=m_right;
    }

    // Parse "A1:C5" style strings
    static CellRange fromString(const QString &s);
    QString toString() const;

    bool operator==(const CellRange &o) const {
        return m_top==o.m_top && m_left==o.m_left && m_bottom==o.m_bottom && m_right==o.m_right;
    }

private:
    int m_top=0, m_left=0, m_bottom=0, m_right=0;
};

} // namespace OpenSheet
