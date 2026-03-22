#include "spreadsheet_view.h"
#include "../engine/workbook.h"
#include "../engine/sheet.h"
#include "../engine/cell.h"
#include "../engine/cell_range.h"

#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QInputDialog>
#include <QMessageBox>
#include <QFontMetrics>

namespace OpenSheet {

// ── Static colors ─────────────────────────────────────────────────────────────
static const QColor kColorBackground   = QColor(0xFF, 0xFF, 0xFF);
static const QColor kColorHeaderBg     = QColor(0xF0, 0xEF, 0xE9);
static const QColor kColorGridLine     = QColor(0xE0, 0xDF, 0xD9);
static const QColor kColorHeaderText   = QColor(0x88, 0x87, 0x80);
static const QColor kColorSelBg        = QColor(0xB5, 0xD4, 0xF4, 100);
static const QColor kColorSelBorder    = QColor(0x18, 0x5F, 0xA5);
static const QColor kColorFormulaText  = QColor(0x18, 0x5F, 0xA5);
static const QColor kColorTotalRow     = QColor(0xF0, 0xF4, 0xFB);

// ─────────────────────────────────────────────────────────────────────────────

SpreadsheetView::SpreadsheetView(QWidget *parent)
    : QAbstractScrollArea(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setAttribute(Qt::WA_InputMethodEnabled);

    // Inline editor
    m_editor = new QLineEdit(viewport());
    m_editor->hide();
    m_editor->setFrame(false);
    m_editor->setStyleSheet(
        "QLineEdit { background: #EAF3FE; border: 2px solid #185FA5; "
        "padding: 1px 4px; font-size: 12px; }");
    connect(m_editor, &QLineEdit::returnPressed, this, [this]{ commitInlineEdit(); });
    connect(m_editor, &QLineEdit::textEdited,    this, [this](const QString &t){
        emit cellEdited(m_editRow, m_editCol, t.startsWith('=') ? t : t);
    });

    installEventFilter(this);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void SpreadsheetView::setWorkbook(Workbook *wb)
{
    m_workbook = wb;
    setSheet(wb ? wb->activeSheet() : nullptr);
}

void SpreadsheetView::setSheet(Sheet *sheet)
{
    m_sheet = sheet;
    m_selRow = m_selCol = m_selR1 = m_selC1 = m_selR2 = m_selC2 = 1;
    cancelInlineEdit();
    updateScrollBars();
    viewport()->update();
}

CellAddress SpreadsheetView::currentCell() const { return {m_selRow, m_selCol}; }

void SpreadsheetView::selectCell(int row, int col, bool extend)
{
    if (!m_sheet) return;
    row = qBound(1, row, kMinRows);
    col = qBound(1, col, kMinCols);

    if (extend) {
        m_selR2 = row; m_selC2 = col;
        m_multiSel = true;
    } else {
        m_selRow = m_selR1 = m_selR2 = row;
        m_selCol = m_selC1 = m_selC2 = col;
        m_multiSel = false;
    }

    ensureCellVisible(row, col);
    viewport()->update();
    emit cellSelected(row, col);
    emit selectionChanged(m_selR1, m_selC1, m_selR2, m_selC2);
}

void SpreadsheetView::selectRange(int r1, int c1, int r2, int c2)
{
    m_selR1 = r1; m_selC1 = c1;
    m_selR2 = r2; m_selC2 = c2;
    m_selRow = r1; m_selCol = c1;
    m_multiSel = true;
    viewport()->update();
    emit selectionChanged(r1, c1, r2, c2);
}

void SpreadsheetView::selectAll()
{
    if (!m_sheet) return;
    auto sz = m_sheet->usedRange();
    selectRange(1, 1, sz.width(), sz.height());
}

void SpreadsheetView::setCurrentCellRaw(const QString &raw)
{
    if (!m_sheet) return;
    m_sheet->setCell(m_selRow, m_selCol, raw);
    viewport()->update();
    if (m_workbook) m_workbook->recalcSheet(m_sheet);
}

void SpreadsheetView::editCurrentCell()
{
    startInlineEdit(m_selRow, m_selCol);
}

void SpreadsheetView::deleteSelection()
{
    if (!m_sheet) return;
    int r1=m_selR1,c1=m_selC1,r2=m_selR2,c2=m_selC2;
    for (int r=r1;r<=r2;++r)
        for (int c=c1;c<=c2;++c)
            m_sheet->clearCell(r, c);
    if (m_workbook) m_workbook->recalcSheet(m_sheet);
    viewport()->update();
}

// ── Clipboard ─────────────────────────────────────────────────────────────────

void SpreadsheetView::copy()
{
    if (!m_sheet) return;
    m_clipboard.clear();
    m_clipCut = false;
    QString tsv;
    for (int r = m_selR1; r <= m_selR2; ++r) {
        QVector<QString> row;
        QStringList rowStrs;
        for (int c = m_selC1; c <= m_selC2; ++c) {
            QString v = m_sheet->cell(r, c).displayText();
            row.append(v);
            rowStrs.append(v);
        }
        m_clipboard.append(row);
        tsv += rowStrs.join('\t') + '\n';
    }
    QApplication::clipboard()->setText(tsv);
    viewport()->update(); // show marching ants
}

void SpreadsheetView::cut()
{
    copy();
    m_clipCut = true;
}

void SpreadsheetView::paste()
{
    if (!m_sheet) return;
    QString text = QApplication::clipboard()->text();
    if (text.isEmpty() && m_clipboard.isEmpty()) return;

    if (!text.isEmpty()) {
        // Parse TSV from clipboard
        QStringList lines = text.split('\n');
        for (int ri = 0; ri < lines.size(); ++ri) {
            if (lines[ri].isEmpty()) continue;
            QStringList cells = lines[ri].split('\t');
            for (int ci = 0; ci < cells.size(); ++ci)
                m_sheet->setCell(m_selRow + ri, m_selCol + ci, cells[ci]);
        }
    } else {
        for (int ri = 0; ri < m_clipboard.size(); ++ri)
            for (int ci = 0; ci < m_clipboard[ri].size(); ++ci)
                m_sheet->setCell(m_selRow + ri, m_selCol + ci, m_clipboard[ri][ci]);
    }

    if (m_clipCut) {
        deleteSelection();
        m_clipCut = false;
        m_clipboard.clear();
    }

    if (m_workbook) m_workbook->recalcSheet(m_sheet);
    viewport()->update();
}

// ── Formatting ────────────────────────────────────────────────────────────────

void SpreadsheetView::toggleBold()
{
    if (!m_sheet) return;
    auto &fmt = m_sheet->cell(m_selRow, m_selCol).format();
    fmt.bold = !fmt.bold;
    viewport()->update();
}

void SpreadsheetView::toggleItalic()
{
    if (!m_sheet) return;
    auto &fmt = m_sheet->cell(m_selRow, m_selCol).format();
    fmt.italic = !fmt.italic;
    viewport()->update();
}

void SpreadsheetView::toggleUnderline()
{
    if (!m_sheet) return;
    auto &fmt = m_sheet->cell(m_selRow, m_selCol).format();
    fmt.underline = !fmt.underline;
    viewport()->update();
}

// ── Dialogs ───────────────────────────────────────────────────────────────────

void SpreadsheetView::showFindDialog()
{
    bool ok;
    QString query = QInputDialog::getText(this, "Find", "Find:", QLineEdit::Normal, "", &ok);
    if (!ok || query.isEmpty() || !m_sheet) return;
    m_sheet->forEachCell([&](int r, int c, Cell &cell){
        if (cell.displayText().contains(query, Qt::CaseInsensitive)) {
            selectCell(r, c);
        }
    });
}

void SpreadsheetView::showFindReplaceDialog()
{
    bool ok;
    QString find = QInputDialog::getText(this, "Find", "Find:", QLineEdit::Normal, "", &ok);
    if (!ok || find.isEmpty() || !m_sheet) return;
    QString replace = QInputDialog::getText(this, "Replace", "Replace with:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    int count = 0;
    m_sheet->forEachCell([&](int r, int c, Cell &cell){
        if (cell.raw().contains(find, Qt::CaseInsensitive)) {
            QString newRaw = cell.raw().replace(find, replace, Qt::CaseInsensitive);
            m_sheet->setCell(r, c, newRaw);
            ++count;
        }
    });
    if (m_workbook) m_workbook->recalcSheet(m_sheet);
    viewport()->update();
    QMessageBox::information(this, "Replace", QString("%1 replacement(s) made.").arg(count));
}

void SpreadsheetView::showGoToDialog()
{
    bool ok;
    QString ref = QInputDialog::getText(this, "Go To", "Cell reference (e.g. B10):",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || ref.isEmpty()) return;
    int row=0, col=0;
    if (FormulaParser::parseCellRef(ref.trimmed().toUpper(), row, col))
        selectCell(row, col);
}

void SpreadsheetView::insertChart() { /* TODO: show chart dialog */ }
void SpreadsheetView::insertPivotTable() { /* TODO: show pivot dialog */ }

void SpreadsheetView::setRowHeight(int row, double px)
{
    if (m_sheet) m_sheet->rowProps(row).height = px;
    updateScrollBars();
    viewport()->update();
}

void SpreadsheetView::setColWidth(int col, double px)
{
    if (m_sheet) m_sheet->colProps(col).width = px;
    updateScrollBars();
    viewport()->update();
}

// ─────────────────────────────────────────────────────────────────────────────
// Painting
// ─────────────────────────────────────────────────────────────────────────────

void SpreadsheetView::paintEvent(QPaintEvent *)
{
    QPainter p(viewport());
    p.setRenderHint(QPainter::TextAntialiasing);

    const QRect vp = viewport()->rect();
    p.fillRect(vp, kColorBackground);

    if (!m_sheet) return;

    drawHeaders(p, vp);
    drawCells(p, vp);
    drawSelection(p);
    drawFreezeLines(p);
    drawFillHandle(p);
}

void SpreadsheetView::drawHeaders(QPainter &p, const QRect &vp)
{
    const QPoint off = scrollOffset();

    // Corner cell
    p.fillRect(QRect(0, 0, kRowHeaderW, kColHeaderH), kColorHeaderBg);
    p.setPen(kColorGridLine);
    p.drawLine(kRowHeaderW - 1, 0, kRowHeaderW - 1, kColHeaderH);
    p.drawLine(0, kColHeaderH - 1, kRowHeaderW, kColHeaderH - 1);

    QFont hFont = p.font();
    hFont.setPointSize(9);
    p.setFont(hFont);

    // Column headers
    int x = kRowHeaderW - off.x();
    for (int c = 1; c <= kMinCols && x < vp.width(); ++c) {
        int w = colWidth(c);
        QRect hr(x, 0, w, kColHeaderH);
        p.fillRect(hr, kColorHeaderBg);
        p.setPen(kColorGridLine);
        p.drawRect(hr.adjusted(0,0,-1,-1));
        p.setPen(kColorHeaderText);
        p.drawText(hr, Qt::AlignCenter, FormulaParser::indexToColLetter(c));
        x += w;
    }

    // Row headers
    int y = kColHeaderH - off.y();
    for (int r = 1; r <= kMinRows && y < vp.height(); ++r) {
        int h = rowHeight(r);
        QRect rh(0, y, kRowHeaderW, h);
        p.fillRect(rh, kColorHeaderBg);
        p.setPen(kColorGridLine);
        p.drawRect(rh.adjusted(0,0,-1,-1));
        p.setPen(kColorHeaderText);
        p.drawText(rh, Qt::AlignCenter, QString::number(r));
        y += h;
    }
}

void SpreadsheetView::drawCells(QPainter &p, const QRect &vp)
{
    const QPoint off = scrollOffset();

    int y = kColHeaderH - off.y();
    for (int r = 1; r <= kMinRows && y < vp.height(); ++r) {
        int rh = rowHeight(r);
        if (y + rh < 0) { y += rh; continue; }

        if (m_sheet && m_sheet->isRowHidden(r)) { y += rh; continue; }

        int x = kRowHeaderW - off.x();
        for (int c = 1; c <= kMinCols && x < vp.width(); ++c) {
            int cw = colWidth(c);
            if (x + cw >= kRowHeaderW)
                drawCell(p, r, c, QRect(x, y, cw, rh));
            x += cw;
        }
        y += rh;
    }
}

void SpreadsheetView::drawCell(QPainter &p, int row, int col, const QRect &cr)
{
    if (!m_sheet) return;
    const Cell &cell = m_sheet->cell(row, col);

    // Background
    QColor bg = kColorBackground;
    if (cell.format().background != Qt::transparent)
        bg = cell.format().background;

    // Conditional formatting override
    auto cFmt = m_sheet->evalConditionalFormat(row, col);
    if (cFmt && cFmt->background != Qt::transparent)
        bg = cFmt->background;

    p.fillRect(cr, bg);

    // Grid line
    p.setPen(QPen(kColorGridLine, 0.5));
    p.drawLine(cr.right(), cr.top(), cr.right(), cr.bottom());
    p.drawLine(cr.left(), cr.bottom(), cr.right(), cr.bottom());

    // Text
    QString txt = cell.displayText();
    if (txt.isEmpty()) return;

    QFont f = p.font();
    f.setPointSize(10);
    f.setBold(cell.format().bold || (cFmt && cFmt->bold));
    f.setItalic(cell.format().italic);
    f.setUnderline(cell.format().underline);
    p.setFont(f);

    QColor fg = cell.format().foreground;
    if (fg == Qt::black || fg == Qt::transparent) fg = QColor(0x1a, 0x1a, 0x18);
    if (cFmt && cFmt->foreground != Qt::black) fg = cFmt->foreground;
    if (cell.hasFormula()) fg = kColorFormulaText;
    if (cell.type() == CellType::Error) fg = QColor(0xE2, 0x4B, 0x4A);
    p.setPen(fg);

    Qt::Alignment align = Qt::AlignVCenter;
    if (cell.type() == CellType::Number || cell.type() == CellType::Formula)
        align |= Qt::AlignRight;
    else
        align |= Qt::AlignLeft;
    if (cell.format().alignment & Qt::AlignHCenter) align = Qt::AlignVCenter | Qt::AlignHCenter;
    if (cell.format().alignment & Qt::AlignRight)   align = Qt::AlignVCenter | Qt::AlignRight;

    QRect textRect = cr.adjusted(4, 1, -4, -1);
    p.drawText(textRect, align, txt);
}

void SpreadsheetView::drawSelection(QPainter &p)
{
    if (!m_sheet) return;
    const QPoint off = scrollOffset();

    // Multi-cell range highlight
    if (m_multiSel) {
        int r1=std::min(m_selR1,m_selR2), r2=std::max(m_selR1,m_selR2);
        int c1=std::min(m_selC1,m_selC2), c2=std::max(m_selC1,m_selC2);
        QRect topLeft = cellRect(r1, c1), botRight = cellRect(r2, c2);
        QRect range(topLeft.topLeft(), botRight.bottomRight());
        range.translate(-off.x() + kRowHeaderW - cellX(1),
                        -off.y() + kColHeaderH - cellY(1));
        p.fillRect(range, kColorSelBg);
        p.setPen(QPen(kColorSelBorder, 1.5));
        p.drawRect(range.adjusted(0,0,-1,-1));
    }

    // Active cell outline
    QRect cr = cellRect(m_selRow, m_selCol);
    int cx = kRowHeaderW + cr.x() - off.x();
    int cy = kColHeaderH + cr.y() - off.y();
    QRect draw(cx, cy, cr.width(), cr.height());
    p.setPen(QPen(kColorSelBorder, 2));
    p.setBrush(Qt::NoBrush);
    p.drawRect(draw.adjusted(0,0,-1,-1));
}

void SpreadsheetView::drawFillHandle(QPainter &p)
{
    if (m_multiSel) return;
    QRect cr = cellRect(m_selRow, m_selCol);
    const QPoint off = scrollOffset();
    int x = kRowHeaderW + cr.right() - off.x() - 4;
    int y = kColHeaderH + cr.bottom() - off.y() - 4;
    p.fillRect(QRect(x, y, 7, 7), kColorSelBorder);
}

void SpreadsheetView::drawFreezeLines(QPainter &p)
{
    if (!m_sheet) return;
    int fr = m_sheet->freezeRow();
    int fc = m_sheet->freezeCol();
    if (fr <= 0 && fc <= 0) return;

    p.setPen(QPen(kColorSelBorder, 1.5, Qt::DashLine));
    const QPoint off = scrollOffset();

    if (fr > 0) {
        int y = kColHeaderH;
        for (int r = 1; r <= fr; ++r) y += rowHeight(r);
        p.drawLine(kRowHeaderW, y, viewport()->width(), y);
    }
    if (fc > 0) {
        int x = kRowHeaderW;
        for (int c = 1; c <= fc; ++c) x += colWidth(c);
        p.drawLine(x, kColHeaderH, x, viewport()->height());
    }
    Q_UNUSED(off);
}

// ─────────────────────────────────────────────────────────────────────────────
// Coordinate helpers
// ─────────────────────────────────────────────────────────────────────────────

int SpreadsheetView::rowHeight(int r) const
{
    if (m_sheet) {
        auto it = m_sheet->rowProps(r);
        return qMax(4, (int)it.height);
    }
    return kDefaultRowH;
}

int SpreadsheetView::colWidth(int c) const
{
    if (m_sheet) {
        auto it = m_sheet->colProps(c);
        return qMax(4, (int)it.width);
    }
    return kDefaultColW;
}

int SpreadsheetView::cellX(int col) const
{
    int x = 0;
    for (int c = 1; c < col; ++c) x += colWidth(c);
    return x;
}

int SpreadsheetView::cellY(int row) const
{
    int y = 0;
    for (int r = 1; r < row; ++r) y += rowHeight(r);
    return y;
}

QRect SpreadsheetView::cellRect(int row, int col) const
{
    return QRect(cellX(col), cellY(row), colWidth(col), rowHeight(row));
}

bool SpreadsheetView::cellAtPoint(const QPoint &pt, int &row, int &col) const
{
    const QPoint off = scrollOffset();
    int vx = pt.x() - kRowHeaderW + off.x();
    int vy = pt.y() - kColHeaderH + off.y();
    if (vx < 0 || vy < 0) return false;

    int x = 0;
    for (int c = 1; c <= kMinCols; ++c) {
        x += colWidth(c);
        if (vx < x) { col = c; break; }
    }
    int y = 0;
    for (int r = 1; r <= kMinRows; ++r) {
        y += rowHeight(r);
        if (vy < y) { row = r; break; }
    }
    return row > 0 && col > 0;
}

QPoint SpreadsheetView::scrollOffset() const
{
    return {horizontalScrollBar()->value(), verticalScrollBar()->value()};
}

void SpreadsheetView::ensureCellVisible(int row, int col)
{
    QRect cr = cellRect(row, col);
    const QRect vp = viewport()->rect().adjusted(kRowHeaderW, kColHeaderH, 0, 0);
    const QPoint off = scrollOffset();

    int newH = off.x(), newV = off.y();
    if (cr.left() - off.x() < 0)                 newH = cr.left();
    else if (cr.right() - off.x() > vp.width())  newH = cr.right() - vp.width();
    if (cr.top() - off.y() < 0)                  newV = cr.top();
    else if (cr.bottom() - off.y() > vp.height()) newV = cr.bottom() - vp.height();

    horizontalScrollBar()->setValue(newH);
    verticalScrollBar()->setValue(newV);
}

void SpreadsheetView::updateScrollBars()
{
    int totalW = 0, totalH = 0;
    for (int c = 1; c <= kMinCols; ++c) totalW += colWidth(c);
    for (int r = 1; r <= kMinRows; ++r) totalH += rowHeight(r);

    horizontalScrollBar()->setRange(0, qMax(0, totalW - viewport()->width()  + kRowHeaderW));
    verticalScrollBar()->setRange  (0, qMax(0, totalH - viewport()->height() + kColHeaderH));
    horizontalScrollBar()->setSingleStep(kDefaultColW);
    verticalScrollBar()->setSingleStep  (kDefaultRowH);
}

// ─────────────────────────────────────────────────────────────────────────────
// Mouse / keyboard
// ─────────────────────────────────────────────────────────────────────────────

void SpreadsheetView::mousePressEvent(QMouseEvent *e)
{
    if (m_editing) commitInlineEdit();
    int r=0, c=0;
    if (cellAtPoint(e->pos(), r, c)) {
        selectCell(r, c, e->modifiers() & Qt::ShiftModifier);
        m_dragging = true;
    }
}

void SpreadsheetView::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_dragging) return;
    int r=0, c=0;
    if (cellAtPoint(e->pos(), r, c))
        selectCell(r, c, true);
}

void SpreadsheetView::mouseReleaseEvent(QMouseEvent *)
{
    m_dragging = false;
}

void SpreadsheetView::mouseDoubleClickEvent(QMouseEvent *e)
{
    int r=0, c=0;
    if (cellAtPoint(e->pos(), r, c))
        startInlineEdit(r, c);
}

void SpreadsheetView::wheelEvent(QWheelEvent *e)
{
    QAbstractScrollArea::wheelEvent(e);
}

void SpreadsheetView::keyPressEvent(QKeyEvent *e)
{
    if (m_editing) {
        if (e->key() == Qt::Key_Escape)  { cancelInlineEdit(); return; }
        if (e->key() == Qt::Key_Return)  { commitInlineEdit(); return; }
        if (e->key() == Qt::Key_Tab)     { commitInlineEdit();
            selectCell(m_selRow, m_selCol + 1); return; }
        return; // let editor handle other keys
    }

    const bool shift = e->modifiers() & Qt::ShiftModifier;
    const bool ctrl  = e->modifiers() & Qt::ControlModifier;

    switch (e->key()) {
    case Qt::Key_Up:    selectCell(m_selRow - 1, m_selCol, shift); break;
    case Qt::Key_Down:  selectCell(m_selRow + 1, m_selCol, shift); break;
    case Qt::Key_Left:  selectCell(m_selRow, m_selCol - 1, shift); break;
    case Qt::Key_Right: selectCell(m_selRow, m_selCol + 1, shift); break;
    case Qt::Key_Tab:   selectCell(m_selRow, m_selCol + (shift ? -1 : 1)); break;
    case Qt::Key_Return:
    case Qt::Key_Enter: selectCell(m_selRow + 1, m_selCol); break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace: deleteSelection(); break;
    case Qt::Key_F2:    startInlineEdit(m_selRow, m_selCol); break;
    case Qt::Key_Home:  selectCell(m_selRow, 1); break;
    case Qt::Key_End: {
        int maxC = m_sheet ? m_sheet->maxCol() : 1;
        selectCell(m_selRow, maxC); break;
    }
    case Qt::Key_PageUp:
        selectCell(qMax(1, m_selRow - 20), m_selCol); break;
    case Qt::Key_PageDown:
        selectCell(m_selRow + 20, m_selCol); break;
    default:
        // Printable key starts editing
        if (!ctrl && !e->text().isEmpty() && e->text()[0].isPrint()) {
            startInlineEdit(m_selRow, m_selCol);
            m_editor->setText(e->text());
            m_editor->end(false);
        }
        break;
    }
}

void SpreadsheetView::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    menu.addAction("Cut",   this, &SpreadsheetView::cut,   QKeySequence::Cut);
    menu.addAction("Copy",  this, &SpreadsheetView::copy,  QKeySequence::Copy);
    menu.addAction("Paste", this, &SpreadsheetView::paste, QKeySequence::Paste);
    menu.addSeparator();
    menu.addAction("Insert Row",    this, [this]{ if(m_sheet) m_sheet->insertRow(m_selRow); viewport()->update(); });
    menu.addAction("Delete Row",    this, [this]{ if(m_sheet) m_sheet->deleteRow(m_selRow); viewport()->update(); });
    menu.addAction("Insert Column", this, [this]{ if(m_sheet) m_sheet->insertCol(m_selCol); viewport()->update(); });
    menu.addAction("Delete Column", this, [this]{ if(m_sheet) m_sheet->deleteCol(m_selCol); viewport()->update(); });
    menu.addSeparator();
    menu.addAction("Clear Cell", this, &SpreadsheetView::deleteSelection);
    menu.exec(e->globalPos());
}

