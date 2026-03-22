#pragma once
#include "dependency_graph.h"
#include <QObject>
#include <QSet>

namespace OpenSheet {

class Workbook;
class Sheet;
class FormulaParser;

/**
 * RecalcEngine drives the recalculation of all formula cells
 * in the correct topological order, using the DependencyGraph
 * to resolve dependencies and detect circular references.
 *
 * Supports:
 *  - Full recalc (all sheets)
 *  - Partial recalc (single sheet or dirty set)
 *  - Circular reference reporting
 *  - Multi-pass iterative calculation (for convergence)
 */
class RecalcEngine : public QObject {
    Q_OBJECT
public:
    explicit RecalcEngine(Workbook *wb, QObject *parent = nullptr);

    // Recalculate everything
    void recalcAll();

    // Recalculate a single sheet
    void recalcSheet(Sheet *sheet, int sheetIdx);

    // Mark a cell as dirty and recalculate affected cells
    void markDirty(int sheetIdx, int row, int col);

    // Rebuild the entire dependency graph from scratch
    void rebuildGraph();

    // Returns cells currently flagged as circular
    QVector<CellKey> circularRefs() const;

signals:
    void recalcStarted();
    void recalcFinished(int cellsUpdated);
    void circularRefDetected(int sheetIdx, int row, int col);

private:
    void   extractDependencies(int sheetIdx, int row, int col,
                                const QString &formula);
    void   evalCell(int sheetIdx, int row, int col);
    void   reportCircular(const QVector<CellKey> &cycle);

    Workbook        *m_workbook;
    DependencyGraph  m_graph;
    FormulaParser   *m_parser;
    QSet<CellKey>    m_dirtySet;
    bool             m_iterativeCalc  = false;
    int              m_maxIterations  = 100;
    double           m_convergenceTol = 0.001;
};

} // namespace OpenSheet
