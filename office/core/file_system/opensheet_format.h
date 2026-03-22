#pragma once
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include "../../engine/workbook.h"
#include "../../engine/sheet.h"
#include "../../engine/cell.h"
#include "../../engine/cell_range.h"

#ifdef OPENSHEET_HAS_QT_CORE_PRIVATE
#  include <QtCore/private/qzipreader_p.h>
#  include <QtCore/private/qzipwriter_p.h>
#endif

namespace OpenSheet {

class OpenSheetFormat {
public:
    static constexpr int kFormatVersion = 1;

    static bool write(Workbook *wb, const QString &path, QString &outError) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) { outError="Cannot create file: "+path; return false; }
#ifdef OPENSHEET_HAS_QT_CORE_PRIVATE
        QZipWriter zip(&file);
        zip.setCompressionPolicy(QZipWriter::AutoCompress);
        zip.addFile("manifest.json",    serializeManifest(wb));
        zip.addFile("styles.json",      serializeStyles(wb));
        zip.addFile("namedRanges.json", serializeNamedRanges(wb));
        for (int i=0;i<wb->sheetCount();++i)
            zip.addFile(QString("sheets/%1.json").arg(i), serializeSheet(wb->sheet(i),i));
        zip.close();
        return zip.status()==QZipWriter::NoError;
#else
        QJsonObject root;
        root["manifest"]=QJsonDocument::fromJson(serializeManifest(wb)).object();
        QJsonArray sheets;
        for (int i=0;i<wb->sheetCount();++i)
            sheets.append(QJsonDocument::fromJson(serializeSheet(wb->sheet(i),i)).object());
        root["sheets"]=sheets;
        file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
        return true;
#endif
    }

    static Workbook *read(const QString &path, QString &outError) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) { outError="Cannot open file: "+path; return nullptr; }
#ifdef OPENSHEET_HAS_QT_CORE_PRIVATE
        QZipReader zip(&file);
        if (!zip.isReadable()) { outError="Not a valid .opensheet file"; return nullptr; }
        auto *wb=new Workbook();
        QByteArray manifest=zip.fileData("manifest.json");
        if (manifest.isEmpty()) { outError="Missing manifest"; delete wb; return nullptr; }
        if (!deserializeManifest(manifest,wb)) { outError="Bad manifest"; delete wb; return nullptr; }
        QByteArray styles=zip.fileData("styles.json");
        if (!styles.isEmpty()) deserializeStyles(styles,wb);
        for (int i=0;;++i) {
            QByteArray data=zip.fileData(QString("sheets/%1.json").arg(i));
            if (data.isEmpty()) break;
            if (!deserializeSheet(data,wb,i)) { outError=QString("Failed reading sheet %1").arg(i); delete wb; return nullptr; }
        }
        return wb;
#else
        QByteArray raw=file.readAll();
        QJsonParseError err;
        QJsonDocument doc=QJsonDocument::fromJson(raw,&err);
        if (err.error!=QJsonParseError::NoError) { outError="JSON parse error: "+err.errorString(); return nullptr; }
        auto root=doc.object();
        auto *wb=new Workbook();
        if (!deserializeManifest(QJsonDocument(root["manifest"].toObject()).toJson(),wb)) { outError="Bad manifest"; delete wb; return nullptr; }
        QJsonArray sheets=root["sheets"].toArray();
        for (int i=0;i<sheets.size();++i) {
            if (!deserializeSheet(QJsonDocument(sheets[i].toObject()).toJson(),wb,i)) { outError=QString("Failed reading sheet %1").arg(i); delete wb; return nullptr; }
        }
        return wb;
#endif
    }

