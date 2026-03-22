#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QDir>
#include <QStandardPaths>
#include "ui/main_window.h"
#include "core/settings_manager.h"
#include "core/crash_recovery.h"
#include "plugins/plugin_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("OpenSheet");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("OpenSheet Project");
    app.setOrganizationDomain("opensheet.io");

    // Ensure app data directories exist
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs");
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/temp");

    // Crash recovery: check for unsaved sessions
    CrashRecovery crashRecovery;
    crashRecovery.checkAndRestore();

    // Splash screen
    QPixmap splashPixmap(":/resources/icons/splash.png");
    QSplashScreen splash(splashPixmap.isNull()
        ? QPixmap(480, 280)
        : splashPixmap);
    if (splashPixmap.isNull()) {
        splash.setStyleSheet("background:#1a1a2e; color:white;");
    }
    splash.showMessage("Loading OpenSheet...", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
    splash.show();
    app.processEvents();

    // Load settings
    SettingsManager settings;
    settings.load();

    // Apply theme
    QString theme = settings.value("theme", "light").toString();
    QFile styleFile(QString(":/themes/%1/style.qss").arg(theme));
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(styleFile.readAll());
    }

    // Initialize plugin manager
    PluginManager pluginManager;
    splash.showMessage("Loading plugins...", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
    app.processEvents();
    pluginManager.scanAndLoad(QApplication::applicationDirPath() + "/../addons/plugins");

    // Create main window
    MainWindow *window = new MainWindow(&settings, &pluginManager);

    // Open file from command line if provided
    if (argc > 1) {
        window->openFile(QString::fromLocal8Bit(argv[1]));
    } else {
        window->newWorkbook();
    }

    QTimer::singleShot(1800, &splash, &QSplashScreen::close);
    QTimer::singleShot(1800, window, &MainWindow::show);

    return app.exec();
}
