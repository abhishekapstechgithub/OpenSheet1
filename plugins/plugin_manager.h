#pragma once
#include <QObject>
#include <QVector>
#include <QString>
#include <QJsonObject>

namespace OpenSheet {

/**
 * Interface every C++ plugin must implement.
 * Loaded via QPluginLoader from shared libraries in addons/plugins/.
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual QString  name()        const = 0;
    virtual QString  version()     const = 0;
    virtual QString  description() const = 0;
    virtual QString  author()      const = 0;

    // Called after plugin is loaded
    virtual bool initialize(class PluginContext *ctx) = 0;
    // Called before plugin is unloaded
    virtual void shutdown() = 0;
};

} // namespace OpenSheet

// Q_DECLARE_INTERFACE must be at global scope (not inside a namespace)
Q_DECLARE_INTERFACE(OpenSheet::IPlugin, "io.opensheet.IPlugin/1.0")

namespace OpenSheet {

struct PluginInfo {
    QString  path;
    QString  name;
    QString  version;
    QString  description;
    QString  author;
    QString  type;          // "cpp" | "python"
    bool     loaded = false;
    bool     enabled = true;
    IPlugin *instance = nullptr;
    QJsonObject manifest;
};

/**
 * Scans plugin directories, loads/unloads plugins,
 * exposes them to the application.
 */
class PluginManager : public QObject {
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    // Scan a directory for plugins (*.so / *.dll / *.py / plugin.json)
    void scanAndLoad(const QString &dirPath);
    void unloadAll();

    int            pluginCount()            const { return m_plugins.size(); }
    const PluginInfo &pluginInfo(int index) const { return m_plugins[index]; }

    bool enablePlugin(int index);
    bool disablePlugin(int index);
    bool reloadPlugin(int index);

    QVector<PluginInfo> enabledPlugins() const;

signals:
    void pluginLoaded(const PluginInfo &info);
    void pluginUnloaded(const QString &name);
    void pluginError(const QString &name, const QString &error);

private:
    bool loadCppPlugin(PluginInfo &info);
    bool loadPythonPlugin(PluginInfo &info);
    void readManifest(PluginInfo &info, const QString &manifestPath);

    QVector<PluginInfo> m_plugins;
};

/**
 * Passed to plugins on initialize() so they can
 * hook into the application without direct coupling.
 */
class PluginContext {
public:
    explicit PluginContext(class Workbook *wb) : m_workbook(wb) {}

    Workbook *workbook() const { return m_workbook; }

    // Add a menu item to the "Add-ins" menu
    void addMenuItem(const QString &label,
                     std::function<void()> callback);

    // Register a custom formula function
    void registerFormula(const QString &name,
                         std::function<double(const QVector<double>&)> fn);

    // Log a message
    void log(const QString &msg);

private:
    Workbook *m_workbook;
};

} // namespace OpenSheet
