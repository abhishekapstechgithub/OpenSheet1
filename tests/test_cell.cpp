#include <QtTest>
#include "cell.h"

using namespace OpenSheet;

class TestCell : public QObject {
    Q_OBJECT
private slots:
    void testEmptyCell() {
        Cell c;
        QCOMPARE(c.type(), CellType::Empty);
        QVERIFY(c.isEmpty());
        QCOMPARE(c.displayText(), QString());
    }

    void testNumberCell() {
        Cell c("42");
        QCOMPARE(c.type(), CellType::Number);
        QCOMPARE(c.value().toDouble(), 42.0);
        QVERIFY(!c.isEmpty());
    }

    void testNegativeNumber() {
        Cell c("-3.14");
        QCOMPARE(c.type(), CellType::Number);
        QVERIFY(qFuzzyCompare(c.value().toDouble(), -3.14));
    }

    void testTextCell() {
        Cell c("Hello World");
        QCOMPARE(c.type(), CellType::Text);
        QCOMPARE(c.value().toString(), QString("Hello World"));
    }

    void testBooleanTrue() {
        Cell c("TRUE");
        QCOMPARE(c.type(), CellType::Boolean);
        QCOMPARE(c.value().toBool(), true);
    }

    void testBooleanFalse() {
        Cell c("FALSE");
        QCOMPARE(c.type(), CellType::Boolean);
        QCOMPARE(c.value().toBool(), false);
    }

    void testFormulaCell() {
        Cell c("=SUM(A1:A5)");
        QCOMPARE(c.type(), CellType::Formula);
        QVERIFY(c.hasFormula());
        QCOMPARE(c.raw(), QString("=SUM(A1:A5)"));
    }

    void testErrorCell() {
        Cell c;
        c.setError(CellError::DivZero);
        QCOMPARE(c.type(), CellType::Error);
        QCOMPARE(c.displayText(), QString("#DIV/0!"));
    }

    void testCellFormat() {
        Cell c("test");
        CellFormat fmt;
        fmt.bold = true;
        fmt.foreground = QColor(Qt::red);
        c.setFormat(fmt);
        QCOMPARE(c.format().bold, true);
        QCOMPARE(c.format().foreground, QColor(Qt::red));
    }

    void testSetRawUpdatesType() {
        Cell c("hello");
        QCOMPARE(c.type(), CellType::Text);
        c.setRaw("123");
        QCOMPARE(c.type(), CellType::Number);
        c.setRaw("");
        QCOMPARE(c.type(), CellType::Empty);
    }

    void testComment() {
        Cell c("data");
        c.setComment("This is a note");
        QCOMPARE(c.comment(), QString("This is a note"));
    }

    void testErrorStrings() {
        QCOMPARE(Cell::errorString(CellError::Name),     QString("#NAME?"));
        QCOMPARE(Cell::errorString(CellError::Value),    QString("#VALUE!"));
        QCOMPARE(Cell::errorString(CellError::Ref),      QString("#REF!"));
        QCOMPARE(Cell::errorString(CellError::Circular), QString("#CIRC!"));
    }
};

QTEST_MAIN(TestCell)
#include "test_cell.moc"
