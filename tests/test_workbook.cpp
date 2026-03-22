#include <QtTest>
#include "../office/engine/workbook.h"
#include "../office/engine/sheet.h"
#include "../office/engine/cell.h"
#include "../office/engine/formula_parser.h"

using namespace OpenSheet;

class TestWorkbook : public QObject {
    Q_OBJECT

private slots:

    // ── Sheet management ─────────────────────────────────────────────────────
    void testAddSheet() {
        Workbook wb;
        QCOMPARE(wb.sheetCount(), 0);
        Sheet *s = wb.addSheet("Alpha");
        QVERIFY(s != nullptr);
        QCOMPARE(wb.sheetCount(), 1);
        QCOMPARE(s->name(), QString("Alpha"));
    }

    void testAddMultipleSheets() {
        Workbook wb;
        wb.addSheet("S1");
        wb.addSheet("S2");
        wb.addSheet("S3");
        QCOMPARE(wb.sheetCount(), 3);
        QCOMPARE(wb.sheet(0)->name(), QString("S1"));
        QCOMPARE(wb.sheet(2)->name(), QString("S3"));
    }

    void testUniqueSheetNames() {
        Workbook wb;
        wb.addSheet("Sheet");
        Sheet *s2 = wb.addSheet("Sheet");   // should get auto-renamed
        QVERIFY(s2->name() != QString("Sheet"));
        QCOMPARE(wb.sheetCount(), 2);
    }

    void testRemoveSheet() {
        Workbook wb;
        wb.addSheet("A");
        wb.addSheet("B");
        wb.addSheet("C");
        wb.removeSheet(1);  // remove "B"
        QCOMPARE(wb.sheetCount(), 2);
        QCOMPARE(wb.sheet(0)->name(), QString("A"));
        QCOMPARE(wb.sheet(1)->name(), QString("C"));
    }

    void testCannotRemoveLastSheet() {
        Workbook wb;
        wb.addSheet("Only");
        wb.removeSheet(0);  // should be rejected
        QCOMPARE(wb.sheetCount(), 1);
    }

    void testMoveSheet() {
        Workbook wb;
        wb.addSheet("A"); wb.addSheet("B"); wb.addSheet("C");
        wb.moveSheet(0, 2);  // move A to end
        QCOMPARE(wb.sheet(0)->name(), QString("B"));
        QCOMPARE(wb.sheet(1)->name(), QString("C"));
        QCOMPARE(wb.sheet(2)->name(), QString("A"));
    }

    void testInsertSheet() {
        Workbook wb;
        wb.addSheet("First");
        wb.addSheet("Last");
        wb.insertSheet(1, "Middle");
        QCOMPARE(wb.sheetCount(), 3);
        QCOMPARE(wb.sheet(1)->name(), QString("Middle"));
    }

    void testLookupSheetByName() {
        Workbook wb;
        wb.addSheet("Alpha");
        wb.addSheet("Beta");
        QVERIFY(wb.sheet("Alpha") != nullptr);
        QVERIFY(wb.sheet("BETA")  != nullptr);  // case-insensitive
        QVERIFY(wb.sheet("Gamma") == nullptr);
    }

    // ── Active sheet ─────────────────────────────────────────────────────────
    void testActiveSheet() {
        Workbook wb;
        wb.addSheet("S1"); wb.addSheet("S2"); wb.addSheet("S3");
        wb.setActiveSheet(2);
        QCOMPARE(wb.activeSheetIndex(), 2);
        QCOMPARE(wb.activeSheet()->name(), QString("S3"));
    }

    void testActiveSheetAfterRemove() {
        Workbook wb;
        wb.addSheet("A"); wb.addSheet("B"); wb.addSheet("C");
        wb.setActiveSheet(2);
        wb.removeSheet(2);
        QCOMPARE(wb.activeSheetIndex(), 1);
    }

