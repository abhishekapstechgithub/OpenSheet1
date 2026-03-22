#include <QtTest>
#include <QTemporaryFile>
#include "../office/engine/workbook.h"
#include "../office/engine/sheet.h"
#include "../office/engine/cell.h"
#include "../office/core/file_system/opensheet_format.h"

using namespace OpenSheet;

class TestOpenSheetFormat : public QObject {
    Q_OBJECT

    QString tempPath() {
        QTemporaryFile f;
        f.setFileTemplate(QDir::temp().filePath("os_test_XXXXXX.opensheet"));
        f.open(); return f.fileName();
    }

private slots:

    void testWriteAndRead_Basic() {
        Workbook wb;
        Sheet *s = wb.addSheet("TestSheet");
        s->setCell(1,1,"Hello");
        s->setCell(1,2,"World");
        s->setCell(2,1,"42");
        s->setCell(3,1,"=A2+1");

        QString path = tempPath();
        QString err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        QVERIFY(err.isEmpty());

        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded != nullptr);
        QVERIFY(err.isEmpty());

        QCOMPARE(loaded->sheetCount(), 1);
        QCOMPARE(loaded->sheet(0)->name(), QString("TestSheet"));
        QCOMPARE(loaded->sheet(0)->cell(1,1).raw(), QString("Hello"));
        QCOMPARE(loaded->sheet(0)->cell(1,2).raw(), QString("World"));
        QCOMPARE(loaded->sheet(0)->cell(2,1).raw(), QString("42"));
        QCOMPARE(loaded->sheet(0)->cell(3,1).raw(), QString("=A2+1"));
        delete loaded;
    }

    void testMultipleSheets() {
        Workbook wb;
        for (int i = 0; i < 5; ++i) {
            Sheet *s = wb.addSheet(QString("Sheet%1").arg(i+1));
            s->setCell(1,1,QString("Data%1").arg(i));
        }
        QString path = tempPath(), err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded);
        QCOMPARE(loaded->sheetCount(), 5);
        for (int i = 0; i < 5; ++i)
            QCOMPARE(loaded->sheet(i)->name(), QString("Sheet%1").arg(i+1));
        delete loaded;
    }

    void testCellFormats() {
        Workbook wb;
        Sheet *s = wb.addSheet("Fmt");
        s->setCell(1,1,"Bold Text");
        CellFormat fmt;
        fmt.bold       = true;
        fmt.foreground = QColor(Qt::red);
        fmt.alignment  = Qt::AlignCenter;
        s->cell(1,1).setFormat(fmt);

        QString path = tempPath(), err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded);
        const CellFormat &rf = loaded->sheet(0)->cell(1,1).format();
        QVERIFY(rf.bold);
        QCOMPARE(rf.foreground, QColor(Qt::red));
        delete loaded;
    }

    void testCellComment() {
        Workbook wb;
        Sheet *s = wb.addSheet("Notes");
        s->setCell(2,3,"Important value");
        s->cell(2,3).setComment("Reviewed by finance team");

        QString path = tempPath(), err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded);
        QCOMPARE(loaded->sheet(0)->cell(2,3).comment(),
                 QString("Reviewed by finance team"));
        delete loaded;
    }

    void testFreezePanes() {
        Workbook wb;
        Sheet *s = wb.addSheet("Frozen");
        s->setFreezeRow(2);
        s->setFreezeCol(1);

        QString path = tempPath(), err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded);
        QCOMPARE(loaded->sheet(0)->freezeRow(), 2);
        QCOMPARE(loaded->sheet(0)->freezeCol(), 1);
        delete loaded;
    }

    void testLargeSheet() {
        Workbook wb;
        Sheet *s = wb.addSheet("Big");
        for (int r = 1; r <= 100; ++r)
            for (int c = 1; c <= 20; ++c)
                s->setCell(r, c, QString("%1").arg(r * 100 + c));

        QString path = tempPath(), err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded);
        QCOMPARE(loaded->sheet(0)->cell(50,10).raw(), QString("5010"));
        QCOMPARE(loaded->sheet(0)->cell(100,20).raw(), QString("10020"));
        delete loaded;
    }

    void testEmptyWorkbook() {
        Workbook wb;
        wb.addSheet("Empty");
        QString path = tempPath(), err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded);
        QCOMPARE(loaded->sheetCount(), 1);
        QCOMPARE(loaded->sheet(0)->maxRow(), 0);
        delete loaded;
    }

    void testInvalidPathFails() {
        Workbook wb;
        wb.addSheet("S");
        QString err;
        QVERIFY(!OpenSheetFormat::write(&wb, "/nonexistent/path/file.opensheet", err));
        QVERIFY(!err.isEmpty());
    }

    void testReadNonexistentFile() {
        QString err;
        Workbook *wb = OpenSheetFormat::read("/no/such/file.opensheet", err);
        QVERIFY(wb == nullptr);
        QVERIFY(!err.isEmpty());
    }

    void testHyperlinkRoundtrip() {
        Workbook wb;
        Sheet *s = wb.addSheet("Links");
        s->setCell(1,1,"GitHub");
        s->cell(1,1).setHyperlink("https://github.com/opensheet");

        QString path = tempPath(), err;
        QVERIFY(OpenSheetFormat::write(&wb, path, err));
        Workbook *loaded = OpenSheetFormat::read(path, err);
        QVERIFY(loaded);
        QCOMPARE(loaded->sheet(0)->cell(1,1).hyperlink(),
                 QString("https://github.com/opensheet"));
        delete loaded;
    }
};

QTEST_MAIN(TestOpenSheetFormat)
#include "test_opensheet_format.moc"
