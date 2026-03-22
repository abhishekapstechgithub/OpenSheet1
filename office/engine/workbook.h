#pragma once
#include "sheet.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <QUndoStack>

namespace OpenSheet {

class RecalcEngine;

class Workbook : public QObject {
    Q_OBJECT
public:
    explicit Workbook(QObject *parent = nullptr);
    ~Workbook();

    // --- Sheets ---
    Sheet      *addSheet(const QString &name = {});
    Sheet      *insertSheet(int index, const QString &name = {});
    void        removeSheet(int index);
    void        moveSheet(int from, int to);
    Sheet      *sheet(int index) const;
    Sheet      *sheet(const QString &name) const;
    int         sheetCount() const { return m_sheets.size(); }
    int         activeSheetIndex() const { return m_activeIdx; }
    Sheet      *activeSheet() const;
    void        setActiveSheet(int index);
    QStringList sheetNames() const;

    // --- File info ---
    QString  filePath()      const { return m_filePath; }
    void     setFilePath(const QString &p) { m_filePath = p; }
    bool     isModified()    const { return m_modified; }
    void     setModified(bool m);
    QString  title()         const;

    // --- Named ranges ---
    void    setNamedRange(const QString &name, Sheet *sheet, const CellRange &range);
    bool    resolveNamedRange(const QString &name, Sheet *&outSheet, CellRange &outRange) const;

    // --- Undo / Redo ---
    QUndoStack *undoStack() { return m_undoStack; }

    // --- Recalculation ---
    void recalcAll();
    void recalcSheet(Sheet *sheet);

    // --- Autosave ---
    void     setAutoSavePath(const QString &p) { m_autoSavePath = p; }
    QString  autoSavePath() const { return m_autoSavePath; }

signals:
    void sheetAdded(int index);
    void sheetRemoved(int index);
    void sheetRenamed(int index, const QString &name);
    void activeSheetChanged(int index);
    void modifiedChanged(bool modified);
    void recalcComplete();

private slots:
    void onCellChanged(int row, int col);

private:
    QVector<Sheet*>  m_sheets;
    int              m_activeIdx  = 0;
    QString          m_filePath;
    bool             m_modified   = false;
    QString          m_autoSavePath;
    QUndoStack      *m_undoStack;
    RecalcEngine    *m_recalc;

    struct NamedRange { Sheet *sheet; CellRange range; };
    QHash<QString, NamedRange> m_namedRanges;

    QString uniqueSheetName(const QString &base) const;
};

} // namespace OpenSheet
