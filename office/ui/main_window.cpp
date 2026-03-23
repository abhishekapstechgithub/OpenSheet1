#include "main_window.h"
#include "ribbon_bar.h"
#include "formula_bar.h"
#include "spreadsheet_view.h"
#include "sheet_tabs.h"
#include "status_bar.h"
#include "side_panel.h"
#include "../engine/workbook.h"
#include "../core/settings_manager.h"
#include "../core/file_manager.h"
#include "../../plugins/plugin_manager.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTimer>
#include <QKeySequence>
#include <QShortcut>
#include <QSettings>
#include <QApplication>
#include <QScreen>
#include <QStatusBar>
#include <QToolBar>

using namespace OpenSheet;

MainWindow::MainWindow(SettingsManager *settings, PluginManager *plugins, QWidget *parent)
    : QMainWindow(parent)
    , m_settings(settings)
    , m_plugins(plugins)
{
    setAcceptDrops(true);
    setMinimumSize(1024, 640);
    resize(1440, 900);

    setupMenuBar();
    setupRibbon();
    setupCentralArea();
    setupSidePanel();
    setupStatusBar();
    setupShortcuts();
    setupAutoSave();
    loadSettings();
    updateWindowTitle();

    // Center on screen
    QScreen *sc = QApplication::primaryScreen();
    if (sc) move(sc->geometry().center() - rect().center());
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::newWorkbook()
{
    delete m_workbook;
    m_workbook = new Workbook(this);
    m_workbook->addSheet("Sheet1");
    m_workbook->addSheet("Sheet2");
    m_workbook->addSheet("Sheet3");

    m_sheetView->setWorkbook(m_workbook);
    m_sheetTabs->setWorkbook(m_workbook);
    m_formulaBar->setWorkbook(m_workbook);
    m_sidePanel->setWorkbook(m_workbook);

    connect(m_workbook, &Workbook::modifiedChanged, this, [this](bool){ updateWindowTitle(); });
    updateWindowTitle();
}

void MainWindow::openFile(const QString &path)
{
    FileManager fm;
    Workbook *wb = fm.open(path);
    if (!wb) {
        QMessageBox::critical(this, "Open Failed", fm.lastError());
        return;
    }
    delete m_workbook;
    m_workbook = wb;
    m_workbook->setParent(this);
    m_workbook->setFilePath(path);

    m_sheetView->setWorkbook(m_workbook);
    m_sheetTabs->setWorkbook(m_workbook);
    m_formulaBar->setWorkbook(m_workbook);
    m_sidePanel->setWorkbook(m_workbook);

    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    if (m_recentFiles.size() > 10) m_recentFiles.removeLast();

    updateWindowTitle();
}

// --- Slots ---

void MainWindow::onNewFile()
{
    if (!confirmClose()) return;
    newWorkbook();
}

void MainWindow::onOpen()
{
    if (!confirmClose()) return;
    QString path = QFileDialog::getOpenFileName(this, "Open File", {},
        "Spreadsheet Files (*.opensheet *.xlsx *.csv);;All Files (*)");
    if (!path.isEmpty()) openFile(path);
}

void MainWindow::onSave()
{
    if (!m_workbook) return;
    if (m_workbook->filePath().isEmpty()) { onSaveAs(); return; }
    FileManager fm;
    if (!fm.save(m_workbook, m_workbook->filePath()))
        QMessageBox::critical(this, "Save Failed", fm.lastError());
    else
        m_workbook->setModified(false);
}

void MainWindow::onSaveAs()
{
    if (!m_workbook) return;
    QString path = QFileDialog::getSaveFileName(this, "Save As", {},
        "OpenSheet (*.opensheet);;Excel (*.xlsx);;CSV (*.csv)");
    if (path.isEmpty()) return;
    m_workbook->setFilePath(path);
    onSave();
    updateWindowTitle();
}

void MainWindow::onClose()  { close(); }
void MainWindow::onPrint()  { /* TODO: QPrinter integration */ }
void MainWindow::onUndo()   { if (m_workbook) m_workbook->undoStack()->undo(); }
void MainWindow::onRedo()   { if (m_workbook) m_workbook->undoStack()->redo(); }
void MainWindow::onCut()    { if (m_sheetView) m_sheetView->cut(); }
void MainWindow::onCopy()   { if (m_sheetView) m_sheetView->copy(); }
void MainWindow::onPaste()  { if (m_sheetView) m_sheetView->paste(); }
void MainWindow::onFind()   { if (m_sheetView) m_sheetView->showFindDialog(); }
void MainWindow::onFindReplace() { if (m_sheetView) m_sheetView->showFindReplaceDialog(); }
void MainWindow::onInsertChart() { if (m_sheetView) m_sheetView->insertChart(); }
void MainWindow::onInsertPivot() { if (m_sheetView) m_sheetView->insertPivotTable(); }

void MainWindow::onToggleDarkMode()
{
    m_currentTheme = (m_currentTheme == "light") ? "dark" : "light";
    applyTheme(m_currentTheme);
    m_settings->setValue("theme", m_currentTheme);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About OpenSheet",
        "<h3>OpenSheet 1.0</h3>"
        "<p>A professional, open-source spreadsheet application.<br>"
        "Built with C++20 and Qt6.</p>"
        "<p>&copy; 2024 OpenSheet Project</p>");
}

