#include <QtTest/QtTest>
#include "../office/engine/cell.h"
#include "../office/engine/sheet.h"
#include "../office/engine/workbook.h"
#include "../office/engine/cell_range.h"
#include "../office/core/file_system/csv_reader.h"

using namespace OpenSheet;

// ============================================================
// Cell Tests
// ============================================================
class TestCell : public QObject {
    Q_OBJECT
private slots:
    void testEmptyCell() {
        Cell c;
        QVERIFY(c.isEmpty());
        QCOMPARE(c.type(), CellType::Empty);
    }

    void testNumberCell() {
        Cell c("42.5");
        QCOMPARE(c.type(), CellType::Number);
        QCOMPARE(c.value().toDouble(), 42.5);
        QVERIFY(!c.isEmpty());
    }

    void testTextCell() {
        Cell c("Hello");
        QCOMPARE(c.type(), CellType::Text);
        QCOMPARE(c.value().toString(), QString("Hello"));
    }

    void testBooleanTrue() {
        Cell c("TRUE");
        QCOMPARE(c.type(), CellType::Boolean);
        QVERIFY(c.value().toBool());
    }

    void testBooleanFalse() {
        Cell c("FALSE");
        QCOMPARE(c.type(), CellType::Boolean);
        QVERIFY(!c.value().toBool());
    }

    void testFormulaCell() {
        Cell c("=SUM(A1:A10)");
        QCOMPARE(c.type(), CellType::Formula);
        QVERIFY(c.hasFormula());
        QCOMPARE(c.raw(), QString("=SUM(A1:A10)"));
    }

    void testNegativeNumber() {
        Cell c("-3.14");
        QCOMPARE(c.type(), CellType::Number);
        QCOMPARE(c.value().toDouble(), -3.14);
    }

    void testFormatDefault() {
        Cell c("test");
        QVERIFY(!c.format().bold);
        QVERIFY(!c.format().italic);
    }

    void testSetFormat() {
        Cell c("test");
        CellFormat fmt;
        fmt.bold = true;
        fmt.foreground = Qt::red;
        c.setFormat(fmt);
        QVERIFY(c.format().bold);
        QCOMPARE(c.format().foreground, Qt::red);
    }

    void testErrorStrings() {
        QCOMPARE(Cell::errorString(CellError::DivZero),  QString("#DIV/0!"));
        QCOMPARE(Cell::errorString(CellError::Name),     QString("#NAME?"));
        QCOMPARE(Cell::errorString(CellError::Circular), QString("#CIRC!"));
        QCOMPARE(Cell::errorString(CellError::Value),    QString("#VALUE!"));
    }

    void testComment() {
        Cell c("data");
        c.setComment("My note here");
        QCOMPARE(c.comment(), QString("My note here"));
    }

    void testHyperlink() {
        Cell c("Click here");
        c.setHyperlink("https://example.com");
        QCOMPARE(c.hyperlink(), QString("https://example.com"));
    }
};

// ============================================================
// Sheet Tests
// ============================================================
class TestSheet : public QObject {
    Q_OBJECT
private slots:
    void testNewSheetName() {
        Sheet s("TestSheet");
        QCOMPARE(s.name(), QString("TestSheet"));
    }

    void testSetGetCell() {
        Sheet s("S");
        s.setCell(1, 1, "hello");
        QCOMPARE(s.cell(1,1).raw(), QString("hello"));
        QVERIFY(s.hasCell(1,1));
    }

    void testClearCell() {
        Sheet s("S");
        s.setCell(2,3,"data");
        QVERIFY(s.hasCell(2,3));
        s.clearCell(2,3);
        QVERIFY(!s.hasCell(2,3));
    }

    void testClearRange() {
        Sheet s("S");
        for (int r=1;r<=3;r++) for (int c=1;c<=3;c++) s.setCell(r,c,QString::number(r*c));
        s.clearRange(CellRange(1,1,3,3));
        for (int r=1;r<=3;r++) for (int c=1;c<=3;c++) QVERIFY(!s.hasCell(r,c));
    }

    void testMaxRowCol() {
        Sheet s("S");
        s.setCell(5,8,"x");
        QCOMPARE(s.maxRow(), 5);
        QCOMPARE(s.maxCol(), 8);
    }

