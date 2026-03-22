#pragma once
#include <QMainWindow>
#include <QUndoStack>
#include <QString>

class QTabWidget;
class QLabel;
class QTimer;

namespace OpenSheet {
class Workbook;
class Sheet;
class SettingsManager;
class PluginManager;
class SpreadsheetView;
class RibbonBar;
class FormulaBar;
class SheetTabs;
class StatusBar;
class SidePanel;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(OpenSheet::SettingsManager *settings,
                        OpenSheet::PluginManager   *plugins,
                        QWidget *parent = nullptr);
    ~MainWindow();

    void newWorkbook();
    void openFile(const QString &path);

public slots:
    void onNewFile();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onClose();
    void onPrint();
    void onUndo();
    void onRedo();
    void onCut();
    void onCopy();
    void onPaste();
    void onFind();
    void onFindReplace();
    void onInsertChart();
    void onInsertPivot();
    void onToggleDarkMode();
    void onAbout();
    void onSheetChanged(int index);
    void onCellSelected(int row, int col);
    void onFormulaEdited(const QString &text);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupMenuBar();
    void setupRibbon();
    void setupCentralArea();
    void setupStatusBar();
    void setupSidePanel();
    void setupShortcuts();
    void setupAutoSave();
    void applyTheme(const QString &theme);
    void updateWindowTitle();
    bool confirmClose();
    void saveSettings();
    void loadSettings();
    void recentFilesMenu();

    // Workbook management
    OpenSheet::Workbook        *m_workbook    = nullptr;
    OpenSheet::SettingsManager *m_settings;
    OpenSheet::PluginManager   *m_plugins;

    // UI components
    OpenSheet::RibbonBar       *m_ribbon      = nullptr;
    OpenSheet::FormulaBar      *m_formulaBar  = nullptr;
    OpenSheet::SpreadsheetView *m_sheetView   = nullptr;
    OpenSheet::SheetTabs       *m_sheetTabs   = nullptr;
    OpenSheet::StatusBar       *m_statusBar   = nullptr;
    OpenSheet::SidePanel       *m_sidePanel   = nullptr;

    // Auto-save
    QTimer  *m_autoSaveTimer  = nullptr;
    int      m_autoSaveIntervalMs = 60000; // 1 min

    QString  m_currentTheme = "light";
    QStringList m_recentFiles;

private slots:
    void doAutoSave();
    void onUndoAvailable(bool available);
    void onRedoAvailable(bool available);
};