void MainWindow::onSheetChanged(int index)
{
    if (!m_workbook) return;
    m_workbook->setActiveSheet(index);
    m_sheetView->setSheet(m_workbook->activeSheet());
    m_formulaBar->setSheet(m_workbook->activeSheet());
}

void MainWindow::onCellSelected(int row, int col)
{
    if (m_statusBar) m_statusBar->updateSelection(row, col);
    if (m_sidePanel) m_sidePanel->updateCellInfo(row, col);
}

void MainWindow::onFormulaEdited(const QString &text)
{
    if (m_sheetView) m_sheetView->setCurrentCellRaw(text);
}

// --- Protected ---

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmClose()) { saveSettings(); event->accept(); }
    else                  event->ignore();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) openFile(urls.first().toLocalFile());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QMainWindow::keyPressEvent(event);
}

// --- Private helpers ---

void MainWindow::setupMenuBar()
{
    auto *mb = menuBar();

    // File
    auto *fileMenu = mb->addMenu("&File");
    fileMenu->addAction("&New",          this, &MainWindow::onNewFile,  QKeySequence::New);
    fileMenu->addAction("&Open...",      this, &MainWindow::onOpen,     QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Save",         this, &MainWindow::onSave,     QKeySequence::Save);
    fileMenu->addAction("Save &As...",   this, &MainWindow::onSaveAs,   QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("&Print...",     this, &MainWindow::onPrint,    QKeySequence::Print);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit",         this, &MainWindow::onClose,    QKeySequence::Quit);

    // Edit
    auto *editMenu = mb->addMenu("&Edit");
    auto *undoAct = editMenu->addAction("&Undo", this, &MainWindow::onUndo, QKeySequence::Undo);
    auto *redoAct = editMenu->addAction("&Redo", this, &MainWindow::onRedo, QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction("Cu&t",      this, &MainWindow::onCut,   QKeySequence::Cut);
    editMenu->addAction("&Copy",     this, &MainWindow::onCopy,  QKeySequence::Copy);
    editMenu->addAction("&Paste",    this, &MainWindow::onPaste, QKeySequence::Paste);
    editMenu->addSeparator();
    editMenu->addAction("&Find...",         this, &MainWindow::onFind,        QKeySequence::Find);
    editMenu->addAction("Find && &Replace", this, &MainWindow::onFindReplace, QKeySequence::Replace);
    Q_UNUSED(undoAct); Q_UNUSED(redoAct);

    // Insert
    auto *insertMenu = mb->addMenu("&Insert");
    insertMenu->addAction("&Chart...",       this, &MainWindow::onInsertChart);
    insertMenu->addAction("&Pivot Table...", this, &MainWindow::onInsertPivot);

    // View
    auto *viewMenu = mb->addMenu("&View");
    viewMenu->addAction("Toggle &Dark Mode", this, &MainWindow::onToggleDarkMode);

    // Help
    auto *helpMenu = mb->addMenu("&Help");
    helpMenu->addAction("&About OpenSheet", this, &MainWindow::onAbout);
}

void MainWindow::setupRibbon()
{
    m_ribbon = new RibbonBar(this);
    connect(m_ribbon, &RibbonBar::actionTriggered, this, [this](const QString &id){
        if      (id=="new")    onNewFile();
        else if (id=="open")   onOpen();
        else if (id=="save")   onSave();
        else if (id=="undo")   onUndo();
        else if (id=="redo")   onRedo();
        else if (id=="cut")    onCut();
        else if (id=="copy")   onCopy();
        else if (id=="paste")  onPaste();
        else if (id=="chart")  onInsertChart();
        else if (id=="dark")   onToggleDarkMode();
    });

    m_formulaBar = new FormulaBar(this);
    connect(m_formulaBar, &FormulaBar::formulaEdited, this, &MainWindow::onFormulaEdited);

    auto *top = new QWidget(this);
    auto *layout = new QVBoxLayout(top);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_ribbon);
    layout->addWidget(m_formulaBar);

    addToolBar(Qt::TopToolBarArea, new QToolBar()); // placeholder
    // Real ribbon is placed via dock
    auto *dock = new QDockWidget(this);
    dock->setTitleBarWidget(new QWidget(dock));
    dock->setWidget(top);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::TopDockWidgetArea, dock);
}

void MainWindow::setupCentralArea()
{
    m_sheetView = new SpreadsheetView(this);
    m_sheetTabs = new SheetTabs(this);

    connect(m_sheetTabs, &SheetTabs::sheetSelected,  this, &MainWindow::onSheetChanged);
    connect(m_sheetView, &SpreadsheetView::cellSelected, this, &MainWindow::onCellSelected);

    auto *container = new QWidget(this);
    auto *vl = new QVBoxLayout(container);
    vl->setSpacing(0); vl->setContentsMargins(0,0,0,0);
    vl->addWidget(m_sheetView, 1);
    vl->addWidget(m_sheetTabs);
    setCentralWidget(container);
}

void MainWindow::setupSidePanel()
{
    m_sidePanel = new SidePanel(this);
    auto *dock = new QDockWidget("Properties", this);
    dock->setWidget(m_sidePanel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = new OpenSheet::StatusBar(this);
    setStatusBar(m_statusBar);
}

void MainWindow::setupShortcuts()
{
    new QShortcut(QKeySequence("Ctrl+B"), this, [this]{ if(m_sheetView) m_sheetView->toggleBold(); });
    new QShortcut(QKeySequence("Ctrl+I"), this, [this]{ if(m_sheetView) m_sheetView->toggleItalic(); });
    new QShortcut(QKeySequence("Ctrl+U"), this, [this]{ if(m_sheetView) m_sheetView->toggleUnderline(); });
    new QShortcut(QKeySequence("Delete"), this, [this]{ if(m_sheetView) m_sheetView->deleteSelection(); });
    new QShortcut(QKeySequence("F2"),     this, [this]{ if(m_sheetView) m_sheetView->editCurrentCell(); });
    new QShortcut(QKeySequence("F5"),     this, [this]{ if(m_sheetView) m_sheetView->showGoToDialog(); });
}

void MainWindow::setupAutoSave()
{
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::doAutoSave);
    m_autoSaveTimer->start(m_autoSaveIntervalMs);
}

void MainWindow::applyTheme(const QString &theme)
{
    QFile f(QString(":/themes/%1/style.qss").arg(theme));
    if (f.open(QFile::ReadOnly)) qApp->setStyleSheet(f.readAll());
}

void MainWindow::updateWindowTitle()
{
    QString title = "OpenSheet";
    if (m_workbook) {
        QString fp = m_workbook->filePath();
        title = (fp.isEmpty() ? "Untitled" : QFileInfo(fp).fileName());
        if (m_workbook->isModified()) title += " *";
        title += " — OpenSheet";
    }
    setWindowTitle(title);
}

bool MainWindow::confirmClose()
{
    if (!m_workbook || !m_workbook->isModified()) return true;
    auto btn = QMessageBox::question(this, "Unsaved Changes",
        "The workbook has unsaved changes. Save before closing?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if      (btn == QMessageBox::Save)    { onSave(); return true; }
    else if (btn == QMessageBox::Discard) return true;
    return false;
}

void MainWindow::saveSettings()
{
    QSettings s;
    s.setValue("geometry",    saveGeometry());
    s.setValue("windowState", saveState());
    s.setValue("theme",       m_currentTheme);
    s.setValue("recentFiles", m_recentFiles);
}

void MainWindow::loadSettings()
{
    QSettings s;
    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("windowState").toByteArray());
    m_currentTheme = s.value("theme", "light").toString();
    m_recentFiles  = s.value("recentFiles").toStringList();
}

void MainWindow::doAutoSave()
{
    if (!m_workbook || !m_workbook->isModified()) return;
    QString tmp = m_workbook->autoSavePath();
    if (tmp.isEmpty()) return;
    FileManager fm;
    fm.save(m_workbook, tmp);
}

void MainWindow::onUndoAvailable(bool) {}
void MainWindow::onRedoAvailable(bool) {}
void MainWindow::recentFilesMenu()     {}
