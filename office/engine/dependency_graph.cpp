#include "dependency_graph.h"

namespace OpenSheet {

void DependencyGraph::addEdge(const CellKey &dependent, const CellKey &dependency)
{
    m_graph[dependency].insert(dependent);
    m_reverse[dependent].insert(dependency);
}

void DependencyGraph::removeEdgesFor(const CellKey &dependent)
{
    // Remove from forward graph for each dependency this cell had
    auto it = m_reverse.find(dependent);
    if (it != m_reverse.end()) {
        for (const CellKey &dep : it.value())
            m_graph[dep].remove(dependent);
        m_reverse.erase(it);
    }
}

void DependencyGraph::clear()
{
    m_graph.clear();
    m_reverse.clear();
}

QVector<CellKey> DependencyGraph::dependentsOf(const CellKey &key) const
{
    QVector<CellKey> result;
    QSet<CellKey>    visited;
    QVector<CellKey> queue;
    queue.append(key);

    while (!queue.isEmpty()) {
        CellKey cur = queue.takeFirst();
        if (visited.contains(cur)) continue;
        visited.insert(cur);

        auto it = m_graph.constFind(cur);
        if (it == m_graph.constEnd()) continue;
        for (const CellKey &dep : it.value()) {
            result.append(dep);
            queue.append(dep);
        }
    }
    return result;
}

bool DependencyGraph::hasDependents(const CellKey &key) const
{
    auto it = m_graph.constFind(key);
    return it != m_graph.constEnd() && !it->isEmpty();
}

bool DependencyGraph::topoSort(const QVector<CellKey> &dirtySet,
                                QVector<CellKey> &outOrder) const
{
    outOrder.clear();
    QSet<CellKey> visited;
    QSet<CellKey> inStack;
    bool hasCycle = false;

    // DFS from every dirty cell
    for (const CellKey &start : dirtySet) {
        if (!visited.contains(start))
            dfs(start, visited, inStack, outOrder, hasCycle);
    }

    return !hasCycle;
}

QVector<CellKey> DependencyGraph::findCircularRefs() const
{
    QVector<CellKey> order;
    QSet<CellKey>    visited;
    QSet<CellKey>    inStack;
    bool hasCycle = false;

    QVector<CellKey> circular;
    for (auto it = m_graph.constBegin(); it != m_graph.constEnd(); ++it) {
        if (!visited.contains(it.key()))
            dfs(it.key(), visited, inStack, order, hasCycle);
    }

    if (!hasCycle) return {};

    // Re-run and collect cycle participants (cells that were in stack when revisited)
    visited.clear(); inStack.clear(); order.clear();
    for (auto it = m_graph.constBegin(); it != m_graph.constEnd(); ++it) {
        if (!visited.contains(it.key()) && inStack.contains(it.key()))
            circular.append(it.key());
    }
    return circular;
}

void DependencyGraph::dfs(const CellKey &node,
                           QSet<CellKey> &visited,
                           QSet<CellKey> &inStack,
                           QVector<CellKey> &order,
                           bool &hasCycle) const
{
    visited.insert(node);
    inStack.insert(node);

    auto it = m_graph.constFind(node);
    if (it != m_graph.constEnd()) {
        for (const CellKey &neighbor : it.value()) {
            if (!visited.contains(neighbor)) {
                dfs(neighbor, visited, inStack, order, hasCycle);
            } else if (inStack.contains(neighbor)) {
                hasCycle = true;
            }
        }
    }

    inStack.remove(node);
    order.prepend(node);   // post-order → prepend gives topo order
}

} // namespace OpenSheet
