#include "data_validation.h"
#include <QDate>
#include <QTime>
#include <cmath>

namespace OpenSheet {

DataValidator::DataValidator(QObject *parent) : QObject(parent) {}

void DataValidator::addRule(const ValidationRule &rule) { m_rules.append(rule); }

void DataValidator::removeRulesForRange(const CellRange &range)
{
    m_rules.erase(
        std::remove_if(m_rules.begin(), m_rules.end(),
                       [&](const ValidationRule &r){ return r.range == range; }),
        m_rules.end());
}

void DataValidator::clearAll() { m_rules.clear(); }

QVector<ValidationRule> DataValidator::rulesForCell(int row, int col) const
{
    QVector<ValidationRule> result;
    for (const auto &r : m_rules)
        if (r.range.contains(row, col)) result.append(r);
    return result;
}

bool DataValidator::hasRule(int row, int col) const
{
    for (const auto &r : m_rules)
        if (r.range.contains(row, col)) return true;
    return false;
}

bool DataValidator::validate(int row, int col, const QString &rawValue,
                              QString &outMessage) const
{
    auto rules = rulesForCell(row, col);
    for (const auto &rule : rules) {
        if (!checkRule(rule, rawValue, outMessage))
            return false;
    }
    return true;
}

bool DataValidator::checkRule(const ValidationRule &rule,
                               const QString &raw,
                               QString &outMsg) const
{
    if (rule.type == ValidationRule::Type::Any) return true;

    // Blank handling
    if (raw.trimmed().isEmpty()) {
        return rule.ignoreBlank;
    }

    bool numOk;
    double num = raw.toDouble(&numOk);

    switch (rule.type) {

    case ValidationRule::Type::WholeNumber:
        if (!numOk || num != std::floor(num)) {
            outMsg = rule.errorMessage.isEmpty()
                   ? "Value must be a whole number."
                   : rule.errorMessage;
            return false;
        }
        return checkNumeric(rule, num) || (outMsg = rule.errorMessage, false);

    case ValidationRule::Type::Decimal:
        if (!numOk) {
            outMsg = rule.errorMessage.isEmpty()
                   ? "Value must be a number."
                   : rule.errorMessage;
            return false;
        }
        if (!checkNumeric(rule, num)) {
            outMsg = rule.errorMessage.isEmpty()
                   ? QString("Value must be between %1 and %2.")
                     .arg(rule.value1.toString())
                     .arg(rule.value2.toString())
                   : rule.errorMessage;
            return false;
        }
        return true;

    case ValidationRule::Type::List:
        if (!rule.listValues.contains(raw, Qt::CaseInsensitive)) {
            outMsg = rule.errorMessage.isEmpty()
                   ? QString("Value must be one of: %1.")
                     .arg(rule.listValues.join(", "))
                   : rule.errorMessage;
            return false;
        }
        return true;

    case ValidationRule::Type::Date: {
        QDate d = QDate::fromString(raw, "dd/MM/yyyy");
        if (!d.isValid()) d = QDate::fromString(raw, Qt::ISODate);
        if (!d.isValid()) {
            outMsg = rule.errorMessage.isEmpty()
                   ? "Value must be a valid date."
                   : rule.errorMessage;
            return false;
        }
        return true;
    }

    case ValidationRule::Type::Time: {
        QTime t = QTime::fromString(raw, "hh:mm:ss");
        if (!t.isValid()) t = QTime::fromString(raw, "hh:mm");
        if (!t.isValid()) {
            outMsg = rule.errorMessage.isEmpty()
                   ? "Value must be a valid time."
                   : rule.errorMessage;
            return false;
        }
        return true;
    }

    case ValidationRule::Type::TextLength:
        if (!checkTextLength(rule, raw.length())) {
            outMsg = rule.errorMessage.isEmpty()
                   ? QString("Text length must be between %1 and %2 characters.")
                     .arg(rule.value1.toString())
                     .arg(rule.value2.toString())
                   : rule.errorMessage;
            return false;
        }
        return true;

    case ValidationRule::Type::Custom:
        // Formula-based validation evaluated by formula engine (stub)
        return true;

    default:
        return true;
    }
}

bool DataValidator::checkNumeric(const ValidationRule &rule, double value) const
{
    double v1 = rule.value1.toDouble();
    double v2 = rule.value2.toDouble();
    return compareOp(rule.op, value, v1, v2);
}

bool DataValidator::checkTextLength(const ValidationRule &rule, int len) const
{
    return compareOp(rule.op, static_cast<double>(len),
                     rule.value1.toDouble(), rule.value2.toDouble());
}

bool DataValidator::compareOp(ValidationRule::Operator op,
                               double v, double v1, double v2) const
{
    switch (op) {
    case ValidationRule::Operator::Between:       return v >= v1 && v <= v2;
    case ValidationRule::Operator::NotBetween:    return v < v1 || v > v2;
    case ValidationRule::Operator::Equal:         return qFuzzyCompare(v, v1);
    case ValidationRule::Operator::NotEqual:      return !qFuzzyCompare(v, v1);
    case ValidationRule::Operator::GreaterThan:   return v > v1;
    case ValidationRule::Operator::LessThan:      return v < v1;
    case ValidationRule::Operator::GreaterOrEqual:return v >= v1;
    case ValidationRule::Operator::LessOrEqual:   return v <= v1;
    }
    return true;
}

} // namespace OpenSheet