    void testInsertRow() {
        Sheet s("S");
        s.setCell(1,1,"A"); s.setCell(2,1,"B"); s.setCell(3,1,"C");
        s.insertRow(2);
        QCOMPARE(s.cell(1,1).raw(), QString("A"));
        QCOMPARE(s.cell(3,1).raw(), QString("B"));
        QCOMPARE(s.cell(4,1).raw(), QString("C"));
        QVERIFY(!s.hasCell(2,1)); // empty inserted row
    }

    void testDeleteRow() {
        Sheet s("S");
        s.setCell(1,1,"A"); s.setCell(2,1,"B"); s.setCell(3,1,"C");
        s.deleteRow(2);
        QCOMPARE(s.cell(1,1).raw(), QString("A"));
        QCOMPARE(s.cell(2,1).raw(), QString("C"));
        QVERIFY(!s.hasCell(3,1));
    }

    void testSortAscending() {
        Sheet s("S");
        s.setCell(1,1,"Charlie"); s.setCell(2,1,"Alice"); s.setCell(3,1,"Bob");
        s.sortRange(CellRange(1,1,3,1), 1, true);
        QCOMPARE(s.cell(1,1).raw(), QString("Alice"));
        QCOMPARE(s.cell(2,1).raw(), QString("Bob"));
        QCOMPARE(s.cell(3,1).raw(), QString("Charlie"));
    }

    void testSortDescending() {
        Sheet s("S");
        s.setCell(1,1,"Charlie"); s.setCell(2,1,"Alice"); s.setCell(3,1,"Bob");
        s.sortRange(CellRange(1,1,3,1), 1, false);
        QCOMPARE(s.cell(1,1).raw(), QString("Charlie"));
    }

    void testFreezeRowCol() {
        Sheet s("S");
        s.setFreezeRow(2);
        s.setFreezeCol(1);
        QCOMPARE(s.freezeRow(), 2);
        QCOMPARE(s.freezeCol(), 1);
    }

    void testVisibility() {
        Sheet s("S");
        QVERIFY(s.isVisible());
        s.setVisible(false);
        QVERIFY(!s.isVisible());
    }
};

// ============================================================
// Workbook Tests
// ============================================================
class TestWorkbook : public QObject {
    Q_OBJECT
private slots:
    void testAddSheets() {
        Workbook wb;
        wb.addSheet("Sheet1");
        wb.addSheet("Sheet2");
        QCOMPARE(wb.sheetCount(), 2);
        QCOMPARE(wb.sheet(0)->name(), QString("Sheet1"));
        QCOMPARE(wb.sheet(1)->name(), QString("Sheet2"));
    }

    void testRemoveSheet() {
        Workbook wb;
        wb.addSheet("A"); wb.addSheet("B"); wb.addSheet("C");
        wb.removeSheet(1); // remove B
        QCOMPARE(wb.sheetCount(), 2);
        QCOMPARE(wb.sheet(0)->name(), QString("A"));
        QCOMPARE(wb.sheet(1)->name(), QString("C"));
    }

    void testActiveSheet() {
        Workbook wb;
        wb.addSheet("X"); wb.addSheet("Y");
        wb.setActiveSheet(1);
        QCOMPARE(wb.activeSheetIndex(), 1);
        QCOMPARE(wb.activeSheet()->name(), QString("Y"));
    }

    void testSheetByName() {
        Workbook wb;
        wb.addSheet("Alpha");
        wb.addSheet("Beta");
        QVERIFY(wb.sheet("Alpha") != nullptr);
        QVERIFY(wb.sheet("Gamma") == nullptr);
    }

    void testModified() {
        Workbook wb;
        QVERIFY(!wb.isModified());
        wb.setModified(true);
        QVERIFY(wb.isModified());
    }

    void testFilePath() {
        Workbook wb;
        QVERIFY(wb.filePath().isEmpty());
        wb.setFilePath("/tmp/test.opensheet");
        QCOMPARE(wb.filePath(), QString("/tmp/test.opensheet"));
    }

    void testSheetNames() {
        Workbook wb;
        wb.addSheet("One"); wb.addSheet("Two"); wb.addSheet("Three");
        auto names = wb.sheetNames();
        QCOMPARE(names, QStringList({"One","Two","Three"}));
    }

