#pragma once
#include <QDialog>
#include "../engine/cell.h"

class QTabWidget;
class QListWidget;
class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QFontComboBox;

namespace OpenSheet {

/**
 * Full-featured "Format Cells" dialog with tabs:
 *   Number | Alignment | Font | Border | Fill | Protection
 */
class CellFormatDialog : public QDialog {
    Q_OBJECT
public:
    explicit CellFormatDialog(QWidget *parent = nullptr);

    void        setFormat(const CellFormat &fmt);
    CellFormat  format() const;

private slots:
    void onCategoryChanged(int idx);
    void onFormatCodeChanged(const QString &code);
    void onPreviewUpdate();

private:
    void buildNumberTab(QWidget *tab);
    void buildAlignmentTab(QWidget *tab);
    void buildFontTab(QWidget *tab);
    void buildBorderTab(QWidget *tab);
    void buildFillTab(QWidget *tab);

    CellFormat m_format;

    // Number tab
    QListWidget *m_categoryList = nullptr;
    QListWidget *m_formatList   = nullptr;
    QLineEdit   *m_codeEdit     = nullptr;
    QLabel      *m_preview      = nullptr;
    QSpinBox    *m_decimalsSpin = nullptr;

    // Alignment tab
    QComboBox   *m_hAlignCombo  = nullptr;
    QComboBox   *m_vAlignCombo  = nullptr;
    QCheckBox   *m_wrapCheck    = nullptr;
    QCheckBox   *m_mergeCheck   = nullptr;
    QSpinBox    *m_indentSpin   = nullptr;

    // Font tab
    QFontComboBox *m_fontCombo  = nullptr;
    QSpinBox      *m_sizeSpin   = nullptr;
    QCheckBox     *m_boldCheck  = nullptr;
    QCheckBox     *m_italicCheck= nullptr;
    QCheckBox     *m_underCheck = nullptr;
    QCheckBox     *m_strikeCheck= nullptr;

    // Fill tab
    class ColorButton *m_fgColorBtn = nullptr;
    class ColorButton *m_bgColorBtn = nullptr;
};

} // namespace OpenSheet
