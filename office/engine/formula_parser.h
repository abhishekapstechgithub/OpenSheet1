#pragma once
#include "cell.h"
#include <QString>
#include <QVariant>
#include <QVector>
#include <functional>
#include <unordered_set>

namespace OpenSheet {

class Sheet;
class Workbook;

// Represents a parsed token
struct Token {
    enum class Type {
        Number, String, Boolean, CellRef, CellRange,
        Operator, LeftParen, RightParen, Comma,
        Function, NamedRange, Error, EndOfInput
    };
    Type    type;
    QString value;
    double  numVal = 0.0;
};

// Context passed to the parser for cell resolution
struct ParseContext {
    Sheet    *sheet    = nullptr;
    Workbook *workbook = nullptr;
    int       baseRow  = 0; // for relative references
    int       baseCol  = 0;
    std::unordered_set<std::string> *visitedCells = nullptr; // circular detection
};

using FunctionImpl = std::function<QVariant(const QVector<QVariant>&, ParseContext&)>;

class FormulaParser {
public:
    explicit FormulaParser(Workbook *wb);

    // Main entry: evaluate formula string (starts with '=')
    QVariant evaluate(const QString &formula, ParseContext &ctx);

    // Register a custom function
    void registerFunction(const QString &name, FunctionImpl fn);

    // Parse a cell reference like "B3" -> {row, col}
    static bool parseCellRef(const QString &ref, int &row, int &col);

    // Parse range like "B2:E6" -> {r1,c1,r2,c2}
    static bool parseCellRange(const QString &range,
                                int &r1, int &c1, int &r2, int &c2);

    // Translate column letter(s) to 1-based index: A->1, Z->26, AA->27
    static int  colLetterToIndex(const QString &letters);
    static QString indexToColLetter(int idx);

    CellError lastError() const { return m_lastError; }

private:
    Workbook        *m_workbook;
    QVector<Token>   m_tokens;
    int              m_pos = 0;
    CellError        m_lastError = CellError::None;
    QHash<QString, FunctionImpl> m_functions;

    // Tokenizer
    QVector<Token> tokenize(const QString &expr);

    // Recursive descent parser
    QVariant parseExpr(ParseContext &ctx);
    QVariant parseTerm(ParseContext &ctx);
    QVariant parseFactor(ParseContext &ctx);
    QVariant parseUnary(ParseContext &ctx);
    QVariant parseAtom(ParseContext &ctx);
    QVariant parseFunction(const QString &name, ParseContext &ctx);
    QVector<QVariant> parseArgList(ParseContext &ctx);

    // Range helpers
    QVector<QVariant> resolveRange(const QString &rangeStr, ParseContext &ctx);
    QVariant          resolveRef(const QString &ref, ParseContext &ctx);

    void registerBuiltins();

    // Built-in function implementations
    static QVariant fnSum(const QVector<QVariant>&, ParseContext&);
    static QVariant fnAverage(const QVector<QVariant>&, ParseContext&);
    static QVariant fnCount(const QVector<QVariant>&, ParseContext&);
    static QVariant fnCountA(const QVector<QVariant>&, ParseContext&);
    static QVariant fnMin(const QVector<QVariant>&, ParseContext&);
    static QVariant fnMax(const QVector<QVariant>&, ParseContext&);
    static QVariant fnIf(const QVector<QVariant>&, ParseContext&);
    static QVariant fnAnd(const QVector<QVariant>&, ParseContext&);
    static QVariant fnOr(const QVector<QVariant>&, ParseContext&);
    static QVariant fnNot(const QVector<QVariant>&, ParseContext&);
    static QVariant fnVLookup(const QVector<QVariant>&, ParseContext&);
    static QVariant fnHLookup(const QVector<QVariant>&, ParseContext&);
    static QVariant fnIndex(const QVector<QVariant>&, ParseContext&);
    static QVariant fnMatch(const QVector<QVariant>&, ParseContext&);
    static QVariant fnConcatenate(const QVector<QVariant>&, ParseContext&);
    static QVariant fnLen(const QVector<QVariant>&, ParseContext&);
    static QVariant fnUpper(const QVector<QVariant>&, ParseContext&);
    static QVariant fnLower(const QVector<QVariant>&, ParseContext&);
    static QVariant fnLeft(const QVector<QVariant>&, ParseContext&);
    static QVariant fnRight(const QVector<QVariant>&, ParseContext&);
    static QVariant fnMid(const QVector<QVariant>&, ParseContext&);
    static QVariant fnTrim(const QVector<QVariant>&, ParseContext&);
    static QVariant fnRound(const QVector<QVariant>&, ParseContext&);
    static QVariant fnAbs(const QVector<QVariant>&, ParseContext&);
    static QVariant fnMod(const QVector<QVariant>&, ParseContext&);
    static QVariant fnPower(const QVector<QVariant>&, ParseContext&);
    static QVariant fnSqrt(const QVector<QVariant>&, ParseContext&);
    static QVariant fnToday(const QVector<QVariant>&, ParseContext&);
    static QVariant fnNow(const QVector<QVariant>&, ParseContext&);
    static QVariant fnYear(const QVector<QVariant>&, ParseContext&);
    static QVariant fnMonth(const QVector<QVariant>&, ParseContext&);
    static QVariant fnDay(const QVector<QVariant>&, ParseContext&);
    static QVariant fnIferror(const QVector<QVariant>&, ParseContext&);
    static QVariant fnSumIf(const QVector<QVariant>&, ParseContext&);
    static QVariant fnCountIf(const QVector<QVariant>&, ParseContext&);
};

} // namespace OpenSheet