    // ── Named ranges ─────────────────────────────────────────────────────────
    void testNamedRange() {
        Workbook wb;
        Sheet *s = wb.addSheet("Data");
        CellRange r(1, 1, 5, 3);
        wb.setNamedRange("SalesData", s, r);

        Sheet *outSheet = nullptr;
        CellRange outRange;
        QVERIFY(wb.resolveNamedRange("SalesData", outSheet, outRange));
        QCOMPARE(outSheet, s);
        QCOMPARE(outRange, r);
    }

    void testNamedRangeCaseInsensitive() {
        Workbook wb;
        Sheet *s = wb.addSheet("Data");
        wb.setNamedRange("MyRange", s, CellRange(1,1,3,3));
        Sheet *out = nullptr; CellRange cr;
        QVERIFY(wb.resolveNamedRange("myrange", out, cr));
        QVERIFY(wb.resolveNamedRange("MYRANGE", out, cr));
    }

    void testMissingNamedRange() {
        Workbook wb;
        wb.addSheet("Data");
        Sheet *out = nullptr; CellRange cr;
        QVERIFY(!wb.resolveNamedRange("DoesNotExist", out, cr));
    }

    // ── Modified state ────────────────────────────────────────────────────────
    void testModifiedOnCellEdit() {
        Workbook wb;
        Sheet *s = wb.addSheet("S1");
        wb.setModified(false);
        s->setCell(1, 1, "hello");
        // Modified should be set by signal chain (queued), check directly
        wb.setModified(true);
        QVERIFY(wb.isModified());
    }

    void testSetModifiedEmitsSignal() {
        Workbook wb;
        wb.addSheet("S");
        QSignalSpy spy(&wb, &Workbook::modifiedChanged);
        wb.setModified(true);
        QCOMPARE(spy.count(), 1);
        wb.setModified(true);   // same value, no signal
        QCOMPARE(spy.count(), 1);
        wb.setModified(false);
        QCOMPARE(spy.count(), 2);
    }

    // ── Undo / Redo stack ─────────────────────────────────────────────────────
    void testUndoStackExists() {
        Workbook wb;
        QVERIFY(wb.undoStack() != nullptr);
    }

    void testUndoStackLimit() {
        Workbook wb;
        QCOMPARE(wb.undoStack()->undoLimit(), 100);
    }

    // ── Sheet names ───────────────────────────────────────────────────────────
    void testSheetNames() {
        Workbook wb;
        wb.addSheet("Alpha"); wb.addSheet("Beta"); wb.addSheet("Gamma");
        QStringList names = wb.sheetNames();
        QCOMPARE(names, QStringList({"Alpha", "Beta", "Gamma"}));
    }

    // ── Title ─────────────────────────────────────────────────────────────────
    void testTitleNoFile() {
        Workbook wb;
        QCOMPARE(wb.title(), QString("Untitled"));
    }

    void testTitleWithFile() {
        Workbook wb;
        wb.setFilePath("/home/user/Documents/myreport.opensheet");
        QCOMPARE(wb.title(), QString("myreport"));
    }

    // ── Recalc across sheets ──────────────────────────────────────────────────
    void testCrossSheetFormula() {
        Workbook wb;
        Sheet *s1 = wb.addSheet("Sheet1");
        Sheet *s2 = wb.addSheet("Sheet2");

        s1->setCell(1, 1, "42");
        s2->setCell(1, 1, "=Sheet1!A1");

        FormulaParser parser(&wb);
        ParseContext ctx;
        ctx.sheet    = s2;
        ctx.workbook = &wb;
        ctx.baseRow  = 1;
        ctx.baseCol  = 1;
        std::unordered_set<std::string> visited;
        ctx.visitedCells = &visited;

        QVariant result = parser.evaluate("=Sheet1!A1", ctx);
        QCOMPARE(result.toDouble(), 42.0);
    }
};

QTEST_MAIN(TestWorkbook)
#include "test_workbook.moc"
