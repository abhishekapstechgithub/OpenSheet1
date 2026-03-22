#include "ribbon_bar.h"
#include <QTabBar>
#include <QStackedWidget>
#include <QToolButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QFontDatabase>

namespace OpenSheet {

RibbonBar::RibbonBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(80);
    auto *vl = new QVBoxLayout(this);
    vl->setSpacing(0);
    vl->setContentsMargins(0,0,0,0);

    // Tab bar
    m_tabBar = new QTabBar(this);
    m_tabBar->setExpanding(false);
    m_tabBar->setDrawBase(false);
    for (auto &t : {"Home","Insert","Page Layout","Formulas","Data","Review","View"})
        m_tabBar->addTab(t);
    connect(m_tabBar, &QTabBar::currentChanged, this, [this](int i){
        m_pages->setCurrentIndex(i);
        emit tabChanged(i);
    });

    // Pages
    m_pages = new QStackedWidget(this);

    auto makeTab = [&](auto buildFn) -> QWidget* {
        auto *w = new QWidget(m_pages);
        buildFn(w);
        m_pages->addWidget(w);
        return w;
    };

    makeTab([this](QWidget *w){ buildHomeTab(w); });
    makeTab([this](QWidget *w){ buildInsertTab(w); });
    makeTab([this](QWidget *w){ buildPageLayoutTab(w); });
    makeTab([this](QWidget *w){ buildFormulasTab(w); });
    makeTab([this](QWidget *w){ buildDataTab(w); });
    makeTab([this](QWidget *w){ buildReviewTab(w); });
    makeTab([this](QWidget *w){ buildViewTab(w); });

    vl->addWidget(m_tabBar);
    vl->addWidget(m_pages, 1);
}

QToolButton *RibbonBar::ribbonButton(const QString &icon, const QString &label,
                                      const QString &actionId, bool tall)
{
    auto *btn = new QToolButton(this);
    btn->setText(tall ? icon + "\n" + label : icon);
    btn->setToolTip(label);
    btn->setToolButtonStyle(tall ? Qt::ToolButtonTextUnderIcon
                                 : Qt::ToolButtonTextBesideIcon);
    if (tall) btn->setFixedHeight(52);
    else      btn->setFixedHeight(24);
    connect(btn, &QToolButton::clicked, this, [this, actionId]{
        emit actionTriggered(actionId);
    });
    return btn;
}

QWidget *RibbonBar::ribbonGroup(const QString &title, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    auto *vl = new QVBoxLayout(frame);
    vl->setSpacing(2); vl->setContentsMargins(4,2,4,2);
    auto *row = new QHBoxLayout;
    vl->addLayout(row, 1);
    auto *lbl = new QLabel(title, frame);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("font-size: 10px; color: #888780;");
    vl->addWidget(lbl);
    frame->setProperty("groupLayout", QVariant::fromValue(row));
    return frame;
}

void RibbonBar::buildHomeTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);
    hl->setSpacing(2); hl->setContentsMargins(4,2,4,2);

    auto addGroup = [&](const QString &title, QList<QToolButton*> btns) {
        auto *g = ribbonGroup(title, tab);
        auto *row = g->property("groupLayout").value<QHBoxLayout*>();
        for (auto *b : btns) row->addWidget(b);
        row->addStretch();
        hl->addWidget(g);
    };

    addGroup("Clipboard", {
        ribbonButton("📋", "Paste", "paste", true),
        ribbonButton("✂", "Cut",   "cut"),
        ribbonButton("⎘", "Copy",  "copy")
    });

    // Font group with combos
    auto *fontGrp = ribbonGroup("Font", tab);
    auto *frow = fontGrp->property("groupLayout").value<QHBoxLayout*>();
    m_fontCombo = new QComboBox(fontGrp);
    m_fontCombo->addItems(QFontDatabase::families());
    m_fontCombo->setFixedWidth(110);
    m_fontCombo->setCurrentText("Calibri");
    connect(m_fontCombo, &QComboBox::currentTextChanged,
            this, &RibbonBar::fontNameChanged);
    m_sizeCombo = new QComboBox(fontGrp);
    for (int s : {8,9,10,11,12,14,16,18,20,24,28,36})
        m_sizeCombo->addItem(QString::number(s));
    m_sizeCombo->setCurrentText("11");
    m_sizeCombo->setFixedWidth(48);
    connect(m_sizeCombo, &QComboBox::currentTextChanged, this, [this](const QString &s){
        emit fontSizeChanged(s.toInt());
    });
    frow->addWidget(m_fontCombo);
    frow->addWidget(m_sizeCombo);
    frow->addWidget(ribbonButton("B", "Bold",      "bold"));
    frow->addWidget(ribbonButton("I", "Italic",    "italic"));
    frow->addWidget(ribbonButton("U", "Underline", "underline"));
    hl->addWidget(fontGrp);

    addGroup("Alignment", {
        ribbonButton("⬛", "Align Left",   "alignLeft"),
        ribbonButton("≡",  "Center",       "alignCenter"),
        ribbonButton("⬛", "Align Right",  "alignRight"),
        ribbonButton("↵",  "Wrap Text",    "wrapText")
    });

    // Number format combo
    auto *numGrp = ribbonGroup("Number", tab);
    auto *nrow   = numGrp->property("groupLayout").value<QHBoxLayout*>();
    m_fmtCombo   = new QComboBox(numGrp);
    m_fmtCombo->addItems({"General","Number","Currency","Accounting",
                           "Short Date","Long Date","Time","Percentage","Fraction","Scientific"});
    m_fmtCombo->setFixedWidth(100);
    connect(m_fmtCombo, &QComboBox::currentTextChanged,
            this, &RibbonBar::numberFormatChanged);
    nrow->addWidget(m_fmtCombo);
    hl->addWidget(numGrp);

    addGroup("Editing", {
        ribbonButton("Σ", "AutoSum", "autosum", true),
        ribbonButton("↕", "Sort",    "sort",    true),
        ribbonButton("▼", "Filter",  "filter",  true),
        ribbonButton("🔍","Find",    "find",    true)
    });

    hl->addStretch();
}

