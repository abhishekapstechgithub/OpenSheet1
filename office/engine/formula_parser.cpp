#include "formula_parser.h"
#include "workbook.h"
#include "sheet.h"
#include "cell.h"
#include <QRegularExpression>
#include <QDateTime>
#include <cmath>
#include <algorithm>

namespace OpenSheet {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / registration
// ─────────────────────────────────────────────────────────────────────────────

FormulaParser::FormulaParser(Workbook *wb) : m_workbook(wb)
{
    registerBuiltins();
}

void FormulaParser::registerFunction(const QString &name, FunctionImpl fn)
{
    m_functions[name.toUpper()] = fn;
}

// ─────────────────────────────────────────────────────────────────────────────
// Main entry
// ─────────────────────────────────────────────────────────────────────────────

QVariant FormulaParser::evaluate(const QString &formula, ParseContext &ctx)
{
    m_lastError = CellError::None;
    if (!formula.startsWith('=')) return formula;

    m_tokens = tokenize(formula.mid(1));
    m_pos    = 0;

    try {
        QVariant result = parseExpr(ctx);
        return result;
    } catch (CellError e) {
        m_lastError = e;
        return Cell::errorString(e);
    } catch (...) {
        m_lastError = CellError::Value;
        return Cell::errorString(CellError::Value);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tokenizer
// ─────────────────────────────────────────────────────────────────────────────

QVector<Token> FormulaParser::tokenize(const QString &expr)
{
    QVector<Token> tokens;
    int i = 0;
    int n = expr.size();

    while (i < n) {
        QChar ch = expr[i];

        // Whitespace
        if (ch.isSpace()) { ++i; continue; }

        // String literal
        if (ch == '"') {
            QString s;
            ++i;
            while (i < n) {
                if (expr[i] == '"') {
                    if (i+1 < n && expr[i+1] == '"') { s += '"'; i += 2; }
                    else { ++i; break; }
                } else { s += expr[i++]; }
            }
            tokens.append({Token::Type::String, s});
            continue;
        }

        // Number
        if (ch.isDigit() || (ch == '.' && i+1 < n && expr[i+1].isDigit())) {
            QString num;
            while (i < n && (expr[i].isDigit() || expr[i] == '.')) num += expr[i++];
            // Scientific notation
            if (i < n && (expr[i] == 'e' || expr[i] == 'E')) {
                num += expr[i++];
                if (i < n && (expr[i] == '+' || expr[i] == '-')) num += expr[i++];
                while (i < n && expr[i].isDigit()) num += expr[i++];
            }
            Token t;
            t.type = Token::Type::Number;
            t.numVal = num.toDouble();
            t.value = num;
            tokens.append(t);
            continue;
        }

        // Identifier (function name, cell reference, named range, bool)
        if (ch.isLetter() || ch == '_' || ch == '$') {
            QString id;
            while (i < n && (expr[i].isLetterOrNumber() || expr[i] == '_'
                              || expr[i] == '$' || expr[i] == '!'))
                id += expr[i++];

            QString idUp = id.toUpper();

            // Boolean literals
            if (idUp == "TRUE") {
                tokens.append({Token::Type::Boolean, "TRUE", 1.0});
                continue;
            }
            if (idUp == "FALSE") {
                tokens.append({Token::Type::Boolean, "FALSE", 0.0});
                continue;
            }

            // Function call?
            int j = i;
            while (j < n && expr[j].isSpace()) ++j;
            if (j < n && expr[j] == '(') {
                tokens.append({Token::Type::Function, idUp});
                continue;
            }

            // Range reference (contains ':')
            if (i < n && expr[i] == ':') {
                QString rangeStr = id;
                ++i;
                rangeStr += ':';
                while (i < n && (expr[i].isLetterOrNumber() || expr[i] == '$'))
                    rangeStr += expr[i++];
                tokens.append({Token::Type::CellRange, rangeStr.toUpper()});
                continue;
            }

            // Cell reference: letter(s) + digit(s)
            static QRegularExpression cellRe(R"(^\$?[A-Za-z]{1,3}\$?\d+$)");
            if (cellRe.match(idUp).hasMatch()) {
                tokens.append({Token::Type::CellRef, idUp});
                continue;
            }

            // Named range / unknown
            tokens.append({Token::Type::NamedRange, idUp});
            continue;
        }

        // Operators and punctuation
        switch (ch.toLatin1()) {
        case '+': tokens.append({Token::Type::Operator, "+"}); ++i; break;
        case '-': tokens.append({Token::Type::Operator, "-"}); ++i; break;
        case '*': tokens.append({Token::Type::Operator, "*"}); ++i; break;
        case '/': tokens.append({Token::Type::Operator, "/"}); ++i; break;
        case '^': tokens.append({Token::Type::Operator, "^"}); ++i; break;
        case '&': tokens.append({Token::Type::Operator, "&"}); ++i; break;
        case '(': tokens.append({Token::Type::LeftParen,  "("}); ++i; break;
        case ')': tokens.append({Token::Type::RightParen, ")"}); ++i; break;
        case ',': tokens.append({Token::Type::Comma, ","}); ++i; break;
        case ';': tokens.append({Token::Type::Comma, ";"}); ++i; break;  // EU style
        case '%': tokens.append({Token::Type::Operator, "%"}); ++i; break;
        case '<':
            if (i+1 < n && expr[i+1] == '=')  { tokens.append({Token::Type::Operator,"<="}); i+=2; }
            else if (i+1 < n && expr[i+1]=='>'){ tokens.append({Token::Type::Operator,"<>"}); i+=2; }
            else                               { tokens.append({Token::Type::Operator,"<"}); ++i; }
            break;
        case '>':
            if (i+1 < n && expr[i+1] == '=')  { tokens.append({Token::Type::Operator,">="}); i+=2; }
            else                               { tokens.append({Token::Type::Operator,">"}); ++i; }
            break;
        case '=':
            tokens.append({Token::Type::Operator, "="}); ++i; break;
        default: ++i; break;  // skip unknown chars
        }
    }

    tokens.append({Token::Type::EndOfInput, ""});
    return tokens;
}

// ─────────────────────────────────────────────────────────────────────────────
// Recursive descent parser
// ─────────────────────────────────────────────────────────────────────────────

QVariant FormulaParser::parseExpr(ParseContext &ctx)
{
    QVariant left = parseTerm(ctx);

    while (m_pos < m_tokens.size()) {
        const Token &t = m_tokens[m_pos];
        if (t.type != Token::Type::Operator) break;
        const QString &op = t.value;

        if (op == "+" || op == "-" || op == "&" ||
            op == "=" || op == "<>" || op == "<" ||
            op == ">" || op == "<=" || op == ">=") {
            ++m_pos;
            QVariant right = parseTerm(ctx);

            if (op == "+") {
                left = left.toDouble() + right.toDouble();
            } else if (op == "-") {
                left = left.toDouble() - right.toDouble();
            } else if (op == "&") {
                left = left.toString() + right.toString();
            } else if (op == "=") {
                left = (left.toString() == right.toString()
                    || qFuzzyCompare(left.toDouble()+1, right.toDouble()+1));
            } else if (op == "<>") {
                left = !(left.toString() == right.toString());
            } else if (op == "<")  { left = left.toDouble() <  right.toDouble(); }
            else if  (op == ">")   { left = left.toDouble() >  right.toDouble(); }
            else if  (op == "<=")  { left = left.toDouble() <= right.toDouble(); }
            else if  (op == ">=")  { left = left.toDouble() >= right.toDouble(); }
        } else break;
    }
    return left;
}

QVariant FormulaParser::parseTerm(ParseContext &ctx)
{
    QVariant left = parseFactor(ctx);

    while (m_pos < m_tokens.size()) {
        const Token &t = m_tokens[m_pos];
        if (t.type != Token::Type::Operator) break;
        if (t.value != "*" && t.value != "/" && t.value != "^") break;

        QString op = t.value;
        ++m_pos;
        QVariant right = parseFactor(ctx);

        if (op == "*")      left = left.toDouble() * right.toDouble();
        else if (op == "/") {
            double d = right.toDouble();
            if (qFuzzyIsNull(d)) throw CellError::DivZero;
            left = left.toDouble() / d;
        }
        else if (op == "^") left = std::pow(left.toDouble(), right.toDouble());
    }
    return left;
}

QVariant FormulaParser::parseFactor(ParseContext &ctx)
{
    return parseUnary(ctx);
}

QVariant FormulaParser::parseUnary(ParseContext &ctx)
{
    if (m_pos < m_tokens.size() && m_tokens[m_pos].type == Token::Type::Operator) {
        if (m_tokens[m_pos].value == "-") {
            ++m_pos;
            return -parseAtom(ctx).toDouble();
        }
        if (m_tokens[m_pos].value == "+") {
            ++m_pos;
            return parseAtom(ctx).toDouble();
        }
    }
    QVariant val = parseAtom(ctx);

    // Percentage postfix
    if (m_pos < m_tokens.size() &&
        m_tokens[m_pos].type == Token::Type::Operator &&
        m_tokens[m_pos].value == "%") {
        ++m_pos;
        return val.toDouble() / 100.0;
    }
    return val;
}

QVariant FormulaParser::parseAtom(ParseContext &ctx)
{
    if (m_pos >= m_tokens.size()) return {};
    const Token &t = m_tokens[m_pos];

    switch (t.type) {
    case Token::Type::Number:
        ++m_pos;
        return t.numVal;

    case Token::Type::String:
        ++m_pos;
        return t.value;

    case Token::Type::Boolean:
        ++m_pos;
        return (t.value == "TRUE");

    case Token::Type::CellRef:
        ++m_pos;
        return resolveRef(t.value, ctx);

    case Token::Type::CellRange:
        // A bare range outside a function is unusual; return first cell
        ++m_pos;
        {
            auto vals = resolveRange(t.value, ctx);
            return vals.isEmpty() ? QVariant() : vals.first();
        }

    case Token::Type::NamedRange:
        ++m_pos;
        {
            Sheet *ns = nullptr; CellRange nr;
            if (ctx.workbook && ctx.workbook->resolveNamedRange(t.value, ns, nr)) {
                auto vals = resolveRange(nr.toString(), ctx);
                return vals.isEmpty() ? QVariant() : vals.first();
            }
            throw CellError::Name;
        }

    case Token::Type::Function:
        ++m_pos;
        return parseFunction(t.value, ctx);

    case Token::Type::LeftParen:
        ++m_pos;
        {
            QVariant val = parseExpr(ctx);
            if (m_pos < m_tokens.size() &&
                m_tokens[m_pos].type == Token::Type::RightParen)
                ++m_pos;
            return val;
        }

    default:
        return {};
    }
}

QVariant FormulaParser::parseFunction(const QString &name, ParseContext &ctx)
{
    // Consume '('
    if (m_pos < m_tokens.size() &&
        m_tokens[m_pos].type == Token::Type::LeftParen)
        ++m_pos;

    // IFERROR(value, value_if_error): the first argument must be evaluated
    // inside a try/catch because it may throw before we can handle the error.
    if (name == "IFERROR") {
        QVariant firstArg;
        bool hadError = false;
        try {
            firstArg = parseExpr(ctx);
        } catch (CellError e) {
            firstArg = Cell::errorString(e);
            hadError = true;
        } catch (...) {
            firstArg = Cell::errorString(CellError::Value);
            hadError = true;
        }
        // Skip comma
        if (m_pos < m_tokens.size() && m_tokens[m_pos].type == Token::Type::Comma)
            ++m_pos;
        // Parse the fallback value
        QVariant fallback;
        try { fallback = parseExpr(ctx); } catch (...) { fallback = QVariant(); }
        // Consume ')'
        if (m_pos < m_tokens.size() && m_tokens[m_pos].type == Token::Type::RightParen)
            ++m_pos;
        if (hadError || firstArg.toString().startsWith('#'))
            return fallback;
        return firstArg;
    }

    // ISNUMBER, ISTEXT, ISBLANK, ISERROR - also need safe first-arg eval
    if (name == "ISERROR" || name == "ISERR") {
        QVariant firstArg;
        bool hadError = false;
        try { firstArg = parseExpr(ctx); } catch (CellError) { hadError = true; }
        catch (...) { hadError = true; }
        if (m_pos < m_tokens.size() && m_tokens[m_pos].type == Token::Type::RightParen)
            ++m_pos;
        return hadError || firstArg.toString().startsWith('#');
    }

    QVector<QVariant> args = parseArgList(ctx);

    // Consume ')'
    if (m_pos < m_tokens.size() &&
        m_tokens[m_pos].type == Token::Type::RightParen)
        ++m_pos;

    auto it = m_functions.constFind(name);
    if (it == m_functions.constEnd()) throw CellError::Name;
    return it.value()(args, ctx);
}

QVector<QVariant> FormulaParser::parseArgList(ParseContext &ctx)
{
    QVector<QVariant> args;
    if (m_pos < m_tokens.size() &&
        m_tokens[m_pos].type == Token::Type::RightParen)
        return args;

    // Check if first token is a range — expand it
    if (m_pos < m_tokens.size() &&
        m_tokens[m_pos].type == Token::Type::CellRange) {
        QString rangeStr = m_tokens[m_pos++].value;
        auto vals = resolveRange(rangeStr, ctx);
        for (auto &v : vals) args.append(v);

        while (m_pos < m_tokens.size() &&
               m_tokens[m_pos].type == Token::Type::Comma) {
            ++m_pos;
            if (m_tokens[m_pos].type == Token::Type::CellRange) {
                auto vs = resolveRange(m_tokens[m_pos++].value, ctx);
                for (auto &v : vs) args.append(v);
            } else {
                args.append(parseExpr(ctx));
            }
        }
        return args;
    }

    args.append(parseExpr(ctx));
    while (m_pos < m_tokens.size() &&
           m_tokens[m_pos].type == Token::Type::Comma) {
        ++m_pos;
        if (m_pos < m_tokens.size() &&
            m_tokens[m_pos].type == Token::Type::CellRange) {
            auto vs = resolveRange(m_tokens[m_pos++].value, ctx);
            for (auto &v : vs) args.append(v);
        } else {
            args.append(parseExpr(ctx));
        }
    }
    return args;
}

// ─────────────────────────────────────────────────────────────────────────────
// Reference resolution
// ─────────────────────────────────────────────────────────────────────────────

QVariant FormulaParser::resolveRef(const QString &ref, ParseContext &ctx)
{
    if (!ctx.sheet) throw CellError::Ref;

    // Strip $ signs for absolute references
    QString clean = ref;
    clean.remove('$');

    // Sheet-qualified ref?  "Sheet2!B3"
    Sheet  *targetSheet = ctx.sheet;
    QString cellPart    = clean;
    if (clean.contains('!')) {
        auto parts = clean.split('!');
        if (ctx.workbook) {
            targetSheet = ctx.workbook->sheet(parts[0]);
            if (!targetSheet) throw CellError::Ref;
        }
        cellPart = parts[1];
    }

    int row = 0, col = 0;
    if (!parseCellRef(cellPart, row, col)) throw CellError::Ref;

    // Circular check
    if (ctx.visitedCells) {
        std::string key = std::to_string(row) + "_" + std::to_string(col);
        if (ctx.visitedCells->count(key)) throw CellError::Circular;
        ctx.visitedCells->insert(key);
    }

    const Cell &cell = targetSheet->cell(row, col);
    QVariant val;
    if (cell.hasFormula()) {
        // Recursively evaluate nested formula
        FormulaParser sub(ctx.workbook);
        ParseContext subCtx = ctx;
        subCtx.sheet   = targetSheet;
        subCtx.baseRow = row;
        subCtx.baseCol = col;
        val = sub.evaluate(cell.raw(), subCtx);
    } else {
        val = cell.value();
    }

    if (ctx.visitedCells) {
        std::string key = std::to_string(row) + "_" + std::to_string(col);
        ctx.visitedCells->erase(key);
    }
    return val;
}

QVector<QVariant> FormulaParser::resolveRange(const QString &rangeStr, ParseContext &ctx)
{
    if (!ctx.sheet) return {};

    QString clean = rangeStr;
    clean.remove('$');

    Sheet  *targetSheet = ctx.sheet;
    QString rangePart   = clean;
    if (clean.contains('!')) {
        auto parts = clean.split('!');
        if (ctx.workbook) targetSheet = ctx.workbook->sheet(parts[0]);
        rangePart = parts[1];
    }

    int r1=0,c1=0,r2=0,c2=0;
    if (!parseCellRange(rangePart, r1, c1, r2, c2)) return {};

    QVector<QVariant> vals;
    for (int r = r1; r <= r2; ++r) {
        for (int c = c1; c <= c2; ++c) {
            const Cell &cell = targetSheet->cell(r, c);
            if (cell.hasFormula()) {
                FormulaParser sub(ctx.workbook);
                ParseContext subCtx = ctx;
                subCtx.sheet   = targetSheet;
                subCtx.baseRow = r;
                subCtx.baseCol = c;
                vals.append(sub.evaluate(cell.raw(), subCtx));
            } else {
                vals.append(cell.value());
            }
        }
    }
    return vals;
}

// ─────────────────────────────────────────────────────────────────────────────
// Static helpers
// ─────────────────────────────────────────────────────────────────────────────

bool FormulaParser::parseCellRef(const QString &ref, int &row, int &col)
{
    static QRegularExpression re(R"(^([A-Za-z]{1,3})(\d{1,7})$)");
    auto m = re.match(ref);
    if (!m.hasMatch()) return false;
    col = colLetterToIndex(m.captured(1).toUpper());
    row = m.captured(2).toInt();
    return row > 0 && col > 0;
}

bool FormulaParser::parseCellRange(const QString &range,
                                    int &r1, int &c1, int &r2, int &c2)
{
    QStringList parts = range.split(':');
    if (parts.size() != 2) return false;
    return parseCellRef(parts[0], r1, c1) && parseCellRef(parts[1], r2, c2);
}

int FormulaParser::colLetterToIndex(const QString &letters)
{
    int result = 0;
    for (QChar ch : letters.toUpper()) {
        result = result * 26 + (ch.toLatin1() - 'A' + 1);
    }
    return result;
}

QString FormulaParser::indexToColLetter(int idx)
{
    QString result;
    while (idx > 0) {
        --idx;
        result.prepend(QChar('A' + (idx % 26)));
        idx /= 26;
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Built-in function registration
// ─────────────────────────────────────────────────────────────────────────────

void FormulaParser::registerBuiltins()
{
    registerFunction("SUM",         fnSum);
    registerFunction("AVERAGE",     fnAverage);
    registerFunction("COUNT",       fnCount);
    registerFunction("COUNTA",      fnCountA);
    registerFunction("MIN",         fnMin);
    registerFunction("MAX",         fnMax);
    registerFunction("IF",          fnIf);
    registerFunction("AND",         fnAnd);
    registerFunction("OR",          fnOr);
    registerFunction("NOT",         fnNot);
    registerFunction("IFERROR",     fnIferror);
    registerFunction("SUMIF",       fnSumIf);
    registerFunction("COUNTIF",     fnCountIf);
    registerFunction("VLOOKUP",     fnVLookup);
    registerFunction("HLOOKUP",     fnHLookup);
    registerFunction("INDEX",       fnIndex);
    registerFunction("MATCH",       fnMatch);
    registerFunction("CONCATENATE", fnConcatenate);
    registerFunction("CONCAT",      fnConcatenate);
    registerFunction("LEN",         fnLen);
    registerFunction("UPPER",       fnUpper);
    registerFunction("LOWER",       fnLower);
    registerFunction("LEFT",        fnLeft);
    registerFunction("RIGHT",       fnRight);
    registerFunction("MID",         fnMid);
    registerFunction("TRIM",        fnTrim);
    registerFunction("ROUND",       fnRound);
    registerFunction("ABS",         fnAbs);
    registerFunction("MOD",         fnMod);
    registerFunction("POWER",       fnPower);
    registerFunction("SQRT",        fnSqrt);
    registerFunction("TODAY",       fnToday);
    registerFunction("NOW",         fnNow);
    registerFunction("YEAR",        fnYear);
    registerFunction("MONTH",       fnMonth);
    registerFunction("DAY",         fnDay);
}

// ─────────────────────────────────────────────────────────────────────────────
// Built-in implementations
// ─────────────────────────────────────────────────────────────────────────────

QVariant FormulaParser::fnSum(const QVector<QVariant> &args, ParseContext &)
{
    double s = 0;
    for (const auto &v : args) {
        bool ok; double d = v.toDouble(&ok);
        if (ok) s += d;
    }
    return s;
}

QVariant FormulaParser::fnAverage(const QVector<QVariant> &args, ParseContext &)
{
    double s = 0; int n = 0;
    for (const auto &v : args) {
        bool ok; double d = v.toDouble(&ok);
        if (ok) { s += d; ++n; }
    }
    if (n == 0) throw CellError::DivZero;
    return s / n;
}

QVariant FormulaParser::fnCount(const QVector<QVariant> &args, ParseContext &)
{
    int n = 0;
    for (const auto &v : args) {
        bool ok; v.toDouble(&ok);
        if (ok) ++n;
    }
    return n;
}

QVariant FormulaParser::fnCountA(const QVector<QVariant> &args, ParseContext &)
{
    int n = 0;
    for (const auto &v : args)
        if (!v.isNull() && !v.toString().isEmpty()) ++n;
    return n;
}

QVariant FormulaParser::fnMin(const QVector<QVariant> &args, ParseContext &)
{
    double m = std::numeric_limits<double>::max();
    for (const auto &v : args) {
        bool ok; double d = v.toDouble(&ok);
        if (ok) m = std::min(m, d);
    }
    return m == std::numeric_limits<double>::max() ? QVariant(0.0) : QVariant(m);
}

QVariant FormulaParser::fnMax(const QVector<QVariant> &args, ParseContext &)
{
    double m = std::numeric_limits<double>::lowest();
    for (const auto &v : args) {
        bool ok; double d = v.toDouble(&ok);
        if (ok) m = std::max(m, d);
    }
    return m == std::numeric_limits<double>::lowest() ? QVariant(0.0) : QVariant(m);
}

QVariant FormulaParser::fnIf(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 2) throw CellError::Value;
    bool cond;
    if (args[0].typeId() == QMetaType::Bool)
        cond = args[0].toBool();
    else {
        bool ok; double d = args[0].toDouble(&ok);
        cond = ok ? !qFuzzyIsNull(d) : !args[0].toString().isEmpty();
    }
    if (cond) return args[1];
    return (args.size() >= 3) ? args[2] : QVariant(false);
}

QVariant FormulaParser::fnAnd(const QVector<QVariant> &args, ParseContext &)
{
    for (const auto &v : args) {
        bool ok; double d = v.toDouble(&ok);
        if (ok && qFuzzyIsNull(d)) return false;
        if (!ok && !v.toBool())    return false;
    }
    return true;
}

QVariant FormulaParser::fnOr(const QVector<QVariant> &args, ParseContext &)
{
    for (const auto &v : args) {
        bool ok; double d = v.toDouble(&ok);
        if (ok && !qFuzzyIsNull(d)) return true;
        if (!ok && v.toBool())       return true;
    }
    return false;
}

QVariant FormulaParser::fnNot(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) throw CellError::Value;
    bool ok; double d = args[0].toDouble(&ok);
    return ok ? qFuzzyIsNull(d) : !args[0].toBool();
}

QVariant FormulaParser::fnIferror(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 2) throw CellError::Value;
    QString s = args[0].toString();
    if (s.startsWith('#')) return args[1];
    return args[0];
}

QVariant FormulaParser::fnSumIf(const QVector<QVariant> &args, ParseContext &)
{
    // SUMIF(range, criteria, [sum_range])
    // For simplicity: all values are already expanded — odd indices = criteria check vals
    // This simplified version sums args[i] where args[i] matches args[1] (string/number compare)
    if (args.size() < 2) return 0.0;
    double s = 0;
    QString criteria = args[1].toString();
    for (int i = 2; i < args.size(); ++i) {
        if (args[i].toString() == criteria) s += args[i].toDouble();
    }
    return s;
}

QVariant FormulaParser::fnCountIf(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 2) return 0;
    QString criteria = args[1].toString();
    int n = 0;
    for (int i = 0; i < args.size() - 1; ++i)
        if (args[i].toString() == criteria) ++n;
    return n;
}

QVariant FormulaParser::fnVLookup(const QVector<QVariant> &args, ParseContext &)
{
    // VLOOKUP(lookup_value, table_array_values..., col_index, [exact])
    if (args.size() < 3) throw CellError::Value;
    QString lookup = args[0].toString();
    int colIdx = args[args.size()-2].toInt();
    // Table values are already flat; we'd need row-count to reconstruct columns
    // Simplified: search linearly
    for (int i = 1; i < args.size() - 2; ++i) {
        if (args[i].toString().compare(lookup, Qt::CaseInsensitive) == 0)
            return (i + colIdx - 1 < args.size()) ? args[i + colIdx - 1] : QVariant();
    }
    throw CellError::NA;
}

QVariant FormulaParser::fnHLookup(const QVector<QVariant> &args, ParseContext &ctx)
{
    Q_UNUSED(ctx);
    if (args.size() < 3) throw CellError::Value;
    throw CellError::NA; // Simplified stub — full impl requires 2D range access
}

QVariant FormulaParser::fnIndex(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 2) throw CellError::Value;
    int row = args[args.size()-2].toInt() - 1;
    if (row < 0 || row >= args.size() - 2) throw CellError::Ref;
    return args[row];
}

