#include "csv_reader.h"
#include "../../../office/engine/workbook.h"
#include "../../../office/engine/sheet.h"
#include <QFile>
#include <QTextStream>

namespace OpenSheet {

CsvReader::CsvReader(QChar delimiter, QChar quoteChar)
    : m_delimiter(delimiter), m_quoteChar(quoteChar) {}

QChar CsvReader::detectDelimiter(const QString &firstLine)
{
    int commas     = firstLine.count(',');
    int semicolons = firstLine.count(';');
    int tabs       = firstLine.count('\t');
    int pipes      = firstLine.count('|');
    int max = std::max({commas, semicolons, tabs, pipes});
    if (max == 0)      return ',';
    if (tabs == max)   return '\t';
    if (commas == max) return ',';
    if (semicolons==max) return ';';
    return '|';
}

Workbook *CsvReader::read(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Cannot open file: " + path;
        return nullptr;
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    // Auto-detect delimiter if needed
    int firstNl = content.indexOf('\n');
    QString firstLine = (firstNl >= 0) ? content.left(firstNl) : content;
    if (m_delimiter == ',') m_delimiter = detectDelimiter(firstLine);

    auto rows = parseCSV(content);
    if (rows.isEmpty()) { m_lastError = "Empty file"; return nullptr; }

    auto *wb    = new Workbook();
    Sheet *sheet = wb->addSheet(QFileInfo(path).baseName());

    for (int r = 0; r < rows.size(); ++r) {
        const auto &row = rows[r];
        for (int c = 0; c < row.size(); ++c) {
            if (!row[c].isEmpty())
                sheet->setCell(r + 1, c + 1, row[c]);
        }
    }
    return wb;
}

QVector<QStringList> CsvReader::parseCSV(const QString &content)
{
    QVector<QStringList> result;
    QStringList lines = content.split('\n');
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty() && result.isEmpty()) continue;
        result.append(parseLine(trimmed));
    }
    // Remove trailing empty rows
    while (!result.isEmpty() && result.last().join("").trimmed().isEmpty())
        result.removeLast();
    return result;
}

QStringList CsvReader::parseLine(const QString &line)
{
    QStringList fields;
    QString current;
    bool inQuote = false;

    for (int i = 0; i < line.size(); ++i) {
        QChar ch = line[i];
        if (ch == m_quoteChar) {
            if (inQuote && i + 1 < line.size() && line[i+1] == m_quoteChar) {
                current += m_quoteChar; ++i; // escaped quote
            } else {
                inQuote = !inQuote;
            }
        } else if (ch == m_delimiter && !inQuote) {
            fields.append(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    fields.append(current);
    return fields;
}

// --- CsvWriter ---

CsvWriter::CsvWriter(QChar delimiter, QChar quoteChar)
    : m_delimiter(delimiter), m_quoteChar(quoteChar) {}

bool CsvWriter::write(Workbook *wb, const QString &path, int sheetIndex)
{
    if (!wb || sheetIndex >= wb->sheetCount()) {
        m_lastError = "Invalid workbook or sheet index";
        return false;
    }
    Sheet *sheet = wb->sheet(sheetIndex);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Cannot write file: " + path;
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    QSize used = sheet->usedRange();
    for (int r = 1; r <= used.width(); ++r) {
        QStringList rowFields;
        for (int c = 1; c <= used.height(); ++c) {
            QString val = sheet->cell(r, c).displayText();
            rowFields.append(escapeField(val));
        }
        out << rowFields.join(m_delimiter) << '\n';
    }
    return true;
}

QString CsvWriter::escapeField(const QString &field)
{
    bool needsQuote = field.contains(m_delimiter) ||
                      field.contains(m_quoteChar)  ||
                      field.contains('\n')          ||
                      field.contains('\r');
    if (!needsQuote) return field;
    QString escaped = field;
    escaped.replace(QString(m_quoteChar), QString(m_quoteChar) + m_quoteChar);
    return m_quoteChar + escaped + m_quoteChar;
}

} // namespace OpenSheet
