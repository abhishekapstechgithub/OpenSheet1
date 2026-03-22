#pragma once
#include <QString>
#include <QChar>

namespace OpenSheet {

class Workbook;
class Sheet;

class CsvReader {
public:
    explicit CsvReader(QChar delimiter = ',', QChar quoteChar = '"');

    Workbook *read(const QString &path);
    QString   lastError() const { return m_lastError; }

    // Auto-detect delimiter from first line
    static QChar detectDelimiter(const QString &firstLine);

private:
    QChar   m_delimiter;
    QChar   m_quoteChar;
    QString m_lastError;

    QVector<QStringList> parseCSV(const QString &content);
    QStringList          parseLine(const QString &line);
};

class CsvWriter {
public:
    explicit CsvWriter(QChar delimiter = ',', QChar quoteChar = '"');
    bool    write(Workbook *wb, const QString &path, int sheetIndex = 0);
    QString lastError() const { return m_lastError; }

private:
    QChar   m_delimiter;
    QChar   m_quoteChar;
    QString m_lastError;
    QString escapeField(const QString &field);
};

} // namespace OpenSheet
