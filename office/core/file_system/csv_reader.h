#pragma once
#include <QString>
#include <QChar>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include "../../engine/workbook.h"
#include "../../engine/sheet.h"

namespace OpenSheet {

class CsvReader {
public:
    explicit CsvReader(QChar delimiter = ',', QChar quoteChar = '"')
        : m_delimiter(delimiter), m_quoteChar(quoteChar) {}

    static QChar detectDelimiter(const QString &firstLine) {
        int commas=firstLine.count(','), semis=firstLine.count(';'),
            tabs=firstLine.count('\t'), pipes=firstLine.count('|');
        int mx = std::max({commas,semis,tabs,pipes});
        if (mx==0)       return ',';
        if (tabs==mx)    return '\t';
        if (commas==mx)  return ',';
        if (semis==mx)   return ';';
        return '|';
    }

    Workbook *read(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            m_lastError = "Cannot open file: "+path; return nullptr;
        }
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        QString content = in.readAll();
        file.close();
        int firstNl = content.indexOf('\n');
        QString firstLine = (firstNl>=0) ? content.left(firstNl) : content;
        if (m_delimiter==',') m_delimiter = detectDelimiter(firstLine);
        auto rows = parseCSV(content);
        if (rows.isEmpty()) { m_lastError="Empty file"; return nullptr; }
        auto *wb   = new Workbook();
        Sheet *sh  = wb->addSheet(QFileInfo(path).baseName());
        for (int r=0;r<rows.size();++r) {
            const auto &row=rows[r];
            for (int c=0;c<row.size();++c)
                if (!row[c].isEmpty()) sh->setCell(r+1,c+1,row[c]);
        }
        return wb;
    }

    QString lastError() const { return m_lastError; }

private:
    QChar   m_delimiter, m_quoteChar;
    QString m_lastError;

    QVector<QStringList> parseCSV(const QString &content) {
        QVector<QStringList> result;
        for (const QString &line : content.split('\n')) {
            QString t=line.trimmed();
            if (t.isEmpty()&&result.isEmpty()) continue;
            result.append(parseLine(t));
        }
        while (!result.isEmpty()&&result.last().join("").trimmed().isEmpty())
            result.removeLast();
        return result;
    }

    QStringList parseLine(const QString &line) {
        QStringList fields;
        QString current;
        bool inQ=false;
        for (int i=0;i<line.size();++i) {
            QChar ch=line[i];
            if (ch==m_quoteChar) {
                if (inQ&&i+1<line.size()&&line[i+1]==m_quoteChar) { current+=m_quoteChar;++i; }
                else { inQ=!inQ; }
            } else if (ch==m_delimiter&&!inQ) { fields.append(current); current.clear(); }
            else { current+=ch; }
        }
        fields.append(current);
        return fields;
    }
};

class CsvWriter {
public:
    explicit CsvWriter(QChar delimiter=',', QChar quoteChar='"')
        : m_delimiter(delimiter), m_quoteChar(quoteChar) {}

    bool write(Workbook *wb, const QString &path, int sheetIndex=0) {
        if (!wb||sheetIndex>=wb->sheetCount()) { m_lastError="Invalid workbook or sheet index"; return false; }
        Sheet *sheet=wb->sheet(sheetIndex);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) { m_lastError="Cannot write: "+path; return false; }
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        QSize used=sheet->usedRange();
        for (int r=1;r<=used.width();++r) {
            QStringList row;
            for (int c=1;c<=used.height();++c)
                row.append(escapeField(sheet->cell(r,c).displayText()));
            out<<row.join(m_delimiter)<<'\n';
        }
        return true;
    }

    QString lastError() const { return m_lastError; }

private:
    QChar m_delimiter, m_quoteChar;
    QString m_lastError;

    QString escapeField(const QString &f) {
        bool need=f.contains(m_delimiter)||f.contains(m_quoteChar)||f.contains('\n')||f.contains('\r');
        if (!need) return f;
        QString e=f; e.replace(QString(m_quoteChar),QString(m_quoteChar)+m_quoteChar);
        return m_quoteChar+e+m_quoteChar;
    }
};

} // namespace OpenSheet
