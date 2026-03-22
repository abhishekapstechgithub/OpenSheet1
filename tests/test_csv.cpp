#include <QtTest>
#include <QTemporaryFile>
#include "file_system/csv_reader.h"
#include "workbook.h"
#include "sheet.h"

using namespace OpenSheet;

class TestCsv : public QObject {
    Q_OBJECT
private slots:
    void testReadSimple() {
        QTemporaryFile f;
        f.open();
        f.write("Name,Age,City\nAlice,30,London\nBob,25,Paris\n");
        f.close();

        CsvReader reader;
        Workbook *wb = reader.read(f.fileName());
        QVERIFY(wb != nullptr);
        QCOMPARE(wb->sheetCount(), 1);
        Sheet *s = wb->sheet(0);
        QCOMPARE(s->cell(1,1).raw(), QString("Name"));
        QCOMPARE(s->cell(1,2).raw(), QString("Age"));
        QCOMPARE(s->cell(2,1).raw(), QString("Alice"));
        QCOMPARE(s->cell(3,2).raw(), QString("25"));
        delete wb;
    }

    void testReadWithQuotes() {
        QTemporaryFile f;
        f.open();
        f.write("\"Last, Name\",Score\n\"Smith, John\",95\n");
        f.close();

        CsvReader reader;
        Workbook *wb = reader.read(f.fileName());
        QVERIFY(wb != nullptr);
        QCOMPARE(wb->sheet(0)->cell(1,1).raw(), QString("Last, Name"));
        QCOMPARE(wb->sheet(0)->cell(2,1).raw(), QString("Smith, John"));
        delete wb;
    }

    void testDetectDelimiter_Tab() {
        QString line = "col1\tcol2\tcol3";
        QCOMPARE(CsvReader::detectDelimiter(line), QChar('\t'));
    }

    void testDetectDelimiter_Semicolon() {
        QString line = "col1;col2;col3";
        QCOMPARE(CsvReader::detectDelimiter(line), QChar(';'));
    }

    void testWriteAndRead() {
        Workbook wb;
        Sheet *s = wb.addSheet("Data");
        s->setCell(1,1,"Product"); s->setCell(1,2,"Price");
        s->setCell(2,1,"Widget");  s->setCell(2,2,"9.99");

        QTemporaryFile f;
        f.open(); f.close();

        CsvWriter writer;
        QVERIFY(writer.write(&wb, f.fileName()));

        CsvReader reader;
        Workbook *wb2 = reader.read(f.fileName());
        QVERIFY(wb2 != nullptr);
        QCOMPARE(wb2->sheet(0)->cell(1,1).raw(), QString("Product"));
        QCOMPARE(wb2->sheet(0)->cell(2,2).raw(), QString("9.99"));
        delete wb2;
    }

    void testEmptyFile() {
        QTemporaryFile f;
        f.open(); f.close();
        CsvReader reader;
        Workbook *wb = reader.read(f.fileName());
        QVERIFY(wb == nullptr);
        QVERIFY(!reader.lastError().isEmpty());
    }

    void testEscapeCommaInField() {
        CsvWriter writer;
        // Write a workbook where a field contains a comma
        Workbook wb;
        Sheet *s = wb.addSheet("T");
        s->setCell(1,1,"Hello, World");
        QTemporaryFile f; f.open(); f.close();
        QVERIFY(writer.write(&wb, f.fileName()));
        // Read it back
        CsvReader reader;
        Workbook *wb2 = reader.read(f.fileName());
        QVERIFY(wb2 != nullptr);
        QCOMPARE(wb2->sheet(0)->cell(1,1).raw(), QString("Hello, World"));
        delete wb2;
    }
};

QTEST_MAIN(TestCsv)
#include "test_csv.moc"
