#pragma once
#include <QSet>
#include <QHash>
#include <QString>
#include <QVector>
#include <functional>

namespace OpenSheet {

struct CellKey {
    int sheetIdx;
    int row;
    int col;
    bool operator==(const CellKey &o) const { return sheetIdx==o.sheetIdx && row==o.row && col==o.col; }
};
inline size_t qHash(const CellKey &k, size_t seed=0) { return qHashMulti(seed,k.sheetIdx,k.row,k.col); }

/**
 * Tracks which cells depend on which other cells.
 * Used by the recalc engine to determine calculation order
 * and detect circular references.
 */
class DependencyGraph {
public:
    // Record that 'dependent' references 'dependency'
    void addEdge(const CellKey &dependent, const CellKey &dependency);
    void removeEdgesFor(const CellKey &dependent);
    void clear();

    // Return all cells that depend on 'key' (direct + transitive)
    QVector<CellKey> dependentsOf(const CellKey &key) const;

    // Topological sort for recalc order; returns false if cycle detected
    bool topoSort(const QVector<CellKey> &dirtySet,
                  QVector<CellKey> &outOrder) const;

    // Returns cells involved in circular references
    QVector<CellKey> findCircularRefs() const;

    bool hasDependents(const CellKey &key) const;

private:
    // dependency -> set of dependents
    QHash<CellKey, QSet<CellKey>> m_graph;
    // dependent  -> set of dependencies (reverse, for fast removal)
    QHash<CellKey, QSet<CellKey>> m_reverse;

    void dfs(const CellKey &node,
             QSet<CellKey> &visited,
             QSet<CellKey> &inStack,
             QVector<CellKey> &order,
             bool &hasCycle) const;
};

} // namespace OpenSheet
