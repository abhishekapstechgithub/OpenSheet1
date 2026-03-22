#pragma once
#include <QString>

namespace OpenSheet {

class Workbook;

/**
 * Native .opensheet format — a ZIP archive containing:
 *   manifest.json     - version, metadata
 *   sheets/N.json     - sheet data (cells, row/col props)
 *   styles.json       - global style table
 *   namedRanges.json  - named ranges
 *   charts/           - chart definitions
 *   media/            - embedded images
 */
class OpenSheetFormat {
public:
    static bool write(Workbook *wb, const QString &path, QString &outError);
    static Workbook *read(const QString &path, QString &outError);

private:
    // Write helpers
    static QByteArray serializeManifest(Workbook *wb);
    static QByteArray serializeSheet(class Sheet *sheet, int index);
    static QByteArray serializeStyles(Workbook *wb);
    static QByteArray serializeNamedRanges(Workbook *wb);

    // Read helpers
    static bool       deserializeManifest(const QByteArray &data, Workbook *wb);
    static bool       deserializeSheet(const QByteArray &data, Workbook *wb, int index);
    static bool       deserializeStyles(const QByteArray &data, Workbook *wb);

    // Cell format <-> JSON
    static QJsonObject cellFormatToJson(const struct CellFormat &fmt);
    static struct CellFormat jsonToCellFormat(const QJsonObject &obj);

    static constexpr int kFormatVersion = 1;
};

} // namespace OpenSheet
