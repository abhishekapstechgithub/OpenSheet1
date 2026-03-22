#pragma once
#include "../engine/sheet.h"
#include "../engine/cell.h"
#include "../engine/cell_range.h"
#include <QUndoCommand>
#include <QVector>
#include <QPair>

namespace OpenSheet {

// ─────────────────────────────────────────────────────────────────────────────
//  CellEditCommand — single cell value change
// ─────────────────────────────────────────────────────────────────────────────
class CellEditCommand : public QUndoCommand {
public:
    CellEditCommand(Sheet *sheet, int row, int col,
                    const QString &oldRaw, const QString &newRaw,
                    QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , m_sheet(sheet), m_row(row), m_col(col)
        , m_oldRaw(oldRaw), m_newRaw(newRaw)
    {
        setText(QString("Edit %1%2")
            .arg(FormulaParser::indexToColLetter(col))
            .arg(row));
    }

    void undo() override { m_sheet->setCell(m_row, m_col, m_oldRaw); }
    void redo() override { m_sheet->setCell(m_row, m_col, m_newRaw); }

    // Merge consecutive edits to the same cell
    bool mergeWith(const QUndoCommand *other) override {
        auto *o = dynamic_cast<const CellEditCommand*>(other);
        if (!o || o->m_sheet != m_sheet ||
            o->m_row != m_row || o->m_col != m_col) return false;
        m_newRaw = o->m_newRaw;
        return true;
    }
    int id() const override { return 1001; }

private:
    Sheet  *m_sheet;
    int     m_row, m_col;
    QString m_oldRaw, m_newRaw;
};

// ─────────────────────────────────────────────────────────────────────────────
//  RangeEditCommand — paste / fill across multiple cells
// ─────────────────────────────────────────────────────────────────────────────
class RangeEditCommand : public QUndoCommand {
public:
    struct CellState { int row; int col; QString raw; };

    RangeEditCommand(Sheet *sheet,
                     const QVector<CellState> &before,
                     const QVector<CellState> &after,
                     const QString &label,
                     QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , m_sheet(sheet), m_before(before), m_after(after)
    {
        setText(label);
    }

    void undo() override { apply(m_before); }
    void redo() override { apply(m_after);  }

private:
    Sheet *m_sheet;
    QVector<CellState> m_before, m_after;

    void apply(const QVector<CellState> &states) {
        for (const auto &s : states)
            m_sheet->setCell(s.row, s.col, s.raw);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  InsertRowCommand
// ─────────────────────────────────────────────────────────────────────────────
class InsertRowCommand : public QUndoCommand {
public:
    InsertRowCommand(Sheet *sheet, int row, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), m_sheet(sheet), m_row(row)
    {
        setText(QString("Insert Row %1").arg(row));
    }
    void redo() override { m_sheet->insertRow(m_row); }
    void undo() override { m_sheet->deleteRow(m_row); }
private:
    Sheet *m_sheet;
    int    m_row;
};

// ─────────────────────────────────────────────────────────────────────────────
//  DeleteRowCommand
// ─────────────────────────────────────────────────────────────────────────────
class DeleteRowCommand : public QUndoCommand {
public:
    DeleteRowCommand(Sheet *sheet, int row, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), m_sheet(sheet), m_row(row)
    {
        setText(QString("Delete Row %1").arg(row));
        // Snapshot the row before deleting
        for (int c = 1; c <= 50; ++c) {
            if (sheet->hasCell(row, c))
                m_snapshot.append({c, sheet->cell(row, c)});
        }
    }
    void redo() override { m_sheet->deleteRow(m_row); }
    void undo() override {
        m_sheet->insertRow(m_row);
        for (auto &[col, cell] : m_snapshot)
            m_sheet->cell(m_row, col) = cell;
    }
private:
    Sheet *m_sheet;
    int    m_row;
    QVector<QPair<int, Cell>> m_snapshot;
};

// ─────────────────────────────────────────────────────────────────────────────
//  InsertColCommand / DeleteColCommand
// ─────────────────────────────────────────────────────────────────────────────
class InsertColCommand : public QUndoCommand {
public:
    InsertColCommand(Sheet *sheet, int col, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), m_sheet(sheet), m_col(col)
    {
        setText(QString("Insert Column %1")
            .arg(FormulaParser::indexToColLetter(col)));
    }
    void redo() override { m_sheet->insertCol(m_col); }
    void undo() override { m_sheet->deleteCol(m_col); }
private:
    Sheet *m_sheet; int m_col;
};

// ─────────────────────────────────────────────────────────────────────────────
//  FormatCommand — cell format change
// ─────────────────────────────────────────────────────────────────────────────
class FormatCommand : public QUndoCommand {
public:
    FormatCommand(Sheet *sheet, int row, int col,
                  const CellFormat &oldFmt, const CellFormat &newFmt,
                  QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , m_sheet(sheet), m_row(row), m_col(col)
        , m_oldFmt(oldFmt), m_newFmt(newFmt)
    {
        setText("Format Cell");
    }
    void undo() override { m_sheet->cell(m_row, m_col).setFormat(m_oldFmt); }
    void redo() override { m_sheet->cell(m_row, m_col).setFormat(m_newFmt); }
private:
    Sheet *m_sheet;
    int m_row, m_col;
    CellFormat m_oldFmt, m_newFmt;
};

} // namespace OpenSheet
// Header-only: all QUndoCommand subclasses are inline.
