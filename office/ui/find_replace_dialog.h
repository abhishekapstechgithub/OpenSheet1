#pragma once
#include <QDialog>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;

namespace OpenSheet {

class Sheet;
class Workbook;

class FindReplaceDialog : public QDialog {
    Q_OBJECT
public:
    explicit FindReplaceDialog(QWidget *parent = nullptr);

    void setWorkbook(Workbook *wb);
    void setCurrentSheet(Sheet *sheet);

    // Open in Find-only mode (hides Replace widgets)
    void openFind();
    // Open in Find+Replace mode
    void openFindReplace();

signals:
    void cellFound(int row, int col);
    void replacementsMade(int count);

private slots:
    void onFindNext();
    void onFindPrev();
    void onReplace();
    void onReplaceAll();
    void onClose();

private:
    struct Match { int row; int col; };
    QVector<Match> findAllMatches();
    bool           matchesQuery(const QString &cellText) const;

    Workbook  *m_workbook    = nullptr;
    Sheet     *m_sheet       = nullptr;
    int        m_matchIdx    = 0;

    QLineEdit   *m_findEdit      = nullptr;
    QLineEdit   *m_replaceEdit   = nullptr;
    QCheckBox   *m_caseSensitive = nullptr;
    QCheckBox   *m_wholeCell     = nullptr;
    QCheckBox   *m_useRegex      = nullptr;
    QCheckBox   *m_allSheets     = nullptr;
    QLabel      *m_statusLabel   = nullptr;
    QWidget     *m_replaceRow    = nullptr; // row with Replace + ReplaceAll
};

} // namespace OpenSheet
