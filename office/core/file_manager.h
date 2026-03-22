#pragma once
#include <QString>
#include <QFileInfo>
#include "file_system/opensheet_format.h"
#include "file_system/xlsx_reader.h"
#include "file_system/csv_reader.h"
#include "../engine/workbook.h"

namespace OpenSheet {

/**
 * Dispatches open/save to the correct format handler based on file extension.
 * Header-only to avoid a separate .cpp translation unit.
 */
class FileManager {
public:
    FileManager() = default;

    enum class Format { OpenSheet, XLSX, CSV, Unknown };

    static Format detectFormat(const QString &path) {
        QString ext = QFileInfo(path).suffix().toLower();
        if (ext == "opensheet")                        return Format::OpenSheet;
        if (ext == "xlsx" || ext == "xls")             return Format::XLSX;
        if (ext == "csv" || ext == "tsv" || ext == "txt") return Format::CSV;
        return Format::Unknown;
    }

    Workbook *open(const QString &path) {
        m_lastError.clear();
        if (!QFileInfo::exists(path)) {
            m_lastError = "File not found: " + path;
            return nullptr;
        }
        switch (detectFormat(path)) {
        case Format::OpenSheet: {
            return OpenSheetFormat::read(path, m_lastError);
        }
        case Format::XLSX: {
            XlsxReader reader;
            Workbook *wb = reader.read(path);
            if (!wb) m_lastError = reader.lastError();
            return wb;
        }
        case Format::CSV: {
            CsvReader reader;
            Workbook *wb = reader.read(path);
            if (!wb) m_lastError = reader.lastError();
            return wb;
        }
        default:
            m_lastError = "Unsupported format: " + QFileInfo(path).suffix();
            return nullptr;
        }
    }

    bool save(Workbook *wb, const QString &path) {
        m_lastError.clear();
        if (!wb) { m_lastError = "No workbook to save"; return false; }
        switch (detectFormat(path)) {
        case Format::OpenSheet:
            return OpenSheetFormat::write(wb, path, m_lastError);
        case Format::XLSX: {
            XlsxWriter writer;
            bool ok = writer.write(wb, path);
            if (!ok) m_lastError = writer.lastError();
            return ok;
        }
        case Format::CSV: {
            CsvWriter writer;
            bool ok = writer.write(wb, path, wb->activeSheetIndex());
            if (!ok) m_lastError = writer.lastError();
            return ok;
        }
        default:
            return OpenSheetFormat::write(wb, path + ".opensheet", m_lastError);
        }
    }

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};

} // namespace OpenSheet
