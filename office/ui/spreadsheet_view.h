#pragma once
#include <QLineEdit>
#include <QAbstractScrollArea>
#include <QPoint>
#include <QRect>

namespace OpenSheet {

class Workbook;
class Sheet;
class Cell;
struct CellAddress;

/**
 * High-performance cell grid using direct QPainter rendering.
 * Supports: selection, multi-select, drag, copy/paste, keyboard nav,
 *           freeze panes, column/row resize, context menu.
 */
class SpreadsheetView : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit SpreadsheetView(QWidget *parent = nullptr);

    void setWorkbook(Workbook *wb);
    void setSheet(Sheet *sheet);
    Sheet *currentSheet() const { return m_sheet; }

    // Selection
    CellAddress currentCell() const;
    void        selectCell(int row, int col, bool extend = false);
    void        selectRange(int r1, int c1, int r2, int c2);
    void        selectAll();

    // Edit
    void setCurrentCellRaw(const QString &raw);
    void editCurrentCell();
    void deleteSelection();
    void insertChart();
    void insertPivotTable();

    // Clipboard
    void cut();
    void copy();
    void paste();

    // Formatting
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();

    // Dialogs
    void showFindDialog();
    void showFindReplaceDialog();
    void showGoToDialog();

    // Row/col dimensions
    void setRowHeight(int row, double px);
    void setColWidth(int col, double px);

signals:
    void cellSelected(int row, int col);
    void cellEdited(int row, int col, const QString &raw);
    void selectionChanged(int r1, int c1, int r2, int c2);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

private:
    // Rendering
    void drawHeaders(QPainter &p, const QRect &viewport);
    void drawCells(QPainter &p, const QRect &viewport);
    void drawCell(QPainter &p, int row, int col, const QRect &cellRect);
    void drawSelection(QPainter &p);
    void drawFillHandle(QPainter &p);
    void drawFreezeLines(QPainter &p);

    // Coordinate mapping
    QRect   cellRect(int row, int col) const;
    bool    cellAtPoint(const QPoint &pt, int &row, int &col) const;
    QPoint  scrollOffset() const;
    int     rowHeight(int row) const;
    int     colWidth(int col) const;
    int     cellX(int col) const;
    int     cellY(int row) const;
    void    ensureCellVisible(int row, int col);

    // Inline editor
    void startInlineEdit(int row, int col);
    void commitInlineEdit();
    void cancelInlineEdit();

    // Scrollbar sync
    void updateScrollBars();
    void updateViewport();

    // Column/row resize
    bool hitTestColResizer(const QPoint &pt, int &col) const;
    bool hitTestRowResizer(const QPoint &pt, int &row) const;

    Workbook   *m_workbook = nullptr;
    Sheet      *m_sheet    = nullptr;

    // Selection state
    int  m_selRow=1, m_selCol=1;
    int  m_selR1=1,  m_selC1=1, m_selR2=1, m_selC2=1;
    bool m_multiSel = false;
    bool m_dragging = false;

    // Inline editor
    QLineEdit *m_editor = nullptr;
    int  m_editRow=0, m_editCol=0;
    bool m_editing = false;

    // Resize
    bool m_resizingCol = false;
    bool m_resizingRow = false;
    int  m_resizeIdx = 0;
    int  m_resizeStart = 0;

    // Fill handle drag
    bool m_fillDragging = false;
    int  m_fillEndRow=0, m_fillEndCol=0;

    // Clipboard
    QVector<QVector<QString>> m_clipboard;
    bool m_clipCut = false;

    // Layout constants
    static constexpr int kRowHeaderW = 50;
    static constexpr int kColHeaderH = 24;
    static constexpr int kDefaultRowH = 22;
    static constexpr int kDefaultColW = 90;
    static constexpr int kMinRows = 500;
    static constexpr int kMinCols = 50;
};

} // namespace OpenSheet
