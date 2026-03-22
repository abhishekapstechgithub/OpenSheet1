#pragma once
#include "cell_range.h"
#include <QObject>
#include <QVariant>
#include <QString>
#include <QVector>

namespace OpenSheet {

class Cell;

/**
 * Defines a validation rule applied to a range of cells.
 * Equivalent to Excel's Data Validation feature.
 */
struct ValidationRule {
    enum class Type {
        Any,            // No restriction
        WholeNumber,    // Integer only
        Decimal,        // Floating point
        List,           // Value from a dropdown list
        Date,           // Valid date
        Time,           // Valid time
        TextLength,     // String length constraint
        Custom          // Custom formula
    };

    enum class Operator {
        Between, NotBetween,
        Equal, NotEqual,
        GreaterThan, LessThan,
        GreaterOrEqual, LessOrEqual
    };

    enum class AlertStyle { Stop, Warning, Information };

    Type        type        = Type::Any;
    Operator    op          = Operator::Between;
    QVariant    value1;
    QVariant    value2;
    QStringList listValues; // for Type::List
    QString     formula;    // for Type::Custom
    bool        ignoreBlank = true;
    bool        showDropdown = true;

    // Input message
    bool    showInputMsg  = false;
    QString inputTitle;
    QString inputMessage;

    // Error alert
    bool       showError  = true;
    AlertStyle alertStyle = AlertStyle::Stop;
    QString    errorTitle;
    QString    errorMessage;

    // Range this rule applies to
    CellRange range;
};

/**
 * Validates cell values against attached ValidationRules.
 * Each Sheet can own a DataValidator instance.
 */
class DataValidator : public QObject {
    Q_OBJECT
public:
    explicit DataValidator(QObject *parent = nullptr);

    void addRule(const ValidationRule &rule);
    void removeRulesForRange(const CellRange &range);
    void clearAll();

    QVector<ValidationRule> rulesForCell(int row, int col) const;
    bool hasRule(int row, int col) const;

    /**
     * Validate a proposed raw value for a cell.
     * Returns true if valid (or no rule applies).
     * outMessage contains the error/warning message if invalid.
     */
    bool validate(int row, int col, const QString &rawValue,
                  QString &outMessage) const;

    const QVector<ValidationRule> &allRules() const { return m_rules; }

private:
    QVector<ValidationRule> m_rules;

    bool checkRule(const ValidationRule &rule, const QString &raw,
                   QString &outMsg) const;
    bool checkNumeric(const ValidationRule &rule, double value) const;
    bool checkTextLength(const ValidationRule &rule, int len) const;
    bool compareOp(ValidationRule::Operator op, double v, double v1, double v2) const;
};

} // namespace OpenSheet
