#pragma once
#include <QWidget>
#include <QString>
#include <QVector>

class QTabBar;
class QStackedWidget;
class QToolButton;
class QComboBox;

namespace OpenSheet {

/**
 * Microsoft Office-style ribbon toolbar.
 * Tabs: Home | Insert | Page Layout | Formulas | Data | Review | View
 * Each tab has groups of buttons/combos.
 */
class RibbonBar : public QWidget {
    Q_OBJECT
public:
    explicit RibbonBar(QWidget *parent = nullptr);

    // Enable/disable specific ribbon buttons
    void setActionEnabled(const QString &id, bool enabled);
    void setActionChecked(const QString &id, bool checked);

    // Update font/size selectors to reflect current cell
    void setFontName(const QString &name);
    void setFontSize(int size);
    void setBold(bool bold);
    void setItalic(bool italic);
    void setUnderline(bool underline);

signals:
    void actionTriggered(const QString &actionId);
    void fontNameChanged(const QString &name);
    void fontSizeChanged(int size);
    void numberFormatChanged(const QString &format);
    void alignmentChanged(Qt::Alignment alignment);
    void tabChanged(int index);

private:
    void buildHomeTab(QWidget *tab);
    void buildInsertTab(QWidget *tab);
    void buildPageLayoutTab(QWidget *tab);
    void buildFormulasTab(QWidget *tab);
    void buildDataTab(QWidget *tab);
    void buildReviewTab(QWidget *tab);
    void buildViewTab(QWidget *tab);

    QToolButton *ribbonButton(const QString &icon, const QString &label,
                               const QString &actionId, bool tall = false);
    QWidget     *ribbonGroup(const QString &title, QWidget *parent);

    QTabBar        *m_tabBar    = nullptr;
    QStackedWidget *m_pages     = nullptr;
    QComboBox      *m_fontCombo = nullptr;
    QComboBox      *m_sizeCombo = nullptr;
    QComboBox      *m_fmtCombo  = nullptr;
};

} // namespace OpenSheet
