#include "cell_format_dialog.h"
#include "../engine/number_formatter.h"
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QFontComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFrame>

namespace OpenSheet {

// ── Simple color picker button ─────────────────────────────────────────────
class ColorButton : public QPushButton {
    Q_OBJECT
public:
    explicit ColorButton(const QColor &c = Qt::white, QWidget *parent = nullptr)
        : QPushButton(parent), m_color(c)
    {
        setFixedSize(32, 22);
        updateSwatch();
        connect(this, &QPushButton::clicked, this, [this]{
            QColor nc = QColorDialog::getColor(m_color, this, "Choose Color");
            if (nc.isValid()) { m_color = nc; updateSwatch(); emit colorChanged(nc); }
        });
    }

    QColor color() const { return m_color; }
    void   setColor(const QColor &c) { m_color = c; updateSwatch(); }

signals:
    void colorChanged(const QColor &c);

private:
    void updateSwatch() {
        setStyleSheet(QString("background:%1; border:1px solid #C8C7C0;")
                      .arg(m_color.name()));
    }
    QColor m_color;
};

#include "cell_format_dialog.moc"

// ─────────────────────────────────────────────────────────────────────────────

CellFormatDialog::CellFormatDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Format Cells");
    setMinimumSize(500, 420);

    auto *vl = new QVBoxLayout(this);
    auto *tabs = new QTabWidget(this);

    auto makeTab = [&](const QString &title, auto buildFn) {
        auto *w = new QWidget(tabs);
        buildFn(w);
        tabs->addTab(w, title);
    };

    makeTab("Number",    [this](QWidget *w){ buildNumberTab(w);    });
    makeTab("Alignment", [this](QWidget *w){ buildAlignmentTab(w); });
    makeTab("Font",      [this](QWidget *w){ buildFontTab(w);      });
    makeTab("Border",    [this](QWidget *w){ buildBorderTab(w);    });
    makeTab("Fill",      [this](QWidget *w){ buildFillTab(w);      });

    vl->addWidget(tabs, 1);

    auto *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    vl->addWidget(btns);
}

void CellFormatDialog::buildNumberTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);

    // Category list
    m_categoryList = new QListWidget(tab);
    m_categoryList->setMaximumWidth(140);
    for (const QString &cat : {"General","Number","Currency","Accounting",
                                "Date","Time","Percentage","Fraction",
                                "Scientific","Text","Special","Custom"})
        m_categoryList->addItem(cat);
    hl->addWidget(m_categoryList);

    // Right panel
    auto *rvl = new QVBoxLayout;

    // Format list
    m_formatList = new QListWidget(tab);
    m_formatList->setMaximumHeight(120);
    rvl->addWidget(new QLabel("Format:", tab));
    rvl->addWidget(m_formatList);

    // Code edit
    auto *codeRow = new QHBoxLayout;
    codeRow->addWidget(new QLabel("Code:", tab));
    m_codeEdit = new QLineEdit(tab);
    codeRow->addWidget(m_codeEdit, 1);
    rvl->addLayout(codeRow);

    // Decimals
    auto *decRow = new QHBoxLayout;
    decRow->addWidget(new QLabel("Decimal places:", tab));
    m_decimalsSpin = new QSpinBox(tab);
    m_decimalsSpin->setRange(0, 10);
    m_decimalsSpin->setValue(2);
    decRow->addWidget(m_decimalsSpin);
    decRow->addStretch();
    rvl->addLayout(decRow);

    // Preview
    rvl->addWidget(new QLabel("Preview:", tab));
    m_preview = new QLabel("1,234.56", tab);
    m_preview->setStyleSheet("border:1px solid #D3D1C7; padding:4px 8px; "
                             "background:#FAFAF8; min-height:28px;");
    rvl->addWidget(m_preview);
    rvl->addStretch();

    hl->addLayout(rvl, 1);

    connect(m_categoryList, &QListWidget::currentRowChanged,
            this, &CellFormatDialog::onCategoryChanged);
    connect(m_codeEdit, &QLineEdit::textChanged,
            this, &CellFormatDialog::onFormatCodeChanged);
}

void CellFormatDialog::buildAlignmentTab(QWidget *tab)
{
    auto *gl = new QGridLayout(tab);
    gl->setSpacing(10);

    gl->addWidget(new QLabel("Horizontal:", tab), 0, 0);
    m_hAlignCombo = new QComboBox(tab);
    m_hAlignCombo->addItems({"General","Left","Center","Right","Fill","Justify"});
    gl->addWidget(m_hAlignCombo, 0, 1);

    gl->addWidget(new QLabel("Vertical:", tab), 1, 0);
    m_vAlignCombo = new QComboBox(tab);
    m_vAlignCombo->addItems({"Top","Center","Bottom"});
    gl->addWidget(m_vAlignCombo, 1, 1);

    gl->addWidget(new QLabel("Indent:", tab), 2, 0);
    m_indentSpin = new QSpinBox(tab);
    m_indentSpin->setRange(0, 15);
    gl->addWidget(m_indentSpin, 2, 1);

    m_wrapCheck  = new QCheckBox("Wrap text", tab);
    m_mergeCheck = new QCheckBox("Merge cells", tab);
    gl->addWidget(m_wrapCheck,  3, 0, 1, 2);
    gl->addWidget(m_mergeCheck, 4, 0, 1, 2);
    gl->setRowStretch(5, 1);
}

