#pragma once
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDateTime>
#include <QVariant>
#include "../../engine/workbook.h"
#include "../../engine/sheet.h"
#include "../../engine/cell.h"

#ifdef OPENSHEET_HAS_QT_CORE_PRIVATE
#  include <QtCore/private/qzipreader_p.h>
#  include <QtCore/private/qzipwriter_p.h>
#endif

namespace OpenSheet {

class XlsxReader {
public:
    XlsxReader() = default;
    QString lastError() const { return m_lastError; }

    Workbook *read(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) { m_lastError="Cannot open: "+path; return nullptr; }
#ifdef OPENSHEET_HAS_QT_CORE_PRIVATE
        QZipReader zip(&file);
        if (!zip.isReadable()) { m_lastError="Not a valid ZIP/xlsx file"; return nullptr; }
        QStringList ss;
        QByteArray ssData=zip.fileData("xl/sharedStrings.xml");
        if (!ssData.isEmpty()) parseSharedStrings(ssData,ss);
        QByteArray wbData=zip.fileData("xl/workbook.xml");
        if (wbData.isEmpty()) { m_lastError="Missing xl/workbook.xml"; return nullptr; }
        auto *wb=new Workbook(); QStringList sheetFiles;
        if (!parseWorkbook(wbData,wb,sheetFiles)) { delete wb; return nullptr; }
        for (int i=0;i<sheetFiles.size();++i) {
            QByteArray shData=zip.fileData("xl/worksheets/"+sheetFiles[i]);
            if (shData.isEmpty()) continue;
            Sheet *sh=wb->sheet(i); if (!sh) continue;
            parseSheet(shData,sh,ss);
        }
        return wb;
#else
        m_lastError="XLSX read requires Qt private headers (build with Qt6::CorePrivate)";
        return nullptr;
#endif
    }

private:
    QString m_lastError;

    bool parseWorkbook(const QByteArray &xml, Workbook *wb, QStringList &sheetFiles) {
        QXmlStreamReader r(xml);
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement()&&r.name()==u"sheet") {
                QString name=r.attributes().value("name").toString();
                if (name.isEmpty()) name=QString("Sheet%1").arg(wb->sheetCount()+1);
                wb->addSheet(name);
                sheetFiles.append(QString("sheet%1.xml").arg(sheetFiles.size()+1));
            }
        }
        if (wb->sheetCount()==0) wb->addSheet("Sheet1");
        return true;
    }

    bool parseSheet(const QByteArray &xml, Sheet *sheet, const QStringList &ss) {
        QXmlStreamReader r(xml);
        QString addr,type; bool inV=false; QString vText;
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement()) {
                if (r.name()==u"c") { addr=r.attributes().value("r").toString(); type=r.attributes().value("t").toString(); inV=false; vText=""; }
                else if (r.name()==u"v"||r.name()==u"t") { inV=true; vText=""; }
            } else if (r.isCharacters()&&inV) { vText+=r.text().toString(); }
            else if (r.isEndElement()) {
                if (r.name()==u"v"||r.name()==u"t") { inV=false; }
                else if (r.name()==u"c"&&!addr.isEmpty()) {
                    int row=0,col=0;
                    if (decodeAddress(addr,row,col)) {
                        QString val;
                        if (type=="s") { int idx=vText.toInt(); val=(idx>=0&&idx<ss.size())?ss[idx]:vText; }
                        else if (type=="b") val=(vText=="1")?"TRUE":"FALSE";
                        else val=vText;
                        if (!val.isEmpty()) sheet->setCell(row,col,val);
                    }
                    addr.clear();
                }
            }
        }
        return true;
    }

    bool parseSharedStrings(const QByteArray &xml, QStringList &strings) {
        QXmlStreamReader r(xml); QString cur; bool inT=false;
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement()&&r.name()==u"t") { inT=true; cur=""; }
            else if (r.isCharacters()&&inT) { cur+=r.text().toString(); }
            else if (r.isEndElement()) {
                if (r.name()==u"t") inT=false;
                else if (r.name()==u"si") { strings.append(cur); cur.clear(); }
            }
        }
        return true;
    }

    static bool decodeAddress(const QString &addr, int &row, int &col) {
        int i=0; col=0;
        while (i<addr.size()&&addr[i].isLetter()) { col=col*26+(addr[i].toUpper().toLatin1()-'A'+1); ++i; }
        row=addr.mid(i).toInt();
        return row>0&&col>0;
    }

    static QVariant excelSerialToDate(double serial) {
        int d=static_cast<int>(serial); if (d>59) --d;
        return QDate(1899,12,31).addDays(d).toString("dd/MM/yyyy");
    }
};

class XlsxWriter {
public:
    XlsxWriter() = default;
    QString lastError() const { return m_lastError; }

    bool write(Workbook *wb, const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) { m_lastError="Cannot create: "+path; return false; }
#ifdef OPENSHEET_HAS_QT_CORE_PRIVATE
        QZipWriter zip(&file);
        zip.setCompressionPolicy(QZipWriter::AutoCompress);
        QStringList ss;
        zip.addFile("[Content_Types].xml", buildContentTypes(wb));
        zip.addFile("_rels/.rels",         buildRels());
        zip.addFile("xl/workbook.xml",     buildWorkbookXml(wb));
        for (int i=0;i<wb->sheetCount();++i)
            zip.addFile(QString("xl/worksheets/sheet%1.xml").arg(i+1), buildSheetXml(wb->sheet(i),ss));
        if (!ss.isEmpty()) zip.addFile("xl/sharedStrings.xml", buildSharedStringsXml(ss));
        zip.addFile("xl/styles.xml", buildStylesXml());
        zip.close();
        return zip.status()==QZipWriter::NoError;
#else
        m_lastError="XLSX write requires Qt private headers (build with Qt6::CorePrivate)";
        return false;
#endif
    }

