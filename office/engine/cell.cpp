#include "cell.h"
#include <QLocale>
#include <QRegularExpression>

namespace OpenSheet {

Cell::Cell(const QString &raw) { setRaw(raw); }

void Cell::setRaw(const QString &raw)
{
    m_raw   = raw;
    m_error = CellError::None;
    if (raw.isEmpty()) {
        m_type  = CellType::Empty;
        m_value = QVariant();
        return;
    }
    if (raw.startsWith('=')) {
        m_type  = CellType::Formula;
        m_value = QVariant(); // will be set by recalc engine
        return;
    }
    detectType();
}

void Cell::detectType()
{
    // Boolean
    if (m_raw.compare("TRUE", Qt::CaseInsensitive) == 0) {
        m_type  = CellType::Boolean;
        m_value = true;
        return;
    }
    if (m_raw.compare("FALSE", Qt::CaseInsensitive) == 0) {
        m_type  = CellType::Boolean;
        m_value = false;
        return;
    }
    // Number
    bool ok = false;
    double d = QLocale::system().toDouble(m_raw, &ok);
    if (!ok) d = m_raw.toDouble(&ok);
    if (ok) {
        m_type  = CellType::Number;
        m_value = d;
        return;
    }
    // Date  dd/mm/yyyy or yyyy-mm-dd
    static QRegularExpression dateRe(R"(^\d{1,4}[\/\-\.]\d{1,2}[\/\-\.]\d{2,4}$)");
    if (dateRe.match(m_raw).hasMatch()) {
        QDateTime dt = QDateTime::fromString(m_raw, Qt::ISODate);
        if (!dt.isValid()) dt = QDateTime::fromString(m_raw, "dd/MM/yyyy");
        if (dt.isValid()) {
            m_type  = CellType::Date;
            m_value = dt;
            return;
        }
    }
    // Default: text
    m_type  = CellType::Text;
    m_value = m_raw;
}

void Cell::setValue(const QVariant &v, CellType t)
{
    m_value = v;
    m_type  = t;
    m_error = CellError::None;
}

void Cell::setError(CellError err)
{
    m_error = err;
    m_type  = CellType::Error;
    m_value = errorString(err);
}

QString Cell::displayText() const
{
    if (m_type == CellType::Empty)   return {};
    if (m_type == CellType::Error)   return errorString(m_error);
    if (m_type == CellType::Boolean) return m_value.toBool() ? "TRUE" : "FALSE";

    if (m_format.numberFormat == "General" || m_format.numberFormat.isEmpty()) {
        if (m_type == CellType::Number) {
            double d = m_value.toDouble();
            // Show as integer if it is one
            if (d == static_cast<long long>(d) && qAbs(d) < 1e15)
                return QLocale::system().toString(static_cast<long long>(d));
            return QLocale::system().toString(d, 'g', 10);
        }
        if (m_type == CellType::Date)
            return m_value.toDateTime().toString("dd/MM/yyyy");
        return m_value.toString();
    }

    // TODO: apply m_format.numberFormat via NumberFormatter
    return m_value.toString();
}

bool Cell::isEmpty() const
{
    return m_type == CellType::Empty ||
           (m_type == CellType::Text && m_raw.trimmed().isEmpty());
}

QString Cell::errorString(CellError e)
{
    switch (e) {
    case CellError::DivZero:  return "#DIV/0!";
    case CellError::Name:     return "#NAME?";
    case CellError::Value:    return "#VALUE!";
    case CellError::Ref:      return "#REF!";
    case CellError::NA:       return "#N/A";
    case CellError::Null:     return "#NULL!";
    case CellError::Circular: return "#CIRC!";
    case CellError::Num:      return "#NUM!";
    default:                  return "";
    }
}

} // namespace OpenSheet
