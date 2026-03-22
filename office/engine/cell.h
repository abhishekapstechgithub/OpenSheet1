#pragma once
#include <QString>
#include <QVariant>
#include <QColor>
#include <QFont>
#include <QDateTime>
#include <optional>

namespace OpenSheet {

enum class CellType {
    Empty,
    Text,
    Number,
    Boolean,
    Date,
    Formula,
    Error
};

enum class CellError {
    None,
    DivZero,   // #DIV/0!
    Name,      // #NAME?
    Value,     // #VALUE!
    Ref,       // #REF!
    NA,        // #N/A
    Null,      // #NULL!
    Circular,  // #CIRC!
    Num        // #NUM!
};

struct CellFormat {
    QFont    font;
    QColor   foreground    = Qt::black;
    QColor   background    = Qt::transparent;
    Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter;
    bool     bold          = false;
    bool     italic        = false;
    bool     underline     = false;
    bool     strikethrough = false;
    bool     wrapText      = false;
    int      indent        = 0;
    QString  numberFormat  = "General"; // e.g. "0.00", "#,##0", "dd/mm/yyyy"
    int      borderTop     = 0;
    int      borderBottom  = 0;
    int      borderLeft    = 0;
    int      borderRight   = 0;
    QColor   borderColor   = Qt::black;

    bool operator==(const CellFormat &o) const = default;
};

class Cell {
public:
    Cell() = default;
    explicit Cell(const QString &raw);

    // --- Raw / computed values ---
    void        setRaw(const QString &raw);
    QString     raw()        const { return m_raw; }
    QVariant    value()      const { return m_value; }
    CellType    type()       const { return m_type; }
    CellError   error()      const { return m_error; }
    bool        hasFormula() const { return m_raw.startsWith('='); }

    void        setValue(const QVariant &v, CellType t = CellType::Number);
    void        setError(CellError err);

    // Displayed string (formatted)
    QString     displayText() const;

    // --- Formatting ---
    CellFormat &format()           { return m_format; }
    const CellFormat &format() const { return m_format; }
    void        setFormat(const CellFormat &f) { m_format = f; }

    // --- Metadata ---
    QString     comment()  const { return m_comment; }
    void        setComment(const QString &c) { m_comment = c; }
    QString     hyperlink() const { return m_hyperlink; }
    void        setHyperlink(const QString &h) { m_hyperlink = h; }

    // --- Merge ---
    bool        isMerged()   const { return m_merged; }
    void        setMerged(bool m)  { m_merged = m; }
    int         mergeSpanCols() const { return m_mergeSpanCols; }
    int         mergeSpanRows() const { return m_mergeSpanRows; }
    void        setMergeSpan(int rows, int cols) { m_mergeSpanRows = rows; m_mergeSpanCols = cols; }

    bool        isEmpty() const;

    static QString errorString(CellError e);

private:
    QString     m_raw;
    QVariant    m_value;
    CellType    m_type    = CellType::Empty;
    CellError   m_error   = CellError::None;
    CellFormat  m_format;
    QString     m_comment;
    QString     m_hyperlink;
    bool        m_merged        = false;
    int         m_mergeSpanRows = 1;
    int         m_mergeSpanCols = 1;

    void detectType();
};

} // namespace OpenSheet
