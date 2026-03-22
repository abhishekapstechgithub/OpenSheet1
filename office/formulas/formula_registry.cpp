#include "formula_registry.h"

namespace OpenSheet {

FormulaRegistry::FormulaRegistry(QObject *parent) : QObject(parent)
{
    buildCatalog();
}

void FormulaRegistry::registerAll(FormulaParser *parser) const
{
    // All built-in functions are already registered inside FormulaParser
    // via registerBuiltins(). This method exists so that plugin-provided
    // functions added to the registry can also be registered into a parser.
    Q_UNUSED(parser);
}

QVector<FormulaRegistry::FunctionMeta> FormulaRegistry::allFunctions() const
{
    return m_catalog;
}

QVector<FormulaRegistry::FunctionMeta>
FormulaRegistry::byCategory(const QString &cat) const
{
    QVector<FunctionMeta> result;
    for (const auto &f : m_catalog)
        if (f.category.compare(cat, Qt::CaseInsensitive) == 0)
            result.append(f);
    return result;
}

QVector<FormulaRegistry::FunctionMeta>
FormulaRegistry::search(const QString &query) const
{
    QVector<FunctionMeta> result;
    for (const auto &f : m_catalog)
        if (f.name.contains(query, Qt::CaseInsensitive) ||
            f.description.contains(query, Qt::CaseInsensitive))
            result.append(f);
    return result;
}

const FormulaRegistry::FunctionMeta *
FormulaRegistry::find(const QString &name) const
{
    for (const auto &f : m_catalog)
        if (f.name.compare(name, Qt::CaseInsensitive) == 0)
            return &f;
    return nullptr;
}

QStringList FormulaRegistry::categories() const
{
    QStringList cats;
    for (const auto &f : m_catalog)
        if (!cats.contains(f.category)) cats.append(f.category);
    cats.sort();
    return cats;
}

void FormulaRegistry::buildCatalog()
{
    // ── Math & Trig ────────────────────────────────────────────────────────
    m_catalog << FunctionMeta{
        "SUM", "Math",
        "SUM(number1, [number2], …)",
        "Returns the sum of all numbers in the argument list.",
        {"number1", "number2"},
        {"The first number or range to add.", "Additional numbers or ranges (optional)."}
    };
    m_catalog << FunctionMeta{
        "ROUND", "Math",
        "ROUND(number, num_digits)",
        "Rounds a number to a specified number of digits.",
        {"number", "num_digits"},
        {"The value to round.", "Number of decimal places (0 = integer)."}
    };
    m_catalog << FunctionMeta{
        "ABS", "Math", "ABS(number)",
        "Returns the absolute value of a number.",
        {"number"}, {"The number whose absolute value you want."}
    };
    m_catalog << FunctionMeta{
        "MOD", "Math", "MOD(number, divisor)",
        "Returns the remainder after dividing number by divisor.",
        {"number", "divisor"},
        {"The number to divide.", "The divisor."}
    };
    m_catalog << FunctionMeta{
        "POWER", "Math", "POWER(number, power)",
        "Returns the result of a number raised to a power.",
        {"number", "power"},
        {"The base number.", "The exponent."}
    };
    m_catalog << FunctionMeta{
        "SQRT", "Math", "SQRT(number)",
        "Returns the positive square root of a number.",
        {"number"}, {"A positive number."}
    };

    // ── Statistical ────────────────────────────────────────────────────────
    m_catalog << FunctionMeta{
        "AVERAGE", "Statistical",
        "AVERAGE(number1, [number2], …)",
        "Returns the arithmetic mean of its arguments.",
        {"number1","number2"},
        {"The first number or range.", "Additional numbers or ranges (optional)."}
    };
    m_catalog << FunctionMeta{
        "COUNT", "Statistical",
        "COUNT(value1, [value2], …)",
        "Counts the number of cells that contain numbers.",
        {"value1","value2"},
        {"The first item or range.", "Additional items (optional)."}
    };
    m_catalog << FunctionMeta{
        "COUNTA", "Statistical",
        "COUNTA(value1, [value2], …)",
        "Counts the number of non-empty cells.",
        {"value1","value2"},
        {"The first item or range.", "Additional items (optional)."}
    };
    m_catalog << FunctionMeta{
        "MIN", "Statistical",
        "MIN(number1, [number2], …)",
        "Returns the smallest value in a set of values.",
        {"number1","number2"},
        {"The first value or range.", "Additional values (optional)."}
    };
    m_catalog << FunctionMeta{
        "MAX", "Statistical",
        "MAX(number1, [number2], …)",
        "Returns the largest value in a set of values.",
        {"number1","number2"},
        {"The first value or range.", "Additional values (optional)."}
    };
    m_catalog << FunctionMeta{
        "SUMIF", "Statistical",
        "SUMIF(range, criteria, [sum_range])",
        "Adds the cells specified by a given condition or criteria.",
        {"range","criteria","sum_range"},
        {"The range to evaluate.", "The condition.", "The range to sum (optional, defaults to range)."}
    };
    m_catalog << FunctionMeta{
        "COUNTIF", "Statistical",
        "COUNTIF(range, criteria)",
        "Counts the number of cells that meet a condition.",
        {"range","criteria"},
        {"The range to evaluate.", "The condition."}
    };

    // ── Logical ───────────────────────────────────────────────────────────
    m_catalog << FunctionMeta{
        "IF", "Logical",
        "IF(logical_test, value_if_true, [value_if_false])",
        "Returns one value if condition is true, another if false.",
        {"logical_test","value_if_true","value_if_false"},
        {"The condition to test.", "Value returned if TRUE.", "Value returned if FALSE (optional)."}
    };
    m_catalog << FunctionMeta{
        "AND", "Logical",
        "AND(logical1, [logical2], …)",
        "Returns TRUE if all arguments are TRUE.",
        {"logical1","logical2"},
        {"The first condition.", "Additional conditions (optional)."}
    };
    m_catalog << FunctionMeta{
        "OR", "Logical",
        "OR(logical1, [logical2], …)",
        "Returns TRUE if any argument is TRUE.",
        {"logical1","logical2"},
        {"The first condition.", "Additional conditions (optional)."}
    };
    m_catalog << FunctionMeta{
        "NOT", "Logical", "NOT(logical)",
        "Reverses the logic of its argument.",
        {"logical"}, {"A value or expression that can be TRUE or FALSE."}
    };
    m_catalog << FunctionMeta{
        "IFERROR", "Logical",
        "IFERROR(value, value_if_error)",
        "Returns value_if_error if value evaluates to an error; otherwise value.",
        {"value","value_if_error"},
        {"The expression to evaluate.", "Value to return on error."}
    };

    // ── Text ──────────────────────────────────────────────────────────────
    m_catalog << FunctionMeta{
        "CONCATENATE", "Text",
        "CONCATENATE(text1, [text2], …)",
        "Joins several text strings into one text string.",
        {"text1","text2"},
        {"The first text item.", "Additional text items (optional)."}
    };
    m_catalog << FunctionMeta{
        "LEN", "Text", "LEN(text)",
        "Returns the number of characters in a text string.",
        {"text"}, {"The text whose length you want."}
    };
    m_catalog << FunctionMeta{
        "UPPER", "Text", "UPPER(text)",
        "Converts text to uppercase.",
        {"text"}, {"The text to convert."}
    };
    m_catalog << FunctionMeta{
        "LOWER", "Text", "LOWER(text)",
        "Converts text to lowercase.",
        {"text"}, {"The text to convert."}
    };
    m_catalog << FunctionMeta{
        "LEFT", "Text", "LEFT(text, [num_chars])",
        "Returns the leftmost characters from a text value.",
        {"text","num_chars"},
        {"The text string.", "Number of characters (default 1)."}
    };
    m_catalog << FunctionMeta{
        "RIGHT", "Text", "RIGHT(text, [num_chars])",
        "Returns the rightmost characters from a text value.",
        {"text","num_chars"},
        {"The text string.", "Number of characters (default 1)."}
    };
    m_catalog << FunctionMeta{
        "MID", "Text", "MID(text, start_num, num_chars)",
        "Returns a specific number of characters from a text string, starting at the position you specify.",
        {"text","start_num","num_chars"},
        {"The text string.", "Starting position (1-based).", "Number of characters to return."}
    };
    m_catalog << FunctionMeta{
        "TRIM", "Text", "TRIM(text)",
        "Removes extra spaces from text.",
        {"text"}, {"The text from which to remove spaces."}
    };

    // ── Date & Time ───────────────────────────────────────────────────────
    m_catalog << FunctionMeta{
        "TODAY", "Date & Time", "TODAY()",
        "Returns the serial number of today's date.",
        {}, {}
    };
    m_catalog << FunctionMeta{
        "NOW", "Date & Time", "NOW()",
        "Returns the serial number of the current date and time.",
        {}, {}
    };
    m_catalog << FunctionMeta{
        "YEAR", "Date & Time", "YEAR(serial_number)",
        "Returns the year corresponding to a date.",
        {"serial_number"}, {"A date value."}
    };
    m_catalog << FunctionMeta{
        "MONTH", "Date & Time", "MONTH(serial_number)",
        "Returns the month as an integer (1–12).",
        {"serial_number"}, {"A date value."}
    };
    m_catalog << FunctionMeta{
        "DAY", "Date & Time", "DAY(serial_number)",
        "Returns the day of the month (1–31).",
        {"serial_number"}, {"A date value."}
    };

    // ── Lookup & Reference ────────────────────────────────────────────────
    m_catalog << FunctionMeta{
        "VLOOKUP", "Lookup",
        "VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])",
        "Looks up a value in the first column of a table and returns a value in the same row from another column.",
        {"lookup_value","table_array","col_index_num","range_lookup"},
        {"The value to look for.", "The table range.", "Column number to return.", "FALSE for exact match (default)."}
    };
    m_catalog << FunctionMeta{
        "HLOOKUP", "Lookup",
        "HLOOKUP(lookup_value, table_array, row_index_num, [range_lookup])",
        "Looks up a value in the first row of a table and returns from another row.",
        {"lookup_value","table_array","row_index_num","range_lookup"},
        {"The value to look for.", "The table range.", "Row number to return.", "FALSE for exact match."}
    };
    m_catalog << FunctionMeta{
        "INDEX", "Lookup",
        "INDEX(array, row_num, [col_num])",
        "Returns a value or the reference to a value from within a range.",
        {"array","row_num","col_num"},
        {"The range.", "Row number.", "Column number (optional)."}
    };
    m_catalog << FunctionMeta{
        "MATCH", "Lookup",
        "MATCH(lookup_value, lookup_array, [match_type])",
        "Returns the relative position of a value in an array.",
        {"lookup_value","lookup_array","match_type"},
        {"The value to find.", "The range to search.", "0 for exact match."}
    };
}

} // namespace OpenSheet
