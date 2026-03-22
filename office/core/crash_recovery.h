#pragma once
#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <QStandardPaths>

namespace OpenSheet {

class CrashRecovery : public QObject {
    Q_OBJECT
public:
    explicit CrashRecovery(QObject *parent = nullptr) : QObject(parent) {
        m_recoveryDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/temp";
        m_sessionFile = m_recoveryDir + "/session.lock";
        QDir().mkpath(m_recoveryDir);
    }

    bool checkAndRestore() {
        if (!QFile::exists(m_sessionFile)) { markDirty(); return false; }
        QDir dir(m_recoveryDir);
        QStringList saves = dir.entryList({"autosave_*.opensheet"}, QDir::Files, QDir::Time);
        if (saves.isEmpty()) { markDirty(); return false; }
        QString candidate = m_recoveryDir + "/" + saves.first();
        int btn = QMessageBox::question(nullptr, "Crash Recovery",
            "OpenSheet did not exit cleanly.\n\nAn auto-saved file was found:\n" + candidate +
            "\n\nWould you like to restore it?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
        if (btn == QMessageBox::Yes) { setProperty("recoveryFile", candidate); markDirty(); return true; }
        QFile::remove(candidate); markDirty(); return false;
    }

    void markClean() { QFile::remove(m_sessionFile); }
    void markDirty() {
        QFile f(m_sessionFile);
        if (f.open(QIODevice::WriteOnly|QIODevice::Text))
            f.write(QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toUtf8());
    }
    QString recoveryFilePath() const {
        return m_recoveryDir + "/autosave_" +
               QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".opensheet";
    }

private:
    QString m_recoveryDir, m_sessionFile;
};

} // namespace OpenSheet
