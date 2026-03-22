#pragma once
#include <QObject>
#include <QString>

namespace OpenSheet {

/**
 * On launch, checks for crash-recovery files from the previous session.
 * On crash/abnormal exit, the auto-save timer writes to a temp path;
 * CrashRecovery detects this file and offers to restore it.
 */
class CrashRecovery : public QObject {
    Q_OBJECT
public:
    explicit CrashRecovery(QObject *parent = nullptr);

    // Call at startup. Returns true if a session was restored.
    bool checkAndRestore();

    // Mark session as clean (call on normal exit)
    void markClean();

    // Write a session marker (call at startup, before any file is loaded)
    void markDirty();

    QString recoveryFilePath() const;

private:
    QString m_sessionFile;
    QString m_recoveryDir;
};

} // namespace OpenSheet
