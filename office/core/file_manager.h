#pragma once
#include <QString>

namespace OpenSheet {

class Workbook;

/**
 * Dispatches open/save to the correct format handler
 * based on file extension.
 */
class FileManager {
public:
    FileManager() = default;

    Workbook *open(const QString &path);
    bool      save(Workbook *wb, const QString &path);

    QString   lastError() const { return m_lastError; }

    // Format detection
    enum class Format { OpenSheet, XLSX, CSV, Unknown };
    static Format detectFormat(const QString &path);

private:
    QString m_lastError;
};

} // namespace OpenSheet
