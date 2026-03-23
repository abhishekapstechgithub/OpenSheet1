#pragma once
#include "dependency_graph.h"
#include <QObject>
#include <QSet>
#include <unordered_set>
#include <string>

namespace OpenSheet {

class Workbook;
class Sheet;
class FormulaParser;

class RecalcEngine : public QObject {
    Q_OBJECT
public:
    explicit RecalcEngine(QObject *parent = nullptr) : QObject(parent) {}

    // Recalculate a single sheet
    void recalcSheet(Sheet *sheet, Workbook *wb);

    // Incremental: recalculate cells that depend on (row,col)
    void recalcDependents(Sheet *sheet, Workbook *wb, int row, int col);

    // Rebuild the dependency graph for a sheet
    void rebuildGraph(Sheet *sheet, Workbook *wb);

    // Returns cells currently flagged as circular
    QVector<CellKey> circularRefs() const { return m_graph.findCircularRefs(); }

signals:
    void recalcStarted();
    void recalcFinished(int cellsUpdated);
    void circularRefDetected(int sheetIdx, int row, int col);

private:
    void evaluateCell(Sheet *sheet, Workbook *wb, int row, int col);
    void parseAndRegisterDependencies(Sheet *sheet, Workbook *wb,
                                       int row, int col,
                                       const QString &formula);

    DependencyGraph m_graph;
};

} // namespace OpenSheet
