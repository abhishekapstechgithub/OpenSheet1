#include "chart_dialog.h"
#include "../engine/sheet.h"
#include "../engine/cell.h"
#include "../engine/cell_range.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QFrame>

namespace OpenSheet {

ChartDialog::ChartDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Insert Chart");
    setMinimumSize(640, 480);

    auto *hl = new QHBoxLayout(this);

    // Left panel: type + options
    auto *leftPanel = new QWidget(this);
    auto *lvl = new QVBoxLayout(leftPanel);

    // Chart type list
    lvl->addWidget(new QLabel("Chart type:", leftPanel));
    m_typeList = new QListWidget(leftPanel);
    m_typeList->setMaximumWidth(130);
    m_typeList->setMaximumHeight(180);
    const QList<QPair<QString,QString>> types = {
        {"📊","Bar"},{"📈","Line"},{"🥧","Pie"},
        {"⚫","Scatter"},{"🏔","Area"}
    };
    for (const auto &[icon, name] : types)
        m_typeList->addItem(icon + "  " + name);
    m_typeList->setCurrentRow(0);
    lvl->addWidget(m_typeList);

    // Data range
    auto *rangeBox = new QGroupBox("Data", leftPanel);
    auto *rgl = new QGridLayout(rangeBox);
    rgl->addWidget(new QLabel("Range:"), 0, 0);
    m_rangeEdit = new QLineEdit(rangeBox);
    m_rangeEdit->setPlaceholderText("e.g. A1:E6");
    rgl->addWidget(m_rangeEdit, 0, 1);
    m_rangeStatus = new QLabel("", rangeBox);
    m_rangeStatus->setStyleSheet("color:#888;font-size:10px;");
    rgl->addWidget(m_rangeStatus, 1, 0, 1, 2);
    lvl->addWidget(rangeBox);

    // Chart options
    auto *optBox = new QGroupBox("Options", leftPanel);
    auto *ogl = new QGridLayout(optBox);
    ogl->addWidget(new QLabel("Title:"), 0, 0);
    m_titleEdit = new QLineEdit(optBox);
    m_titleEdit->setPlaceholderText("Chart title…");
    ogl->addWidget(m_titleEdit, 0, 1);
    ogl->addWidget(new QLabel("Legend:"), 1, 0);
    m_legendCombo = new QComboBox(optBox);
    m_legendCombo->addItems({"Bottom","Top","Right","Left","None"});
    ogl->addWidget(m_legendCombo, 1, 1);
    m_dataLabels = new QCheckBox("Show data labels", optBox);
    ogl->addWidget(m_dataLabels, 2, 0, 1, 2);
    lvl->addWidget(optBox);
    lvl->addStretch();

    hl->addWidget(leftPanel);

    // Right panel: preview
    auto *rightPanel = new QFrame(this);
    rightPanel->setFrameShape(QFrame::StyledPanel);
    auto *rvl = new QVBoxLayout(rightPanel);
    rvl->addWidget(new QLabel("Preview:", rightPanel));

    m_config.type = ChartType::Bar;
    m_preview = ChartRenderer::create(ChartType::Bar, rightPanel);
    m_preview->setMinimumSize(340, 260);
    rvl->addWidget(m_preview, 1);
    hl->addWidget(rightPanel, 1);

    // Buttons
    auto *vl = new QVBoxLayout;
    hl->addLayout(vl);

    auto *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    vl->addStretch();
    vl->addWidget(btns);

    // Connections
    connect(m_typeList,    &QListWidget::currentRowChanged,
            this, &ChartDialog::onTypeSelected);
    connect(m_rangeEdit,   &QLineEdit::textChanged,
            this, &ChartDialog::onRangeChanged);
    connect(m_titleEdit,   &QLineEdit::textChanged,
            this, &ChartDialog::onTitleChanged);
    connect(m_legendCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChartDialog::onLegendChanged);
    connect(m_dataLabels,  &QCheckBox::toggled,
            this, [this](bool){ onPreviewUpdate(); });
}

void ChartDialog::setSheet(Sheet *s) { m_sheet = s; }

void ChartDialog::setDataRange(const CellRange &range)
{
    m_rangeEdit->setText(range.toString());
}

void ChartDialog::onTypeSelected(int row)
{
    static const ChartType types[] = {
        ChartType::Bar, ChartType::Line, ChartType::Pie,
        ChartType::Scatter, ChartType::Area
    };
    if (row >= 0 && row < 5) {
        m_config.type = types[row];
        delete m_preview;
        auto *parent = qobject_cast<QFrame*>(sender() ? sender()->parent() : nullptr);
        if (!parent) parent = qobject_cast<QFrame*>(this->children().last());
        m_preview = ChartRenderer::create(m_config.type, this);
        m_preview->setMinimumSize(340, 260);
        onPreviewUpdate();
    }
}

void ChartDialog::onRangeChanged(const QString &rangeStr)
{
    if (!m_sheet) return;
    CellRange r = CellRange::fromString(rangeStr.trimmed().toUpper());
    if (!r.isValid()) {
        m_rangeStatus->setText("Invalid range");
        return;
    }
    m_rangeStatus->setText(QString("%1 rows × %2 cols")
                           .arg(r.rowCount()).arg(r.colCount()));
    onPreviewUpdate();
}

void ChartDialog::onTitleChanged(const QString &t)
{
    m_config.title = t;
    if (m_preview) m_preview->setConfig(m_config);
}

void ChartDialog::onLegendChanged(int idx)
{
    static const LegendPos poses[] = {
        LegendPos::Bottom, LegendPos::Top,
        LegendPos::Right,  LegendPos::Left, LegendPos::None
    };
    if (idx >= 0 && idx < 5) m_config.legend = poses[idx];
    onPreviewUpdate();
}

void ChartDialog::onPreviewUpdate()
{
    if (!m_sheet || !m_preview) return;

    // Build series from the range
    CellRange r = CellRange::fromString(
        m_rangeEdit->text().trimmed().toUpper());
    if (!r.isValid()) { m_preview->setConfig(m_config); return; }

    m_config.series.clear();
    m_config.showDataLabels = m_dataLabels->isChecked();

    // Treat first column as labels, remaining as series
    ChartSeries s;
    s.name = "Series 1";
    for (int row = r.top() + 1; row <= r.bottom(); ++row) {
        s.labels.append(m_sheet->cell(row, r.left()).displayText());
        bool ok;
        double v = m_sheet->cell(row, r.left() + 1).value().toDouble(&ok);
        s.yValues.append(ok ? v : 0.0);
    }
    if (!s.yValues.isEmpty()) m_config.series.append(s);

    m_preview->setConfig(m_config);
    m_preview->update();
}

} // namespace OpenSheet