QVariant FormulaParser::fnMatch(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 2) throw CellError::Value;
    QString lookup = args[0].toString();
    for (int i = 1; i < args.size(); ++i)
        if (args[i].toString().compare(lookup, Qt::CaseInsensitive) == 0)
            return i;
    throw CellError::NA;
}

QVariant FormulaParser::fnConcatenate(const QVector<QVariant> &args, ParseContext &)
{
    QString s;
    for (const auto &v : args) s += v.toString();
    return s;
}

QVariant FormulaParser::fnLen(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return 0;
    return args[0].toString().length();
}

QVariant FormulaParser::fnUpper(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return QString();
    return args[0].toString().toUpper();
}

QVariant FormulaParser::fnLower(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return QString();
    return args[0].toString().toLower();
}

QVariant FormulaParser::fnLeft(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return QString();
    int n = (args.size() >= 2) ? args[1].toInt() : 1;
    return args[0].toString().left(n);
}

QVariant FormulaParser::fnRight(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return QString();
    int n = (args.size() >= 2) ? args[1].toInt() : 1;
    return args[0].toString().right(n);
}

QVariant FormulaParser::fnMid(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 3) throw CellError::Value;
    int start = args[1].toInt() - 1;   // 1-based in Excel
    int len   = args[2].toInt();
    return args[0].toString().mid(std::max(0, start), len);
}