private:
    QString m_lastError;

    QByteArray buildWorkbookXml(Workbook *wb) {
        QByteArray buf; QXmlStreamWriter x(&buf); x.setAutoFormatting(true);
        x.writeStartDocument();
        x.writeStartElement("workbook");
        x.writeAttribute("xmlns","http://schemas.openxmlformats.org/spreadsheetml/2006/main");
        x.writeStartElement("sheets");
        for (int i=0;i<wb->sheetCount();++i) {
            x.writeStartElement("sheet");
            x.writeAttribute("name",wb->sheet(i)->name());
            x.writeAttribute("sheetId",QString::number(i+1));
            x.writeAttribute("r:id",QString("rId%1").arg(i+1));
            x.writeEndElement();
        }
        x.writeEndElement(); x.writeEndElement(); x.writeEndDocument();
        return buf;
    }

    QByteArray buildSheetXml(Sheet *sheet, QStringList &ss) {
        QByteArray buf; QXmlStreamWriter x(&buf); x.setAutoFormatting(false);
        x.writeStartDocument();
        x.writeStartElement("worksheet");
        x.writeAttribute("xmlns","http://schemas.openxmlformats.org/spreadsheetml/2006/main");
        x.writeStartElement("sheetData");
        QSize used=sheet->usedRange();
        for (int r=1;r<=used.width();++r) {
            bool hasData=false;
            for (int c=1;c<=used.height();++c) if (sheet->hasCell(r,c)) { hasData=true; break; }
            if (!hasData) continue;
            x.writeStartElement("row"); x.writeAttribute("r",QString::number(r));
            for (int c=1;c<=used.height();++c) {
                if (!sheet->hasCell(r,c)) continue;
                const Cell &cell=sheet->cell(r,c);
                QString addr; int tmp=c;
                while (tmp>0) { --tmp; addr.prepend(QChar('A'+tmp%26)); tmp/=26; }
                addr+=QString::number(r);
                x.writeStartElement("c"); x.writeAttribute("r",addr);
                if (cell.hasFormula()) {
                    x.writeTextElement("f",cell.raw().mid(1));
                    x.writeTextElement("v",cell.value().isValid()?cell.value().toString():"");
                } else if (cell.type()==CellType::Text) {
                    int idx=ss.indexOf(cell.raw()); if (idx<0) { idx=ss.size(); ss.append(cell.raw()); }
                    x.writeAttribute("t","s"); x.writeTextElement("v",QString::number(idx));
                } else { x.writeTextElement("v",cell.raw()); }
                x.writeEndElement();
            }
            x.writeEndElement();
        }
        x.writeEndElement(); x.writeEndElement(); x.writeEndDocument();
        return buf;
    }

    QByteArray buildSharedStringsXml(const QStringList &strings) {
        QByteArray buf; QXmlStreamWriter x(&buf); x.setAutoFormatting(false);
        x.writeStartDocument();
        x.writeStartElement("sst");
        x.writeAttribute("xmlns","http://schemas.openxmlformats.org/spreadsheetml/2006/main");
        x.writeAttribute("count",QString::number(strings.size()));
        x.writeAttribute("uniqueCount",QString::number(strings.size()));
        for (const QString &s:strings) { x.writeStartElement("si"); x.writeTextElement("t",s); x.writeEndElement(); }
        x.writeEndElement(); x.writeEndDocument();
        return buf;
    }

    QByteArray buildStylesXml() {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
               "<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
               "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill>"
               "<fill><patternFill patternType=\"gray125\"/></fill></fills>"
               "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
               "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
               "<cellXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/></cellXfs>"
               "</styleSheet>";
    }

    QByteArray buildContentTypes(Workbook *wb) {
        QByteArray buf; QXmlStreamWriter x(&buf); x.setAutoFormatting(true);
        x.writeStartDocument();
        x.writeStartElement("Types"); x.writeAttribute("xmlns","http://schemas.openxmlformats.org/package/2006/content-types");
        x.writeEmptyElement("Default"); x.writeAttribute("Extension","rels"); x.writeAttribute("ContentType","application/vnd.openxmlformats-package.relationships+xml");
        x.writeEmptyElement("Default"); x.writeAttribute("Extension","xml"); x.writeAttribute("ContentType","application/xml");
        x.writeEmptyElement("Override"); x.writeAttribute("PartName","/xl/workbook.xml"); x.writeAttribute("ContentType","application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml");
        for (int i=0;i<wb->sheetCount();++i) {
            x.writeEmptyElement("Override");
            x.writeAttribute("PartName",QString("/xl/worksheets/sheet%1.xml").arg(i+1));
            x.writeAttribute("ContentType","application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml");
        }
        x.writeEmptyElement("Override"); x.writeAttribute("PartName","/xl/sharedStrings.xml"); x.writeAttribute("ContentType","application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml");
        x.writeEmptyElement("Override"); x.writeAttribute("PartName","/xl/styles.xml"); x.writeAttribute("ContentType","application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml");
        x.writeEndElement(); x.writeEndDocument();
        return buf;
    }

    QByteArray buildRels() {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
               "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
               "</Relationships>";
    }
};

} // namespace OpenSheet
