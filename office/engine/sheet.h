#pragma once
#include "cell.h"
#include "cell_range.h"
#include <QHash>
#include <QString>
#include <QSize>
#include <QVector>
#include <functional>

namespace OpenSheet {

struct CellAddress {
    int row = 0; // 1-based
    int col = 0; // 1-based
    bool operator==(const CellAddress &o) const { return row==o.row && col==o.col; }
};
inline size_t qHash(const CellAddress &a, size_t seed = 0) {
    return qHashMulti(seed, a.row, a.col);
}

struct RowProps { double height = 20.0; bool hidden = false; };
struct ColProps { double width  = 80.0; bool hidden = false; };

struct ConditionalRule {
    enum class Type { GreaterThan, LessThan, Between, Equal, ContainsText, Top10 };
    Type     type;
    double   value1 = 0, value2 = 0;
    QString  text;
    CellFormat applyFormat;
    CellRange  range;
};

struct AutoFilter {
    bool       active = false;
    int        col    = 0;
    QStringList allowedValues;
};

class Sheet : public QObject {
    Q_OBJECT
public:
    explicit Sheet(const QString &name, QObject *parent = nullptr);

    // --- Identity ---
    QString name()               const { return m_name; }
    void    setName(const QString &n)  { m_name = n; emit nameChanged(n); }
    bool    isVisible()          const { return m_visible; }
    void    setVisible(bool v)         { m_visible = v; }

    // --- Cell access ---
    Cell       &cell(int row, int col);
    const Cell &cell(int row, int col) const;
    Cell       &cell(const CellAddress &a) { return cell(a.row, a.col); }
    void        setCell(int row, int col, const QString &raw);
    void        clearCell(int row, int col);
    bool        hasCell(int row, int col) const;

    // --- Range operations ---
    void clearRange(const CellRange &range);
    void copyRange(const CellRange &src, const CellAddress &dest);

    // --- Row / Col props ---
    RowProps &rowProps(int row);
    ColProps &colProps(int col);
    void      insertRow(int before);
    void      deleteRow(int row);
    void      insertCol(int before);
    void      deleteCol(int col);

    // --- Dimensions ---
    int  maxRow() const;
    int  maxCol() const;
    QSize usedRange() const; // {maxRow, maxCol}

    // --- Sort ---
    void sortRange(const CellRange &range, int byCol, bool ascending = true);

    // --- Filter ---
    void           setAutoFilter(const AutoFilter &f);
    AutoFilter     autoFilter() const { return m_autoFilter; }
    void           clearAutoFilter();
    bool           isRowHidden(int row) const;

    // --- Freeze ---
    void setFreezeRow(int row) { m_freezeRow = row; }
    void setFreezeCol(int col) { m_freezeCol = col; }
    int  freezeRow() const { return m_freezeRow; }
    int  freezeCol() const { return m_freezeCol; }

    // --- Conditional formatting ---
    void addConditionalRule(const ConditionalRule &rule);
    QVector<ConditionalRule> conditionalRules() const { return m_condRules; }
    std::optional<CellFormat> evalConditionalFormat(int row, int col) const;

    // --- Iteration ---
    void forEachCell(std::function<void(int,int,Cell&)> fn);

signals:
    void nameChanged(const QString &name);
    void cellChanged(int row, int col);
    void structureChanged();

private:
    QString  m_name;
    bool     m_visible = true;
    int      m_freezeRow = 0, m_freezeCol = 0;

    QHash<CellAddress, Cell>   m_cells;
    QHash<int, RowProps>       m_rowProps;
    QHash<int, ColProps>       m_colProps;
    QVector<ConditionalRule>   m_condRules;
    AutoFilter                 m_autoFilter;

    static Cell s_emptyCell; // returned for const access to missing cells
};

} // namespace OpenSheet