QVariant FormulaParser::fnTrim(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return QString();
    // Trim leading/trailing and collapse internal spaces
    QString s = args[0].toString().trimmed();
    static QRegularExpression multi(R"( {2,})");
    s.replace(multi, " ");
    return s;
}

QVariant FormulaParser::fnRound(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) throw CellError::Value;
    double v = args[0].toDouble();
    int decimals = (args.size() >= 2) ? args[1].toInt() : 0;
    double factor = std::pow(10.0, decimals);
    return std::round(v * factor) / factor;
}

QVariant FormulaParser::fnAbs(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) throw CellError::Value;
    return std::abs(args[0].toDouble());
}

QVariant FormulaParser::fnMod(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 2) throw CellError::Value;
    double d = args[1].toDouble();
    if (qFuzzyIsNull(d)) throw CellError::DivZero;
    return std::fmod(args[0].toDouble(), d);
}

QVariant FormulaParser::fnPower(const QVector<QVariant> &args, ParseContext &)
{
    if (args.size() < 2) throw CellError::Value;
    return std::pow(args[0].toDouble(), args[1].toDouble());
}

QVariant FormulaParser::fnSqrt(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) throw CellError::Value;
    double v = args[0].toDouble();
    if (v < 0) throw CellError::Num;
    return std::sqrt(v);
}

