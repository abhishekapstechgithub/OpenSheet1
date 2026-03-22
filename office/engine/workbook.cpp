#include "workbook.h"
#include "recalc_engine.h"
#include <QUndoStack>
#include <QFileInfo>

namespace OpenSheet {

Workbook::Workbook(QObject *parent)
    : QObject(parent)
    , m_undoStack(new QUndoStack(this))
    , m_recalc(new RecalcEngine(this))
{
    m_undoStack->setUndoLimit(100);
}

Workbook::~Workbook()
{
    qDeleteAll(m_sheets);
}

// ── Sheet management ──────────────────────────────────────────────────────────

Sheet *Workbook::addSheet(const QString &name)
{
    return insertSheet(m_sheets.size(), name);
}

Sheet *Workbook::insertSheet(int index, const QString &name)
{
    QString sheetName = name.isEmpty()
        ? uniqueSheetName("Sheet")
        : uniqueSheetName(name);

    auto *s = new Sheet(sheetName, this);
    connect(s, &Sheet::cellChanged, this, &Workbook::onCellChanged);

    index = qBound(0, index, m_sheets.size());
    m_sheets.insert(index, s);

    if (m_activeIdx >= index && m_sheets.size() > 1)
        m_activeIdx++;

    setModified(true);
    emit sheetAdded(index);
    return s;
}

void Workbook::removeSheet(int index)
{
    if (index < 0 || index >= m_sheets.size()) return;
    if (m_sheets.size() == 1) return;     // must keep at least one sheet

    Sheet *s = m_sheets.takeAt(index);
    s->deleteLater();

    if (m_activeIdx >= m_sheets.size())
        m_activeIdx = m_sheets.size() - 1;

    setModified(true);
    emit sheetRemoved(index);
}

void Workbook::moveSheet(int from, int to)
{
    if (from < 0 || from >= m_sheets.size()) return;
    if (to   < 0 || to   >= m_sheets.size()) return;
    if (from == to) return;

    m_sheets.move(from, to);

    if (m_activeIdx == from) m_activeIdx = to;
    else if (from < m_activeIdx && to >= m_activeIdx) m_activeIdx--;
    else if (from > m_activeIdx && to <= m_activeIdx) m_activeIdx++;

    setModified(true);
    emit structureChanged();
}

Sheet *Workbook::sheet(int index) const
{
    if (index < 0 || index >= m_sheets.size()) return nullptr;
    return m_sheets[index];
}

Sheet *Workbook::sheet(const QString &name) const
{
    for (Sheet *s : m_sheets)
        if (s->name().compare(name, Qt::CaseInsensitive) == 0)
            return s;
    return nullptr;
}

Sheet *Workbook::activeSheet() const
{
    return sheet(m_activeIdx);
}

void Workbook::setActiveSheet(int index)
{
    if (index < 0 || index >= m_sheets.size()) return;
    if (index == m_activeIdx) return;
    m_activeIdx = index;
    emit activeSheetChanged(index);
}

QStringList Workbook::sheetNames() const
{
    QStringList names;
    names.reserve(m_sheets.size());
    for (const Sheet *s : m_sheets)
        names << s->name();
    return names;
}

// ── File info ─────────────────────────────────────────────────────────────────

void Workbook::setModified(bool m)
{
    if (m == m_modified) return;
    m_modified = m;
    emit modifiedChanged(m);
}

QString Workbook::title() const
{
    if (!m_filePath.isEmpty())
        return QFileInfo(m_filePath).baseName();
    return "Untitled";
}

// ── Named ranges ──────────────────────────────────────────────────────────────

void Workbook::setNamedRange(const QString &name, Sheet *s, const CellRange &range)
{
    m_namedRanges[name.toUpper()] = {s, range};
    setModified(true);
}

bool Workbook::resolveNamedRange(const QString &name,
                                  Sheet *&outSheet,
                                  CellRange &outRange) const
{
    auto it = m_namedRanges.constFind(name.toUpper());
    if (it == m_namedRanges.constEnd()) return false;
    outSheet = it->sheet;
    outRange = it->range;
    return true;
}

// ── Recalculation ─────────────────────────────────────────────────────────────

void Workbook::recalcAll()
{
    for (Sheet *s : m_sheets)
        m_recalc->recalcSheet(s, this);
    emit recalcComplete();
}

void Workbook::recalcSheet(Sheet *s)
{
    if (!s) return;
    m_recalc->recalcSheet(s, this);
    emit recalcComplete();
}

// ── Private helpers ───────────────────────────────────────────────────────────

QString Workbook::uniqueSheetName(const QString &base) const
{
    QStringList existing = sheetNames();
    if (!existing.contains(base, Qt::CaseInsensitive))
        return base;

    int n = 2;
    while (true) {
        QString candidate = QString("%1%2").arg(base).arg(n++);
        if (!existing.contains(candidate, Qt::CaseInsensitive))
            return candidate;
    }
}

void Workbook::onCellChanged(int /*row*/, int /*col*/)
{
    setModified(true);
    // Kick a deferred recalc (debounced so rapid edits don't flood)
    QMetaObject::invokeMethod(this, [this]{
        Sheet *s = activeSheet();
        if (s) m_recalc->recalcSheet(s, this);
    }, Qt::QueuedConnection);
}

} // namespace OpenSheet
