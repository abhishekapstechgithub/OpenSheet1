#pragma once
#include <QString>

namespace OpenSheet {

class Workbook;

/**
 * Reads .xlsx files (Office Open XML format).
 * .xlsx is a ZIP archive containing XML files:
 *   xl/workbook.xml      - sheet list
 *   xl/worksheets/       - sheet data
 *   xl/sharedStrings.xml - string table
 *   xl/styles.xml        - cell styles
 */
class XlsxReader {
public:
    XlsxReader() = default;

    Workbook *read(const QString &path);
    QString   lastError() const { return m_lastError; }

private:
    QString m_lastError;

    bool parseWorkbook(const QByteArray &xml, Workbook *wb, QStringList &sheetFiles);
    bool parseSheet(const QByteArray &xml, class Sheet *sheet,
                    const QStringList &sharedStrings);
    bool parseSharedStrings(const QByteArray &xml, QStringList &strings);
    bool parseStyles(const QByteArray &xml, Workbook *wb);

    // Decode Excel cell address "B3" -> row=3, col=2
    static bool decodeAddress(const QString &addr, int &row, int &col);

    // Excel stores dates as days since 1900-01-00
    static QVariant excelSerialToDate(double serial);
};

class XlsxWriter {
public:
    XlsxWriter() = default;
    bool write(Workbook *wb, const QString &path);
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
    QByteArray buildWorkbookXml(Workbook *wb);
    QByteArray buildSheetXml(class Sheet *sheet, QStringList &sharedStrings);
    QByteArray buildSharedStringsXml(const QStringList &strings);
    QByteArray buildStylesXml(Workbook *wb);
    QByteArray buildContentTypes(Workbook *wb);
    QByteArray buildRels();
};

} // namespace OpenSheet
