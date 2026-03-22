#include "recalc_engine.h"
#include "sheet.h"
#include "workbook.h"
#include "cell.h"
#include "formula_parser.h"
#include "cell_range.h"
#include <QRegularExpression>

namespace OpenSheet {

RecalcEngine::RecalcEngine(QObject *parent) : QObject(parent) {}

// ── Full sheet recalc ─────────────────────────────────────────────────────────

void RecalcEngine::recalcSheet(Sheet *sheet, Workbook *wb)
{
    if (!sheet) return;
    emit recalcStarted();

    // Step 1: Rebuild dependency graph for this sheet
    rebuildGraph(sheet, wb);

    // Step 2: Collect all formula cells as dirty
    int sheetIdx = -1;
    for (int i = 0; i < wb->sheetCount(); ++i)
        if (wb->sheet(i) == sheet) { sheetIdx = i; break; }

    QVector<CellKey> formulaCells;
    sheet->forEachCell([&](int r, int c, Cell &cell) {
        if (cell.hasFormula())
            formulaCells.append(CellKey{sheetIdx, r, c});
    });

    // Step 3: Topo-sort
    QVector<CellKey> order;
    bool ok = m_graph.topoSort(formulaCells, order);

    // Step 4: Evaluate in order
    int evaluated = 0;
    for (const CellKey &key : order) {
        if (key.sheetIdx == sheetIdx) {
            evaluateCell(sheet, wb, key.row, key.col);
            ++evaluated;
        }
    }

    // Step 5: Mark circular references
    if (!ok) {
        auto circs = m_graph.findCircularRefs();
        for (const CellKey &key : circs) {
            if (key.sheetIdx == sheetIdx)
                sheet->cell(key.row, key.col).setError(CellError::Circular);
        }
    }

    emit recalcFinished(evaluated);
}

// ── Incremental recalc ────────────────────────────────────────────────────────

void RecalcEngine::recalcDependents(Sheet *sheet, Workbook *wb, int row, int col)
{
    if (!sheet) return;

    int sheetIdx = -1;
    for (int i = 0; i < wb->sheetCount(); ++i)
        if (wb->sheet(i) == sheet) { sheetIdx = i; break; }

    CellKey changed{sheetIdx, row, col};
    QVector<CellKey> deps = m_graph.dependentsOf(changed);

    QVector<CellKey> order;
    m_graph.topoSort(deps, order);

    for (const CellKey &key : order) {
        Sheet *s = wb->sheet(key.sheetIdx);
        if (s) evaluateCell(s, wb, key.row, key.col);
    }
}

// ── Rebuild graph ─────────────────────────────────────────────────────────────

void RecalcEngine::rebuildGraph(Sheet *sheet, Workbook *wb)
{
    int sheetIdx = -1;
    for (int i = 0; i < wb->sheetCount(); ++i)
        if (wb->sheet(i) == sheet) { sheetIdx = i; break; }

    // Remove old edges for all formula cells in this sheet
    sheet->forEachCell([&](int r, int c, Cell &cell) {
        if (cell.hasFormula())
            m_graph.removeEdgesFor(CellKey{sheetIdx, r, c});
    });

    // Add new edges
    sheet->forEachCell([&](int r, int c, Cell &cell) {
        if (cell.hasFormula())
            parseAndRegisterDependencies(sheet, wb, r, c, cell.raw());
    });
}

// ── Evaluate a single cell ────────────────────────────────────────────────────

void RecalcEngine::evaluateCell(Sheet *sheet, Workbook *wb, int row, int col)
{
    Cell &cell = sheet->cell(row, col);
    if (!cell.hasFormula()) return;

    FormulaParser parser(wb);
    std::unordered_set<std::string> visited;
    visited.insert(std::to_string(row) + "_" + std::to_string(col));

    ParseContext ctx;
    ctx.sheet    = sheet;
    ctx.workbook = wb;
    ctx.baseRow  = row;
    ctx.baseCol  = col;
    ctx.visitedCells = &visited;

    QVariant result = parser.evaluate(cell.raw(), ctx);

    if (parser.lastError() != CellError::None) {
        cell.setError(parser.lastError());
    } else if (result.typeId() == QMetaType::Double || result.typeId() == QMetaType::LongLong) {
        cell.setValue(result, CellType::Number);
    } else if (result.typeId() == QMetaType::Bool) {
        cell.setValue(result, CellType::Boolean);
    } else {
        cell.setValue(result, CellType::Text);
    }
}

// ── Parse formula and register dependencies ───────────────────────────────────

void RecalcEngine::parseAndRegisterDependencies(Sheet *sheet, Workbook *wb,
                                                  int row, int col,
                                                  const QString &formula)
{
    int sheetIdx = -1;
    for (int i = 0; i < wb->sheetCount(); ++i)
        if (wb->sheet(i) == sheet) { sheetIdx = i; break; }

    CellKey dependent{sheetIdx, row, col};

    // Extract all cell references from the formula string
    // Matches: optional "SheetName!" prefix + column letters + row digits
    static QRegularExpression refRe(
        R"((?:([A-Za-z0-9_]+)!)?([A-Za-z]{1,3})(\d{1,7})(?::([A-Za-z]{1,3})(\d{1,7}))?)");

    QString expr = formula.mid(1);  // strip leading '='
    auto it = refRe.globalMatch(expr);
    while (it.hasNext()) {
        auto m = it.next();

        QString sheetName = m.captured(1);
        QString col1Str   = m.captured(2).toUpper();
        int     row1      = m.captured(3).toInt();
        QString col2Str   = m.captured(4).toUpper();
        int     row2      = m.captured(5).toInt();

        // Resolve sheet
        int depSheetIdx = sheetIdx;
        if (!sheetName.isEmpty()) {
            for (int i = 0; i < wb->sheetCount(); ++i) {
                if (wb->sheet(i)->name().compare(sheetName, Qt::CaseInsensitive) == 0) {
                    depSheetIdx = i; break;
                }
            }
        }

        int c1 = FormulaParser::colLetterToIndex(col1Str);

        if (col2Str.isEmpty()) {
            // Single cell reference
            m_graph.addEdge(dependent, CellKey{depSheetIdx, row1, c1});
        } else {
            // Range reference — add every cell in range
            int c2   = FormulaParser::colLetterToIndex(col2Str);
            int rMin = std::min(row1, row2), rMax = std::max(row1, row2);
            int cMin = std::min(c1,  c2),   cMax = std::max(c1,  c2);
            for (int r = rMin; r <= rMax; ++r)
                for (int c = cMin; c <= cMax; ++c)
                    m_graph.addEdge(dependent, CellKey{depSheetIdx, r, c});
        }
    }
}

} // namespace OpenSheet
