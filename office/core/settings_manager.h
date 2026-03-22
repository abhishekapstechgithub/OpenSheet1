#pragma once
#include <QObject>
#include <QVariant>
#include <QString>
#include <QSettings>

namespace OpenSheet {

class SettingsManager : public QObject {
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    void     load();
    void     save();
    QVariant value(const QString &key, const QVariant &defaultVal = {}) const;
    void     setValue(const QString &key, const QVariant &val);

    // Convenience accessors
    QString  theme()           const { return value("theme", "light").toString(); }
    QString  language()        const { return value("language", "en-us").toString(); }
    int      autoSaveInterval() const { return value("autoSaveInterval", 60).toInt(); }
    bool     showGridLines()   const { return value("showGridLines", true).toBool(); }
    bool     showFormulaBar()  const { return value("showFormulaBar", true).toBool(); }
    bool     showStatusBar()   const { return value("showStatusBar", true).toBool(); }
    int      defaultRowHeight() const { return value("defaultRowHeight", 22).toInt(); }
    int      defaultColWidth()  const { return value("defaultColWidth", 90).toInt(); }
    QStringList recentFiles()   const { return value("recentFiles").toStringList(); }

    void     addRecentFile(const QString &path);

signals:
    void settingChanged(const QString &key, const QVariant &value);

private:
    QSettings m_store;
};

} // namespace OpenSheet