void CellFormatDialog::buildFontTab(QWidget *tab)
{
    auto *gl = new QGridLayout(tab);
    gl->setSpacing(8);

    gl->addWidget(new QLabel("Font:", tab), 0, 0);
    m_fontCombo = new QFontComboBox(tab);
    gl->addWidget(m_fontCombo, 0, 1);

    gl->addWidget(new QLabel("Size:", tab), 1, 0);
    m_sizeSpin = new QSpinBox(tab);
    m_sizeSpin->setRange(6, 72);
    m_sizeSpin->setValue(11);
    gl->addWidget(m_sizeSpin, 1, 1);

    m_boldCheck   = new QCheckBox("Bold",          tab);
    m_italicCheck = new QCheckBox("Italic",         tab);
    m_underCheck  = new QCheckBox("Underline",      tab);
    m_strikeCheck = new QCheckBox("Strikethrough",  tab);
    gl->addWidget(m_boldCheck,   2, 0);
    gl->addWidget(m_italicCheck, 2, 1);
    gl->addWidget(m_underCheck,  3, 0);
    gl->addWidget(m_strikeCheck, 3, 1);
    gl->setRowStretch(4, 1);
}

void CellFormatDialog::buildBorderTab(QWidget *tab)
{
    auto *vl = new QVBoxLayout(tab);
    vl->addWidget(new QLabel(
        "Border presets and custom border styling\n"
        "(full border painter widget — implementation pending)", tab));
    vl->addStretch();
}

void CellFormatDialog::buildFillTab(QWidget *tab)
{
    auto *gl = new QGridLayout(tab);
    gl->setSpacing(8);

    gl->addWidget(new QLabel("Background color:", tab), 0, 0);
    m_bgColorBtn = new ColorButton(Qt::white, tab);
    gl->addWidget(m_bgColorBtn, 0, 1);

    gl->addWidget(new QLabel("Text color:", tab), 1, 0);
    m_fgColorBtn = new ColorButton(Qt::black, tab);
    gl->addWidget(m_fgColorBtn, 1, 1);
    gl->setRowStretch(2, 1);
}

void CellFormatDialog::onCategoryChanged(int idx)
{
    m_formatList->clear();
    QStringList presets;
    switch (idx) {
    case 1: presets = {"0","0.00","#,##0","#,##0.00"}; break;
    case 2: presets = {"$#,##0","$#,##0.00","€#,##0","£#,##0"}; break;
    case 4: presets = {"dd/MM/yyyy","MM/dd/yyyy","yyyy-MM-dd","dd MMM yyyy"}; break;
    case 5: presets = {"hh:mm","hh:mm:ss","h:mm AM/PM"}; break;
    case 6: presets = {"0%","0.00%"}; break;
    case 8: presets = {"0.00E+00","##0.0E+0"}; break;
    default: break;
    }
    for (const auto &p : presets) m_formatList->addItem(p);
    if (!presets.isEmpty()) m_formatList->setCurrentRow(0);
    connect(m_formatList, &QListWidget::currentTextChanged,
            m_codeEdit,   &QLineEdit::setText);
}

void CellFormatDialog::onFormatCodeChanged(const QString &code)
{
    QString preview = NumberFormatter::format(QVariant(1234.567), code);
    m_preview->setText(preview.isEmpty() ? "1234.567" : preview);
}

void CellFormatDialog::onPreviewUpdate() { onFormatCodeChanged(m_codeEdit->text()); }

void CellFormatDialog::setFormat(const CellFormat &fmt)
{
    m_format = fmt;
    if (m_boldCheck)      m_boldCheck->setChecked(fmt.bold);
    if (m_italicCheck)    m_italicCheck->setChecked(fmt.italic);
    if (m_underCheck)     m_underCheck->setChecked(fmt.underline);
    if (m_strikeCheck)    m_strikeCheck->setChecked(fmt.strikethrough);
    if (m_wrapCheck)      m_wrapCheck->setChecked(fmt.wrapText);
    if (m_fgColorBtn)     m_fgColorBtn->setColor(fmt.foreground);
    if (m_bgColorBtn)     m_bgColorBtn->setColor(fmt.background);
    if (m_codeEdit)       m_codeEdit->setText(fmt.numberFormat);
    if (m_sizeSpin)       m_sizeSpin->setValue(fmt.font.pointSize() > 0 ? fmt.font.pointSize() : 11);
    if (m_fontCombo)      m_fontCombo->setCurrentFont(fmt.font);
}

CellFormat CellFormatDialog::format() const
{
    CellFormat f = m_format;
    if (m_boldCheck)   f.bold        = m_boldCheck->isChecked();
    if (m_italicCheck) f.italic      = m_italicCheck->isChecked();
    if (m_underCheck)  f.underline   = m_underCheck->isChecked();
    if (m_strikeCheck) f.strikethrough = m_strikeCheck->isChecked();
    if (m_wrapCheck)   f.wrapText    = m_wrapCheck->isChecked();
    if (m_fgColorBtn)  f.foreground  = m_fgColorBtn->color();
    if (m_bgColorBtn)  f.background  = m_bgColorBtn->color();
    if (m_codeEdit)    f.numberFormat = m_codeEdit->text();
    if (m_fontCombo)   f.font        = m_fontCombo->currentFont();
    if (m_sizeSpin)    f.font.setPointSize(m_sizeSpin->value());

    if (m_hAlignCombo) {
        static const Qt::Alignment haligns[] = {
            Qt::AlignLeft, Qt::AlignLeft, Qt::AlignHCenter,
            Qt::AlignRight, Qt::AlignJustify, Qt::AlignJustify
        };
        int i = m_hAlignCombo->currentIndex();
        if (i >= 0 && i < 6) f.alignment = haligns[i] | Qt::AlignVCenter;
    }
    return f;
}

} // namespace OpenSheet
