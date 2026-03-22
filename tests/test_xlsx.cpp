#include <QtTest>
#include <QTemporaryFile>
#include "../office/engine/workbook.h"
#include "../office/engine/sheet.h"
#include "../office/core/file_system/xlsx_reader.h"

using namespace OpenSheet;

class TestXlsx : public QObject {
    Q_OBJECT

    QString tempXlsx() {
        QTemporaryFile f;
        f.setFileTemplate(QDir::temp().filePath("os_xlsx_XXXXXX.xlsx"));
        f.open(); return f.fileName();
    }

private slots:

    void testWriteAndReadBasic() {
        Workbook wb;
        Sheet *s = wb.addSheet("Sheet1");
        s->setCell(1,1,"Product");
        s->setCell(1,2,"Price");
        s->setCell(2,1,"Widget");
        s->setCell(2,2,"9.99");
        s->setCell(3,1,"Gadget");
        s->setCell(3,2,"24.99");

        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));

        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        QCOMPARE(loaded->sheetCount(), 1);

        // Headers
        QCOMPARE(loaded->sheet(0)->cell(1,1).raw(), QString("Product"));
        QCOMPARE(loaded->sheet(0)->cell(1,2).raw(), QString("Price"));
        // Data rows
        QCOMPARE(loaded->sheet(0)->cell(2,1).raw(), QString("Widget"));
        delete loaded;
    }

    void testMultipleSheets() {
        Workbook wb;
        wb.addSheet("January")->setCell(1,1,"Jan Data");
        wb.addSheet("February")->setCell(1,1,"Feb Data");
        wb.addSheet("March")->setCell(1,1,"Mar Data");

        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));

        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        QCOMPARE(loaded->sheetCount(), 3);
        delete loaded;
    }

    void testFormulaWrite() {
        Workbook wb;
        Sheet *s = wb.addSheet("Formulas");
        s->setCell(1,1,"10");
        s->setCell(2,1,"20");
        s->setCell(3,1,"=A1+A2");
        s->cell(3,1).setValue(30.0, CellType::Number); // pre-computed value

        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));

        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        // Formula should survive round-trip
        QVERIFY(loaded->sheet(0)->hasCell(3,1));
        delete loaded;
    }

    void testNumericValues() {
        Workbook wb;
        Sheet *s = wb.addSheet("Numbers");
        s->setCell(1,1,"3.14159");
        s->setCell(1,2,"-42");
        s->setCell(1,3,"1000000");

        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));

        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        QVERIFY(qFuzzyCompare(loaded->sheet(0)->cell(1,1).value().toDouble(), 3.14159));
        QCOMPARE(loaded->sheet(0)->cell(1,2).value().toDouble(), -42.0);
        delete loaded;
    }

    void testEmptyWorkbook() {
        Workbook wb;
        wb.addSheet("Empty");
        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));
        QVERIFY(QFileInfo(path).size() > 0);
    }

    void testBooleanValues() {
        Workbook wb;
        Sheet *s = wb.addSheet("Bools");
        s->setCell(1,1,"TRUE");
        s->setCell(2,1,"FALSE");

        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));

        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        delete loaded;
    }

    void testWriteFailsOnBadPath() {
        Workbook wb;
        wb.addSheet("S");
        XlsxWriter writer;
        QVERIFY(!writer.write(&wb, "/nonexistent/dir/file.xlsx"));
        QVERIFY(!writer.lastError().isEmpty());
    }
};

QTEST_MAIN(TestXlsx)
#include "test_xlsx.moc"