    void testMoveSheet() {
        Workbook wb;
        wb.addSheet("A"); wb.addSheet("B"); wb.addSheet("C");
        wb.moveSheet(0, 2); // A moves to position 2
        QCOMPARE(wb.sheet(0)->name(), QString("B"));
        QCOMPARE(wb.sheet(1)->name(), QString("C"));
        QCOMPARE(wb.sheet(2)->name(), QString("A"));
    }
};

// ============================================================
// CellRange Tests
// ============================================================
class TestCellRange : public QObject {
    Q_OBJECT
private slots:
    void testBasicRange() {
        CellRange r(1,1,5,5);
        QCOMPARE(r.top(),1); QCOMPARE(r.left(),1);
        QCOMPARE(r.bottom(),5); QCOMPARE(r.right(),5);
        QCOMPARE(r.rowCount(),5); QCOMPARE(r.colCount(),5);
    }

    void testContains() {
        CellRange r(2,2,4,4);
        QVERIFY(r.contains(2,2));
        QVERIFY(r.contains(4,4));
        QVERIFY(r.contains(3,3));
        QVERIFY(!r.contains(1,1));
        QVERIFY(!r.contains(5,5));
    }

    void testIsValid() {
        QVERIFY(CellRange(1,1,3,3).isValid());
        QVERIFY(!CellRange(0,1,3,3).isValid());
        QVERIFY(!CellRange(3,1,1,3).isValid());
    }

    void testFromString() {
        auto r = CellRange::fromString("B3:D7");
        QCOMPARE(r.top(),3); QCOMPARE(r.left(),2);
        QCOMPARE(r.bottom(),7); QCOMPARE(r.right(),4);
    }
};

// ============================================================
// CSV Tests
// ============================================================
class TestCsvReader : public QObject {
    Q_OBJECT
private slots:
    void testDelimiterDetection() {
        QCOMPARE(CsvReader::detectDelimiter("a,b,c"),     QChar(','));
        QCOMPARE(CsvReader::detectDelimiter("a;b;c"),     QChar(';'));
        QCOMPARE(CsvReader::detectDelimiter("a\tb\tc"),   QChar('\t'));
    }

    void testRoundTrip() {
        // Write CSV, read it back, verify contents
        Workbook wb;
        Sheet *s = wb.addSheet("Test");
        s->setCell(1,1,"Name"); s->setCell(1,2,"Score");
        s->setCell(2,1,"Alice"); s->setCell(2,2,"95");
        s->setCell(3,1,"Bob");   s->setCell(3,2,"87");

        CsvWriter writer;
        QString tmpPath = QDir::temp().filePath("opensheet_test.csv");
        QVERIFY(writer.write(&wb, tmpPath, 0));

        CsvReader reader;
        Workbook *wb2 = reader.read(tmpPath);
        QVERIFY(wb2 != nullptr);
        QCOMPARE(wb2->sheet(0)->cell(1,1).raw(), QString("Name"));
        QCOMPARE(wb2->sheet(0)->cell(2,1).raw(), QString("Alice"));
        QCOMPARE(wb2->sheet(0)->cell(2,2).raw(), QString("95"));
        delete wb2;
        QFile::remove(tmpPath);
    }

    void testQuotedFields() {
        CsvReader reader(',');
        // Fields with commas inside quotes
        QString csv = "\"Smith, John\",42\n\"Doe, Jane\",38\n";
        QTemporaryFile f;
        f.open();
        f.write(csv.toUtf8());
        f.flush();
        Workbook *wb = reader.read(f.fileName());
        QVERIFY(wb != nullptr);
        QCOMPARE(wb->sheet(0)->cell(1,1).raw(), QString("Smith, John"));
        QCOMPARE(wb->sheet(0)->cell(2,1).raw(), QString("Doe, Jane"));
        delete wb;
    }
};

// ============================================================
// Run all test classes
// ============================================================
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;

    { TestCell t;     status |= QTest::qExec(&t, argc, argv); }
    { TestSheet t;    status |= QTest::qExec(&t, argc, argv); }
    { TestWorkbook t; status |= QTest::qExec(&t, argc, argv); }
    { TestCellRange t; status |= QTest::qExec(&t, argc, argv); }
    { TestCsvReader t; status |= QTest::qExec(&t, argc, argv); }

    return status;
}

#include "test_engine.moc"
