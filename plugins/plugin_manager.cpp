#include "plugin_manager.h"
#include <QDir>
#include <QPluginLoader>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

#ifdef OPENSHEET_PYTHON_SUPPORT
#  include <Python.h>
#endif

namespace OpenSheet {

PluginManager::PluginManager(QObject *parent) : QObject(parent) {}

PluginManager::~PluginManager() { unloadAll(); }

void PluginManager::scanAndLoad(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        qWarning() << "Plugin directory not found:" << dirPath;
        return;
    }

    // C++ shared lib plugins
    QStringList libFilter;
#if defined(Q_OS_WIN)
    libFilter << "*.dll";
#elif defined(Q_OS_MAC)
    libFilter << "*.dylib";
#else
    libFilter << "*.so";
#endif

    for (const QFileInfo &fi : dir.entryInfoList(libFilter, QDir::Files)) {
        PluginInfo info;
        info.path = fi.absoluteFilePath();
        info.type = "cpp";

        // Look for manifest.json alongside the lib
        QString manifestPath = fi.absolutePath() + "/" + fi.baseName() + ".json";
        if (QFile::exists(manifestPath)) readManifest(info, manifestPath);
        if (info.name.isEmpty()) info.name = fi.baseName();

        if (loadCppPlugin(info)) {
            m_plugins.append(info);
            emit pluginLoaded(info);
        }
    }

    // Python plugins: any subdirectory with plugin.json + main.py
    for (const QFileInfo &fi : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString manifestPath = fi.absoluteFilePath() + "/plugin.json";
        QString mainPath     = fi.absoluteFilePath() + "/main.py";
        if (!QFile::exists(manifestPath) || !QFile::exists(mainPath)) continue;

        PluginInfo info;
        info.type = "python";
        info.path = fi.absoluteFilePath();
        readManifest(info, manifestPath);
        if (info.name.isEmpty()) info.name = fi.fileName();

        if (loadPythonPlugin(info)) {
            m_plugins.append(info);
            emit pluginLoaded(info);
        }
    }

    qInfo() << "Loaded" << m_plugins.size() << "plugin(s) from" << dirPath;
}

void PluginManager::unloadAll()
{
    for (auto &p : m_plugins) {
        if (p.loaded && p.instance) {
            p.instance->shutdown();
            p.instance = nullptr;
        }
        p.loaded = false;
    }
}

bool PluginManager::enablePlugin(int index)
{
    if (index < 0 || index >= m_plugins.size()) return false;
    m_plugins[index].enabled = true;
    return loadCppPlugin(m_plugins[index]);
}

bool PluginManager::disablePlugin(int index)
{
    if (index < 0 || index >= m_plugins.size()) return false;
    auto &p = m_plugins[index];
    if (p.instance) { p.instance->shutdown(); p.instance = nullptr; }
    p.enabled = false;
    p.loaded  = false;
    emit pluginUnloaded(p.name);
    return true;
}

bool PluginManager::reloadPlugin(int index)
{
    if (index < 0 || index >= m_plugins.size()) return false;
    disablePlugin(index);
    m_plugins[index].enabled = true;
    return loadCppPlugin(m_plugins[index]);
}

QVector<PluginInfo> PluginManager::enabledPlugins() const
{
    QVector<PluginInfo> result;
    for (const auto &p : m_plugins)
        if (p.enabled && p.loaded) result.append(p);
    return result;
}

bool PluginManager::loadCppPlugin(PluginInfo &info)
{
    QPluginLoader loader(info.path);
    QObject *obj = loader.instance();
    if (!obj) {
        QString err = loader.errorString();
        emit pluginError(info.name, err);
        qWarning() << "Plugin load failed:" << info.path << "-" << err;
        return false;
    }

    IPlugin *plugin = qobject_cast<IPlugin*>(obj);
    if (!plugin) {
        emit pluginError(info.name, "Does not implement IPlugin interface");
        loader.unload();
        return false;
    }

    // Use metadata from plugin itself if not in manifest
    if (info.name.isEmpty())    info.name    = plugin->name();
    if (info.version.isEmpty()) info.version = plugin->version();

    PluginContext ctx(nullptr); // workbook attached later
    if (!plugin->initialize(&ctx)) {
        emit pluginError(info.name, "Plugin initialize() returned false");
        loader.unload();
        return false;
    }

    info.instance = plugin;
    info.loaded   = true;
    return true;
}

bool PluginManager::loadPythonPlugin(PluginInfo &info)
{
#ifdef OPENSHEET_PYTHON_SUPPORT
    if (!Py_IsInitialized()) Py_Initialize();

    QString mainPath = info.path + "/main.py";
    FILE *fp = fopen(mainPath.toLocal8Bit().constData(), "r");
    if (!fp) {
        emit pluginError(info.name, "Cannot open main.py");
        return false;
    }
    int ret = PyRun_SimpleFile(fp, mainPath.toLocal8Bit().constData());
    fclose(fp);

    if (ret != 0) {
        emit pluginError(info.name, "Python script execution failed");
        return false;
    }
    info.loaded = true;
    return true;
#else
    Q_UNUSED(info);
    qWarning() << "Python plugin support not compiled in.";
    return false;
#endif
}

void PluginManager::readManifest(PluginInfo &info, const QString &manifestPath)
{
    QFile f(manifestPath);
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError) return;

    auto obj = doc.object();
    info.manifest    = obj;
    info.name        = obj["name"].toString();
    info.version     = obj["version"].toString();
    info.description = obj["description"].toString();
    info.author      = obj["author"].toString();
}

} // namespace OpenSheet