private:
    static QByteArray serializeManifest(Workbook *wb) {
        QJsonObject o;
        o["version"]=kFormatVersion; o["application"]="OpenSheet"; o["appVersion"]="1.0.0";
        o["created"]=QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        o["sheetCount"]=wb->sheetCount(); o["activeSheet"]=wb->activeSheetIndex();
        QJsonArray names; for (const auto &n:wb->sheetNames()) names.append(n);
        o["sheetNames"]=names;
        return QJsonDocument(o).toJson(QJsonDocument::Indented);
    }

    static QByteArray serializeSheet(Sheet *sheet, int) {
        QJsonObject root;
        root["name"]=sheet->name(); root["visible"]=sheet->isVisible();
        root["freezeRow"]=sheet->freezeRow(); root["freezeCol"]=sheet->freezeCol();
        QJsonArray cells;
        sheet->forEachCell([&](int r,int c,Cell &cell) {
            if (cell.isEmpty()) return;
            QJsonObject o; o["r"]=r; o["c"]=c; o["raw"]=cell.raw();
            if (cell.format()!=CellFormat{}) o["fmt"]=cellFormatToJson(cell.format());
            if (!cell.comment().isEmpty())   o["comment"]=cell.comment();
            if (!cell.hyperlink().isEmpty()) o["link"]=cell.hyperlink();
            cells.append(o);
        });
        root["cells"]=cells;
        return QJsonDocument(root).toJson(QJsonDocument::Compact);
    }

    static QByteArray serializeStyles(Workbook *)  { return QJsonDocument(QJsonObject{{"version",1}}).toJson(); }
    static QByteArray serializeNamedRanges(Workbook *) { return QJsonDocument(QJsonArray{}).toJson(); }

    static bool deserializeManifest(const QByteArray &data, Workbook *wb) {
        QJsonParseError e; auto doc=QJsonDocument::fromJson(data,&e);
        if (e.error!=QJsonParseError::NoError) return false;
        if (doc.object()["version"].toInt()>kFormatVersion) return false;
        Q_UNUSED(wb); return true;
    }

    static bool deserializeSheet(const QByteArray &data, Workbook *wb, int) {
        QJsonParseError e; auto doc=QJsonDocument::fromJson(data,&e);
        if (e.error!=QJsonParseError::NoError) return false;
        auto root=doc.object();
        auto *sheet=wb->addSheet(root["name"].toString());
        sheet->setVisible(root["visible"].toBool(true));
        sheet->setFreezeRow(root["freezeRow"].toInt(0));
        sheet->setFreezeCol(root["freezeCol"].toInt(0));
        for (const auto &cv:root["cells"].toArray()) {
            auto c=cv.toObject();
            int r=c["r"].toInt(), col=c["c"].toInt();
            sheet->setCell(r,col,c["raw"].toString());
            if (c.contains("fmt"))     sheet->cell(r,col).setFormat(jsonToCellFormat(c["fmt"].toObject()));
            if (c.contains("comment")) sheet->cell(r,col).setComment(c["comment"].toString());
            if (c.contains("link"))    sheet->cell(r,col).setHyperlink(c["link"].toString());
        }
        return true;
    }

    static bool deserializeStyles(const QByteArray &, Workbook *) { return true; }

    static QJsonObject cellFormatToJson(const CellFormat &fmt) {
        QJsonObject o;
        if (fmt.bold)      o["bold"]=true;
        if (fmt.italic)    o["italic"]=true;
        if (fmt.underline) o["underline"]=true;
        if (fmt.wrapText)  o["wrap"]=true;
        if (fmt.foreground!=Qt::black)       o["fg"]=fmt.foreground.name();
        if (fmt.background!=Qt::transparent) o["bg"]=fmt.background.name();
        if (!fmt.numberFormat.isEmpty()&&fmt.numberFormat!="General") o["numFmt"]=fmt.numberFormat;
        o["align"]=(int)fmt.alignment;
        return o;
    }

    static CellFormat jsonToCellFormat(const QJsonObject &o) {
        CellFormat fmt;
        fmt.bold=o["bold"].toBool(); fmt.italic=o["italic"].toBool();
        fmt.underline=o["underline"].toBool(); fmt.wrapText=o["wrap"].toBool();
        if (o.contains("fg"))     fmt.foreground=QColor(o["fg"].toString());
        if (o.contains("bg"))     fmt.background=QColor(o["bg"].toString());
        if (o.contains("numFmt")) fmt.numberFormat=o["numFmt"].toString();
        if (o.contains("align"))  fmt.alignment=(Qt::Alignment)o["align"].toInt();
        return fmt;
    }
};

} // namespace OpenSheet
