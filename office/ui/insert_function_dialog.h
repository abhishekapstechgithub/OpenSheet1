#pragma once
#include <QDialog>

class QListWidget;
class QLabel;
class QLineEdit;
class QComboBox;
class QTextEdit;

namespace OpenSheet {

class FormulaRegistry;

/**
 * "Insert Function" dialog — browse and search the formula catalog,
 * select a function, and see its syntax and description.
 */
class InsertFunctionDialog : public QDialog {
    Q_OBJECT
public:
    explicit InsertFunctionDialog(QWidget *parent = nullptr);

    void setRegistry(FormulaRegistry *registry);

    // Returns the selected function name (empty if cancelled)
    QString selectedFunction() const { return m_selected; }

    // Returns a skeleton formula ready to paste into the formula bar
    QString formulaSkeleton() const;

private slots:
    void onCategoryChanged(int idx);
    void onFunctionSelected(int idx);
    void onSearchChanged(const QString &query);
    void onInsert();

private:
    void populate(const QStringList &names);

    FormulaRegistry *m_registry = nullptr;
    QString          m_selected;

    QComboBox  *m_categoryCombo  = nullptr;
    QLineEdit  *m_searchEdit     = nullptr;
    QListWidget *m_functionList  = nullptr;
    QLabel      *m_syntaxLabel   = nullptr;
    QTextEdit   *m_descEdit      = nullptr;
    QTextEdit   *m_argEdit       = nullptr;
};

} // namespace OpenSheet
