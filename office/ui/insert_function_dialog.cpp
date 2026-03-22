#include "insert_function_dialog.h"
#include "../formulas/formula_registry.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSplitter>
#include <QGroupBox>
#include <QTimer>

namespace OpenSheet {

InsertFunctionDialog::InsertFunctionDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Insert Function");
    setMinimumSize(520, 480);

    auto *vl = new QVBoxLayout(this);

    // Search bar
    auto *searchRow = new QHBoxLayout;
    searchRow->addWidget(new QLabel("Search:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Type function name or keyword…");
    m_searchEdit->setClearButtonEnabled(true);
    searchRow->addWidget(m_searchEdit, 1);
    vl->addLayout(searchRow);

    // Category filter
    auto *catRow = new QHBoxLayout;
    catRow->addWidget(new QLabel("Category:", this));
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem("All");
    catRow->addWidget(m_categoryCombo, 1);
    vl->addLayout(catRow);

    // Splitter: function list | description
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    // Left: function list
    m_functionList = new QListWidget(splitter);
    splitter->addWidget(m_functionList);

    // Right: description panel
    auto *descWidget = new QWidget(splitter);
    auto *dvl = new QVBoxLayout(descWidget);
    dvl->setContentsMargins(8,0,0,0);

    m_syntaxLabel = new QLabel(this);
    m_syntaxLabel->setWordWrap(true);
    m_syntaxLabel->setStyleSheet(
        "font-family: 'Courier New', monospace; font-size: 12px; "
        "background: #F0EFE9; padding: 6px; border-radius: 4px;");
    dvl->addWidget(m_syntaxLabel);

    dvl->addWidget(new QLabel("Description:", descWidget));
    m_descEdit = new QTextEdit(descWidget);
    m_descEdit->setReadOnly(true);
    m_descEdit->setMaximumHeight(100);
    m_descEdit->setStyleSheet("font-size: 12px;");
    dvl->addWidget(m_descEdit);

    dvl->addWidget(new QLabel("Arguments:", descWidget));
    m_argEdit = new QTextEdit(descWidget);
    m_argEdit->setReadOnly(true);
    m_argEdit->setStyleSheet("font-size: 11px; color: #5F5E5A;");
    dvl->addWidget(m_argEdit, 1);

    splitter->addWidget(descWidget);
    splitter->setSizes({200, 300});
    vl->addWidget(splitter, 1);

    // Buttons
    auto *btns = new QDialogButtonBox(this);
    auto *insertBtn = btns->addButton("Insert", QDialogButtonBox::AcceptRole);
    btns->addButton(QDialogButtonBox::Cancel);
    insertBtn->setDefault(true);
    connect(btns, &QDialogButtonBox::accepted, this, &InsertFunctionDialog::onInsert);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    vl->addWidget(btns);

    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InsertFunctionDialog::onCategoryChanged);
    connect(m_functionList,  &QListWidget::currentRowChanged,
            this, &InsertFunctionDialog::onFunctionSelected);
    connect(m_searchEdit,    &QLineEdit::textChanged,
            this, &InsertFunctionDialog::onSearchChanged);
}

void InsertFunctionDialog::setRegistry(FormulaRegistry *reg)
{
    m_registry = reg;
    if (!reg) return;

    // Populate categories
    m_categoryCombo->clear();
    m_categoryCombo->addItem("All");
    for (const QString &cat : reg->categories())
        m_categoryCombo->addItem(cat);

    // Show all functions
    onCategoryChanged(0);
}

void InsertFunctionDialog::onCategoryChanged(int)
{
    if (!m_registry) return;
    QString cat = m_categoryCombo->currentText();
    QStringList names;
    if (cat == "All")
        for (const auto &f : m_registry->allFunctions()) names.append(f.name);
    else
        for (const auto &f : m_registry->byCategory(cat)) names.append(f.name);
    names.sort();
    populate(names);
}

void InsertFunctionDialog::onSearchChanged(const QString &q)
{
    if (!m_registry) return;
    if (q.trimmed().isEmpty()) { onCategoryChanged(0); return; }
    QStringList names;
    for (const auto &f : m_registry->search(q)) names.append(f.name);
    names.sort();
    populate(names);
}

void InsertFunctionDialog::populate(const QStringList &names)
{
    m_functionList->clear();
    for (const auto &n : names) m_functionList->addItem(n);
    if (m_functionList->count() > 0) m_functionList->setCurrentRow(0);
}

void InsertFunctionDialog::onFunctionSelected(int idx)
{
    if (!m_registry || idx < 0) return;
    QString name = m_functionList->item(idx)->text();
    const auto *meta = m_registry->find(name);
    if (!meta) return;

    m_syntaxLabel->setText(meta->syntax);
    m_descEdit->setText(meta->description);

    QString args;
    for (int i = 0; i < meta->argNames.size(); ++i) {
        args += QString("• <b>%1</b>: %2\n")
                    .arg(meta->argNames[i])
                    .arg(i < meta->argDescriptions.size()
                             ? meta->argDescriptions[i] : "");
    }
    m_argEdit->setHtml(args.isEmpty() ? "<i>No arguments</i>" : args);
}

void InsertFunctionDialog::onInsert()
{
    int idx = m_functionList->currentRow();
    if (idx >= 0)
        m_selected = m_functionList->item(idx)->text();
    accept();
}

QString InsertFunctionDialog::formulaSkeleton() const
{
    if (m_selected.isEmpty() || !m_registry) return {};
    const auto *meta = m_registry->find(m_selected);
    if (!meta || meta->argNames.isEmpty())
        return "=" + m_selected + "()";
    // Build skeleton: =SUM(number1, [number2])
    return "=" + meta->syntax;
}

} // namespace OpenSheet
