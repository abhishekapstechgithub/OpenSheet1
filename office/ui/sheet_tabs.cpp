#include "sheet_tabs.h"
#include "../engine/workbook.h"
#include <QTabBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

namespace OpenSheet {

SheetTabs::SheetTabs(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(30);
    auto *hl = new QHBoxLayout(this);
    hl->setSpacing(2); hl->setContentsMargins(4,0,4,0);

    m_tabBar = new QTabBar(this);
    m_tabBar->setExpanding(false);
    m_tabBar->setMovable(true);
    m_tabBar->setTabsClosable(false);
    m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_tabBar, &QTabBar::currentChanged, this, &SheetTabs::onTabChanged);
    connect(m_tabBar, &QTabBar::tabBarDoubleClicked, this, &SheetTabs::onTabDoubleClicked);
    connect(m_tabBar, &QTabBar::customContextMenuRequested,
            this,     &SheetTabs::onTabContextMenu);
    connect(m_tabBar, &QTabBar::tabMoved, this, [this](int from, int to){
        emit sheetMoved(from, to);
    });

    m_addBtn = new QToolButton(this);
    m_addBtn->setText("+");
    m_addBtn->setFixedSize(22, 22);
    m_addBtn->setToolTip("Add Sheet");
    connect(m_addBtn, &QToolButton::clicked, this, &SheetTabs::onAddSheet);

    hl->addWidget(m_tabBar, 1);
    hl->addWidget(m_addBtn);
}

void SheetTabs::setWorkbook(Workbook *wb)
{
    m_workbook = wb;
    refresh();
}

void SheetTabs::refresh()
{
    if (!m_workbook || m_updating) return;
    m_updating = true;

    // Remove all tabs
    while (m_tabBar->count()) m_tabBar->removeTab(0);

    // Re-add
    for (int i = 0; i < m_workbook->sheetCount(); ++i)
        m_tabBar->addTab(m_workbook->sheet(i)->name());

    m_tabBar->setCurrentIndex(m_workbook->activeSheetIndex());
    m_updating = false;
}

void SheetTabs::onTabChanged(int index)
{
    if (m_updating) return;
    emit sheetSelected(index);
}

void SheetTabs::onAddSheet()
{
    if (!m_workbook) return;
    m_workbook->addSheet();
    refresh();
    m_tabBar->setCurrentIndex(m_workbook->sheetCount() - 1);
    emit sheetInserted(m_workbook->sheetCount() - 1);
}

void SheetTabs::onTabDoubleClicked(int index)
{
    if (!m_workbook) return;
    Sheet *sheet = m_workbook->sheet(index);
    if (!sheet) return;

    bool ok;
    QString name = QInputDialog::getText(this, "Rename Sheet",
                                          "Sheet name:", QLineEdit::Normal,
                                          sheet->name(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        sheet->setName(name.trimmed());
        m_tabBar->setTabText(index, name.trimmed());
        emit sheetRenamed(index, name.trimmed());
    }
}

void SheetTabs::onTabContextMenu(const QPoint &pos)
{
    int idx = m_tabBar->tabAt(pos);
    if (idx < 0) return;

    QMenu menu(this);
    menu.addAction("Rename…", this, [this, idx]{ onTabDoubleClicked(idx); });
    menu.addSeparator();
    menu.addAction("Insert Sheet Before…", this, [this, idx]{
        if (!m_workbook) return;
        m_workbook->insertSheet(idx);
        refresh();
        emit sheetInserted(idx);
    });
    menu.addAction("Delete Sheet", this, [this, idx]{
        if (!m_workbook || m_workbook->sheetCount() <= 1) {
            QMessageBox::warning(this, "Delete Sheet",
                                 "Cannot delete the only sheet.");
            return;
        }
        auto btn = QMessageBox::question(this, "Delete Sheet",
            "Delete '" + m_workbook->sheet(idx)->name() + "'?");
        if (btn == QMessageBox::Yes) {
            m_workbook->removeSheet(idx);
            refresh();
            emit sheetDeleted(idx);
        }
    });
    menu.addSeparator();
    menu.addAction("Move Left",  this, [this, idx]{
        if (!m_workbook || idx <= 0) return;
        m_workbook->moveSheet(idx, idx - 1);
        refresh(); emit sheetMoved(idx, idx - 1);
    });
    menu.addAction("Move Right", this, [this, idx]{
        if (!m_workbook || idx >= m_workbook->sheetCount()-1) return;
        m_workbook->moveSheet(idx, idx + 1);
        refresh(); emit sheetMoved(idx, idx + 1);
    });
    menu.exec(m_tabBar->mapToGlobal(pos));
}

} // namespace OpenSheet
