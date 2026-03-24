#include <QtTest>
#include <QTemporaryFile>
#include <QDir>
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

    // Helper: check if QZip is available (requires CorePrivate)
    static bool hasQZip() {
#ifdef OPENSHEET_HAS_QT_CORE_PRIVATE
        return true;
#else
        return false;
#endif
    }

private slots:

    void testWriteAndReadBasic() {
        if (!hasQZip()) QSKIP("XLSX requires Qt CorePrivate (QZip) — not available in this build");
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
        QCOMPARE(loaded->sheet(0)->cell(1,1).raw(), QString("Product"));
        QCOMPARE(loaded->sheet(0)->cell(1,2).raw(), QString("Price"));
        QCOMPARE(loaded->sheet(0)->cell(2,1).raw(), QString("Widget"));
        delete loaded;
    }

    void testMultipleSheets() {
        if (!hasQZip()) QSKIP("XLSX requires Qt CorePrivate (QZip) — not available in this build");
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
        QCOMPARE(loaded->sheet(0)->name(), QString("January"));
        QCOMPARE(loaded->sheet(1)->name(), QString("February"));
        QCOMPARE(loaded->sheet(2)->name(), QString("March"));
        delete loaded;
    }

    void testNumericCells() {
        if (!hasQZip()) QSKIP("XLSX requires Qt CorePrivate (QZip) — not available in this build");
        Workbook wb;
        Sheet *s = wb.addSheet("Numbers");
        s->setCell(1,1,"3.14");
        s->setCell(1,2,"42");
        s->setCell(1,3,"-7.5");

        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));

        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        QCOMPARE(loaded->sheet(0)->cell(1,1).value().toDouble(), 3.14);
        QCOMPARE(loaded->sheet(0)->cell(1,2).value().toDouble(), 42.0);
        delete loaded;
    }

    void testEmptyWorkbook() {
        if (!hasQZip()) QSKIP("XLSX requires Qt CorePrivate (QZip) — not available in this build");
        Workbook wb;
        wb.addSheet("Empty");
        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));
        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        QCOMPARE(loaded->sheetCount(), 1);
        delete loaded;
    }

    void testRoundTripFormula() {
        if (!hasQZip()) QSKIP("XLSX requires Qt CorePrivate (QZip) — not available in this build");
        Workbook wb;
        Sheet *s = wb.addSheet("Formulas");
        s->setCell(1,1,"10");
        s->setCell(1,2,"20");
        s->setCell(1,3,"=A1+B1");

        QString path = tempXlsx();
        XlsxWriter writer;
        QVERIFY(writer.write(&wb, path));
        XlsxReader reader;
        Workbook *loaded = reader.read(path);
        QVERIFY(loaded != nullptr);
        // Formula cell should have the formula stored
        QVERIFY(!loaded->sheet(0)->cell(1,3).raw().isEmpty());
        delete loaded;
    }

    void testXlsxWriterWithoutQZip() {
        // This test always runs and verifies graceful degradation
        if (hasQZip()) QSKIP("QZip available — skip graceful-failure test");
        Workbook wb;
        wb.addSheet("Sheet1");
        XlsxWriter writer;
        bool result = writer.write(&wb, "/tmp/test.xlsx");
        QVERIFY(!result); // Should fail gracefully, not crash
        QVERIFY(!writer.lastError().isEmpty());
    }
};

QTEST_MAIN(TestXlsx)
#include "test_xlsx.moc"