void SpreadsheetView::resizeEvent(QResizeEvent *)
{
    updateScrollBars();
}

// ─────────────────────────────────────────────────────────────────────────────
// Inline editor
// ─────────────────────────────────────────────────────────────────────────────

void SpreadsheetView::startInlineEdit(int row, int col)
{
    if (!m_sheet) return;
    if (m_editing) commitInlineEdit();

    m_editRow = row; m_editCol = col;
    m_editing = true;

    QRect cr = cellRect(row, col);
    const QPoint off = scrollOffset();
    QRect edRect(kRowHeaderW + cr.x() - off.x(),
                 kColHeaderH + cr.y() - off.y(),
                 cr.width(), cr.height());

    m_editor->setGeometry(edRect);
    m_editor->setText(m_sheet->cell(row, col).raw());
    m_editor->show();
    m_editor->setFocus();
    m_editor->selectAll();
}

void SpreadsheetView::commitInlineEdit()
{
    if (!m_editing || !m_sheet) return;
    m_editing = false;
    QString raw = m_editor->text();
    m_editor->hide();
    m_sheet->setCell(m_editRow, m_editCol, raw);
    if (m_workbook) m_workbook->recalcSheet(m_sheet);
    emit cellEdited(m_editRow, m_editCol, raw);
    viewport()->update();
}

void SpreadsheetView::cancelInlineEdit()
{
    m_editing = false;
    if (m_editor) m_editor->hide();
}

// ─────────────────────────────────────────────────────────────────────────────
// Input method (CJK / IME support)
// ─────────────────────────────────────────────────────────────────────────────

void SpreadsheetView::inputMethodEvent(QInputMethodEvent *e)
{
    if (!m_editing) startInlineEdit(m_selRow, m_selCol);
    QApplication::sendEvent(m_editor, e);
}

QVariant SpreadsheetView::inputMethodQuery(Qt::InputMethodQuery q) const
{
    return m_editor->inputMethodQuery(q);
}

} // namespace OpenSheet
