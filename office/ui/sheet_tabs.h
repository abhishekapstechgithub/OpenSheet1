#pragma once
#include <QWidget>

class QTabBar;
class QToolButton;

namespace OpenSheet {
class Workbook;

class SheetTabs : public QWidget {
    Q_OBJECT
public:
    explicit SheetTabs(QWidget *parent = nullptr);

    void setWorkbook(Workbook *wb);
    void refresh();

signals:
    void sheetSelected(int index);
    void sheetRenamed(int index, const QString &newName);
    void sheetInserted(int index);
    void sheetDeleted(int index);
    void sheetMoved(int from, int to);

private slots:
    void onTabChanged(int index);
    void onAddSheet();
    void onTabDoubleClicked(int index);
    void onTabContextMenu(const QPoint &pos);

private:
    QTabBar     *m_tabBar = nullptr;
    QToolButton *m_addBtn = nullptr;
    Workbook    *m_workbook = nullptr;
    bool         m_updating = false;
};

} // namespace OpenSheet
