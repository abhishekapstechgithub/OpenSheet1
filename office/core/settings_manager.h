#pragma once
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <functional>

namespace OpenSheet {

class SettingsManager {
public:
    explicit SettingsManager()
        : m_store(QSettings::IniFormat, QSettings::UserScope, "OpenSheet", "OpenSheet")
    {}

    void load() {}
    void save() { m_store.sync(); }

    QVariant value(const QString &key, const QVariant &defaultVal = {}) const {
        return m_store.value(key, defaultVal);
    }
    void setValue(const QString &key, const QVariant &val) {
        m_store.setValue(key, val);
        for (auto &cb : m_callbacks) cb(key, val);
    }

    // Observer pattern replaces Qt signals (no Q_OBJECT needed)
    void onSettingChanged(std::function<void(const QString&, const QVariant&)> cb) {
        m_callbacks.push_back(std::move(cb));
    }

    QString     theme()               const { return m_store.value("theme","light").toString(); }
    bool        showGridLines()       const { return m_store.value("showGridLines",true).toBool(); }
    int         defaultRowHeight()    const { return m_store.value("defaultRowHeight",22).toInt(); }
    int         defaultColWidth()     const { return m_store.value("defaultColWidth",90).toInt(); }
    int         autosaveIntervalSec() const { return m_store.value("autosaveInterval",60).toInt(); }
    QString     language()            const { return m_store.value("language","en-us").toString(); }
    QStringList recentFiles()         const { return m_store.value("recentFiles").toStringList(); }

    void addRecentFile(const QString &path) {
        QStringList recent = recentFiles();
        recent.removeAll(path); recent.prepend(path);
        while (recent.size() > 10) recent.removeLast();
        setValue("recentFiles", recent);
    }

private:
    QSettings m_store;
    std::vector<std::function<void(const QString&, const QVariant&)>> m_callbacks;
};

} // namespace OpenSheet
