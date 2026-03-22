#include <QtTest/QtTest>
#include "../office/engine/formula_parser.h"
#include "../office/engine/workbook.h"
#include "../office/engine/sheet.h"

using namespace OpenSheet;

class TestFormulaParser : public QObject {
    Q_OBJECT

private:
    Workbook      *m_wb     = nullptr;
    Sheet         *m_sheet  = nullptr;
    FormulaParser *m_parser = nullptr;

private slots:
    void initTestCase() {
        m_wb     = new Workbook(this);
        m_sheet  = m_wb->addSheet("TestSheet");
        m_parser = new FormulaParser(m_wb);

        // Populate test data
        //      A    B    C
        // 1    10   20   30
        // 2    5    15   25
        // 3    2    8    12
        m_sheet->setCell(1,1,"10"); m_sheet->setCell(1,2,"20"); m_sheet->setCell(1,3,"30");
        m_sheet->setCell(2,1,"5");  m_sheet->setCell(2,2,"15"); m_sheet->setCell(2,3,"25");
        m_sheet->setCell(3,1,"2");  m_sheet->setCell(3,2,"8");  m_sheet->setCell(3,3,"12");
        m_sheet->setCell(4,1,"Hello"); m_sheet->setCell(4,2,"World");
    }

    void cleanupTestCase() { delete m_parser; delete m_wb; }

    // ---- Arithmetic ----

    void testLiteralNumber() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=42", ctx).toDouble(), 42.0);
    }

    void testAddition() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=2+3", ctx).toDouble(), 5.0);
    }

    void testSubtraction() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=10-4", ctx).toDouble(), 6.0);
    }

    void testMultiplication() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=3*7", ctx).toDouble(), 21.0);
    }

    void testDivision() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=20/4", ctx).toDouble(), 5.0);
    }

    void testDivisionByZero() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        auto result = m_parser->evaluate("=1/0", ctx);
        QVERIFY(result.toString().contains("#DIV/0"));
    }

    void testNegation() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=-5+10", ctx).toDouble(), 5.0);
    }

    void testParentheses() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=(2+3)*4", ctx).toDouble(), 20.0);
    }

    void testPrecedence() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=2+3*4", ctx).toDouble(), 14.0);
    }

    void testPower() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=2^10", ctx).toDouble(), 1024.0);
    }

    // ---- Cell References ----

    void testCellRef() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=A1", ctx).toDouble(), 10.0);
        QCOMPARE(m_parser->evaluate("=B2", ctx).toDouble(), 15.0);
    }

    void testCellRefArithmetic() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=A1+B1", ctx).toDouble(), 30.0);
    }

    // ---- Built-in Functions ----

    void testSUM() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=SUM(A1:A3)", ctx).toDouble(), 17.0); // 10+5+2
    }

    void testSUMRange2D() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=SUM(A1:C1)", ctx).toDouble(), 60.0); // 10+20+30
    }

    void testAVERAGE() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        double avg = m_parser->evaluate("=AVERAGE(A1:A3)", ctx).toDouble();
        QCOMPARE(avg, (10.0+5.0+2.0)/3.0);
    }

    void testCOUNT() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=COUNT(A1:A3)", ctx).toDouble(), 3.0);
    }

    void testMIN() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=MIN(A1:C1)", ctx).toDouble(), 10.0);
    }

    void testMAX() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=MAX(A1:C1)", ctx).toDouble(), 30.0);
    }

    void testIF_True() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=IF(1>0,42,99)", ctx).toDouble(), 42.0);
    }

    void testIF_False() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=IF(0>1,42,99)", ctx).toDouble(), 99.0);
    }

    void testROUND() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=ROUND(3.14159,2)", ctx).toDouble(), 3.14);
    }

    void testABS() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=ABS(-42)", ctx).toDouble(), 42.0);
    }

    void testMOD() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=MOD(10,3)", ctx).toDouble(), 1.0);
    }

    void testSQRT() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=SQRT(16)", ctx).toDouble(), 4.0);
    }

    void testLEN() {
        ParseContext ctx{m_sheet, m_wb, 4, 1};
        QCOMPARE(m_parser->evaluate("=LEN(A4)", ctx).toDouble(), 5.0); // "Hello"
    }

    void testUPPER() {
        ParseContext ctx{m_sheet, m_wb, 4, 1};
        QCOMPARE(m_parser->evaluate("=UPPER(A4)", ctx).toString(), QString("HELLO"));
    }

    void testLOWER() {
        ParseContext ctx{m_sheet, m_wb, 4, 1};
        QCOMPARE(m_parser->evaluate("=LOWER(A4)", ctx).toString(), QString("hello"));
    }

    void testCONCATENATE() {
        ParseContext ctx{m_sheet, m_wb, 4, 1};
        QCOMPARE(m_parser->evaluate("=CONCATENATE(A4,\" \",B4)", ctx).toString(),
                 QString("Hello World"));
    }

    void testTRIM() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        m_sheet->setCell(5,1,"  spaces  ");
        QCOMPARE(m_parser->evaluate("=TRIM(A5)", ctx).toString(), QString("spaces"));
    }

    void testTODAY() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        auto result = m_parser->evaluate("=TODAY()", ctx);
        QVERIFY(!result.isNull());
    }

    void testIFERROR() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        QCOMPARE(m_parser->evaluate("=IFERROR(1/0,\"ERR\")", ctx).toString(),
                 QString("ERR"));
    }

    void testNestedFunctions() {
        ParseContext ctx{m_sheet, m_wb, 1, 1};
        double result = m_parser->evaluate("=SUM(A1:A3)*AVERAGE(B1:B3)", ctx).toDouble();
        double expected = 17.0 * ((20.0+15.0+8.0)/3.0);
        QVERIFY(qAbs(result - expected) < 0.001);
    }

    // ---- Column Letter Parsing ----

    void testColLetterToIndex() {
        QCOMPARE(FormulaParser::colLetterToIndex("A"),  1);
        QCOMPARE(FormulaParser::colLetterToIndex("Z"), 26);
        QCOMPARE(FormulaParser::colLetterToIndex("AA"),27);
        QCOMPARE(FormulaParser::colLetterToIndex("AB"),28);
        QCOMPARE(FormulaParser::colLetterToIndex("AZ"),52);
    }

    void testIndexToColLetter() {
        QCOMPARE(FormulaParser::indexToColLetter(1),  QString("A"));
        QCOMPARE(FormulaParser::indexToColLetter(26), QString("Z"));
        QCOMPARE(FormulaParser::indexToColLetter(27), QString("AA"));
    }

    // ---- Cell Range Parsing ----

    void testParseCellRange() {
        int r1,c1,r2,c2;
        QVERIFY(FormulaParser::parseCellRange("A1:C3",r1,c1,r2,c2));
        QCOMPARE(r1,1); QCOMPARE(c1,1); QCOMPARE(r2,3); QCOMPARE(c2,3);
    }
};

QTEST_MAIN(TestFormulaParser)
#include "test_formula_parser.moc"