void RibbonBar::buildInsertTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);
    hl->setSpacing(2); hl->setContentsMargins(4,2,4,2);
    hl->addWidget(ribbonButton("📊","Chart",       "chart",       true));
    hl->addWidget(ribbonButton("📉","Sparkline",   "sparkline",   true));
    hl->addWidget(ribbonButton("🗃","Pivot Table", "pivot",       true));
    hl->addWidget(ribbonButton("⊞", "Table",      "table",       true));
    hl->addWidget(ribbonButton("🖼","Image",       "image",       true));
    hl->addWidget(ribbonButton("💬","Comment",     "comment",     true));
    hl->addWidget(ribbonButton("🔗","Hyperlink",   "hyperlink",   true));
    hl->addStretch();
}

void RibbonBar::buildPageLayoutTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);
    hl->setSpacing(2); hl->setContentsMargins(4,2,4,2);
    hl->addWidget(ribbonButton("🖨","Margins",     "margins",    true));
    hl->addWidget(ribbonButton("📄","Orientation", "orient",     true));
    hl->addWidget(ribbonButton("📐","Paper Size",  "papersize",  true));
    hl->addWidget(ribbonButton("🔢","Print Area",  "printarea",  true));
    hl->addStretch();
}

void RibbonBar::buildFormulasTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);
    hl->setSpacing(2); hl->setContentsMargins(4,2,4,2);
    hl->addWidget(ribbonButton("Σ","AutoSum",      "autosum",    true));
    hl->addWidget(ribbonButton("fx","Insert Fn",   "insertfn",   true));
    hl->addWidget(ribbonButton("⊞","Name Manager", "namemgr",   true));
    hl->addWidget(ribbonButton("↻","Recalculate",  "recalc",    true));
    hl->addWidget(ribbonButton("👁","Show Formulas","showfm",    true));
    hl->addStretch();
}

void RibbonBar::buildDataTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);
    hl->setSpacing(2); hl->setContentsMargins(4,2,4,2);
    hl->addWidget(ribbonButton("↑A","Sort A→Z",   "sortaz",    true));
    hl->addWidget(ribbonButton("↓Z","Sort Z→A",   "sortza",    true));
    hl->addWidget(ribbonButton("▼","Filter",       "filter",    true));
    hl->addWidget(ribbonButton("✗","Clear Filter", "clearfilt", true));
    hl->addWidget(ribbonButton("✂","Remove Dups",  "dedup",     true));
    hl->addWidget(ribbonButton("✓","Validate",     "validate",  true));
    hl->addStretch();
}

void RibbonBar::buildReviewTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);
    hl->setSpacing(2); hl->setContentsMargins(4,2,4,2);
    hl->addWidget(ribbonButton("ABC","Spell Check", "spell",     true));
    hl->addWidget(ribbonButton("💬","Comment",      "comment",   true));
    hl->addWidget(ribbonButton("🔒","Protect Sheet","protect",   true));
    hl->addWidget(ribbonButton("📖","Protect WB",   "protectwb", true));
    hl->addStretch();
}

void RibbonBar::buildViewTab(QWidget *tab)
{
    auto *hl = new QHBoxLayout(tab);
    hl->setSpacing(2); hl->setContentsMargins(4,2,4,2);
    hl->addWidget(ribbonButton("🌙","Dark Mode",    "dark",      true));
    hl->addWidget(ribbonButton("❄","Freeze Panes", "freeze",    true));
    hl->addWidget(ribbonButton("⊡","Grid Lines",   "gridlines", true));
    hl->addWidget(ribbonButton("⛶","Split",        "split",     true));
    hl->addWidget(ribbonButton("🔍","Zoom",         "zoom",      true));
    hl->addStretch();
}

void RibbonBar::setActionEnabled(const QString &, bool)  {}
void RibbonBar::setActionChecked(const QString &, bool)  {}
void RibbonBar::setFontName(const QString &n) { if(m_fontCombo) m_fontCombo->setCurrentText(n); }
void RibbonBar::setFontSize(int s) { if(m_sizeCombo) m_sizeCombo->setCurrentText(QString::number(s)); }
void RibbonBar::setBold(bool)      {}
void RibbonBar::setItalic(bool)    {}
void RibbonBar::setUnderline(bool) {}

} // namespace OpenSheet