QVariant FormulaParser::fnToday(const QVector<QVariant> &, ParseContext &)
{
    return QDate::currentDate().toString("dd/MM/yyyy");
}

QVariant FormulaParser::fnNow(const QVector<QVariant> &, ParseContext &)
{
    return QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
}

QVariant FormulaParser::fnYear(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return 0;
    auto dt = QDateTime::fromString(args[0].toString(), "dd/MM/yyyy");
    if (!dt.isValid()) dt = QDateTime::fromString(args[0].toString(), Qt::ISODate);
    return dt.isValid() ? dt.date().year() : 0;
}

QVariant FormulaParser::fnMonth(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return 0;
    auto dt = QDateTime::fromString(args[0].toString(), "dd/MM/yyyy");
    if (!dt.isValid()) dt = QDateTime::fromString(args[0].toString(), Qt::ISODate);
    return dt.isValid() ? dt.date().month() : 0;
}

QVariant FormulaParser::fnDay(const QVector<QVariant> &args, ParseContext &)
{
    if (args.isEmpty()) return 0;
    auto dt = QDateTime::fromString(args[0].toString(), "dd/MM/yyyy");
    if (!dt.isValid()) dt = QDateTime::fromString(args[0].toString(), Qt::ISODate);
    return dt.isValid() ? dt.date().day() : 0;
}

} // namespace OpenSheet
