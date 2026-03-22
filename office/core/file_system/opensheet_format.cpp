#include "opensheet_format.h"
#include "../../engine/workbook.h"
#include "../../engine/sheet.h"
#include "../../engine/cell.h"
#include "../../engine/cell_range.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>

// We use Qt's built-in ZIP via QZipWriter/QZipReader (available in Qt6)
#include <QtCore/private/qzipreader_p.h>
#include <QtCore/private/qzipwriter_p.h>

namespace OpenSheet {

bool OpenSheetFormat::write(Workbook *wb, const QString &path, QString &outError)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        outError = "Cannot create file: " + path;
        return false;
    }

    QZipWriter zip(&file);
    zip.setCompressionPolicy(QZipWriter::AutoCompress);

    zip.addFile("manifest.json", serializeManifest(wb));
    zip.addFile("styles.json",   serializeStyles(wb));
    zip.addFile("namedRanges.json", serializeNamedRanges(wb));

    for (int i = 0; i < wb->sheetCount(); ++i) {
        QString name = QString("sheets/%1.json").arg(i);
        zip.addFile(name, serializeSheet(wb->sheet(i), i));
    }

    zip.close();
    return zip.status() == QZipWriter::NoError;
}

Workbook *OpenSheetFormat::read(const QString &path, QString &outError)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        outError = "Cannot open file: " + path;
        return nullptr;
    }

    QZipReader zip(&file);
    if (!zip.isReadable()) { outError = "Not a valid .opensheet file"; return nullptr; }

    auto *wb = new Workbook();

    QByteArray manifest = zip.fileData("manifest.json");
    if (manifest.isEmpty()) { outError = "Missing manifest"; delete wb; return nullptr; }
    if (!deserializeManifest(manifest, wb)) { outError = "Bad manifest"; delete wb; return nullptr; }

    QByteArray styles = zip.fileData("styles.json");
    if (!styles.isEmpty()) deserializeStyles(styles, wb);

    // Read sheets in order
    int i = 0;
    while (true) {
        QByteArray data = zip.fileData(QString("sheets/%1.json").arg(i));
        if (data.isEmpty()) break;
        if (!deserializeSheet(data, wb, i)) {
            outError = QString("Failed reading sheet %1").arg(i);
            delete wb; return nullptr;
        }
        ++i;
    }
    return wb;
}

// ---------- Serialization ----------

QByteArray OpenSheetFormat::serializeManifest(Workbook *wb)
{
    QJsonObject obj;
    obj["version"]     = kFormatVersion;
    obj["application"] = "OpenSheet";
    obj["appVersion"]  = "1.0.0";
    obj["created"]     = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    obj["sheetCount"]  = wb->sheetCount();
    obj["activeSheet"] = wb->activeSheetIndex();

    QJsonArray sheetNames;
    for (const auto &n : wb->sheetNames()) sheetNames.append(n);
    obj["sheetNames"] = sheetNames;

    return QJsonDocument(obj).toJson(QJsonDocument::Indented);
}

QByteArray OpenSheetFormat::serializeSheet(Sheet *sheet, int /*index*/)
{
    QJsonObject root;
    root["name"]       = sheet->name();
    root["visible"]    = sheet->isVisible();
    root["freezeRow"]  = sheet->freezeRow();
    root["freezeCol"]  = sheet->freezeCol();

    QJsonArray cells;
    sheet->forEachCell([&](int row, int col, Cell &cell) {
        if (cell.isEmpty()) return;
        QJsonObject c;
        c["r"]   = row;
        c["c"]   = col;
        c["raw"] = cell.raw();
        if (cell.format() != CellFormat{})
            c["fmt"] = cellFormatToJson(cell.format());
        if (!cell.comment().isEmpty())  c["comment"]  = cell.comment();
        if (!cell.hyperlink().isEmpty()) c["link"]    = cell.hyperlink();
        if (cell.isMerged()) {
            c["mergeRows"] = cell.mergeSpanRows();
            c["mergeCols"] = cell.mergeSpanCols();
        }
        cells.append(c);
    });
    root["cells"] = cells;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray OpenSheetFormat::serializeStyles(Workbook */*wb*/)
{
    QJsonObject obj;
    obj["version"] = 1;
    return QJsonDocument(obj).toJson();
}

QByteArray OpenSheetFormat::serializeNamedRanges(Workbook */*wb*/)
{
    QJsonArray arr;
    return QJsonDocument(arr).toJson();
}

// ---------- Deserialization ----------

bool OpenSheetFormat::deserializeManifest(const QByteArray &data, Workbook *wb)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return false;
    auto obj = doc.object();
    if (obj["version"].toInt() > kFormatVersion) return false;
    Q_UNUSED(wb);
    return true;
}

bool OpenSheetFormat::deserializeSheet(const QByteArray &data, Workbook *wb, int /*index*/)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return false;

    auto root  = doc.object();
    auto *sheet = wb->addSheet(root["name"].toString());
    sheet->setVisible(root["visible"].toBool(true));
    sheet->setFreezeRow(root["freezeRow"].toInt(0));
    sheet->setFreezeCol(root["freezeCol"].toInt(0));

    for (const auto &cv : root["cells"].toArray()) {
        auto c = cv.toObject();
        int r = c["r"].toInt(), col = c["c"].toInt();
        sheet->setCell(r, col, c["raw"].toString());
        if (c.contains("fmt")) {
            sheet->cell(r, col).setFormat(jsonToCellFormat(c["fmt"].toObject()));
        }
        if (c.contains("comment"))  sheet->cell(r,col).setComment(c["comment"].toString());
        if (c.contains("link"))     sheet->cell(r,col).setHyperlink(c["link"].toString());
        if (c.contains("mergeRows")) {
            sheet->cell(r,col).setMerged(true);
            sheet->cell(r,col).setMergeSpan(c["mergeRows"].toInt(1), c["mergeCols"].toInt(1));
        }
    }
    return true;
}

bool OpenSheetFormat::deserializeStyles(const QByteArray &, Workbook *) { return true; }

QJsonObject OpenSheetFormat::cellFormatToJson(const CellFormat &fmt)
{
    QJsonObject o;
    if (fmt.bold)           o["bold"]      = true;
    if (fmt.italic)         o["italic"]    = true;
    if (fmt.underline)      o["underline"] = true;
    if (fmt.wrapText)       o["wrap"]      = true;
    if (fmt.foreground != Qt::black)
        o["fg"] = fmt.foreground.name();
    if (fmt.background != Qt::transparent)
        o["bg"] = fmt.background.name();
    if (!fmt.numberFormat.isEmpty() && fmt.numberFormat != "General")
        o["numFmt"] = fmt.numberFormat;
    o["align"] = (int)fmt.alignment;
    return o;
}

CellFormat OpenSheetFormat::jsonToCellFormat(const QJsonObject &o)
{
    CellFormat fmt;
    fmt.bold        = o["bold"].toBool();
    fmt.italic      = o["italic"].toBool();
    fmt.underline   = o["underline"].toBool();
    fmt.wrapText    = o["wrap"].toBool();
    if (o.contains("fg")) fmt.foreground  = QColor(o["fg"].toString());
    if (o.contains("bg")) fmt.background  = QColor(o["bg"].toString());
    if (o.contains("numFmt")) fmt.numberFormat = o["numFmt"].toString();
    if (o.contains("align"))  fmt.alignment    = (Qt::Alignment)o["align"].toInt();
    return fmt;
}

} // namespace OpenSheet
