#include <QtTest>
#include "sheet.h"
#include "cell_range.h"

using namespace OpenSheet;

class TestSheet : public QObject {
    Q_OBJECT
private slots:
    void testSetAndGetCell() {
        Sheet s("Sheet1");
        s.setCell(1, 1, "Hello");
        QCOMPARE(s.cell(1,1).raw(), QString("Hello"));
        QVERIFY(s.hasCell(1,1));
        QVERIFY(!s.hasCell(2,2));
    }

    void testClearCell() {
        Sheet s("Sheet1");
        s.setCell(3, 3, "data");
        QVERIFY(s.hasCell(3,3));
        s.clearCell(3, 3);
        QVERIFY(!s.hasCell(3,3));
    }

    void testEmptyRawReturnsNoCell() {
        Sheet s("Sheet1");
        s.setCell(1, 1, "");
        QVERIFY(!s.hasCell(1,1));
    }

    void testMaxRowCol() {
        Sheet s("Sheet1");
        s.setCell(5, 3, "val");
        s.setCell(2, 8, "val");
        QCOMPARE(s.maxRow(), 5);
        QCOMPARE(s.maxCol(), 8);
    }

    void testClearRange() {
        Sheet s("Sheet1");
        for (int r=1;r<=5;++r) s.setCell(r, 1, "x");
        s.clearRange(CellRange(2,1,4,1));
        QVERIFY( s.hasCell(1,1));
        QVERIFY(!s.hasCell(2,1));
        QVERIFY(!s.hasCell(3,1));
        QVERIFY(!s.hasCell(4,1));
        QVERIFY( s.hasCell(5,1));
    }

    void testInsertRow() {
        Sheet s("Sheet1");
        s.setCell(1,1,"A"); s.setCell(2,1,"B"); s.setCell(3,1,"C");
        s.insertRow(2);
        QCOMPARE(s.cell(1,1).raw(), QString("A"));
        QVERIFY(!s.hasCell(2,1));   // inserted blank row
        QCOMPARE(s.cell(3,1).raw(), QString("B"));
        QCOMPARE(s.cell(4,1).raw(), QString("C"));
    }

    void testDeleteRow() {
        Sheet s("Sheet1");
        s.setCell(1,1,"A"); s.setCell(2,1,"B"); s.setCell(3,1,"C");
        s.deleteRow(2);
        QCOMPARE(s.cell(1,1).raw(), QString("A"));
        QCOMPARE(s.cell(2,1).raw(), QString("C"));
        QVERIFY(!s.hasCell(3,1));
    }

    void testInsertCol() {
        Sheet s("Sheet1");
        s.setCell(1,1,"A"); s.setCell(1,2,"B"); s.setCell(1,3,"C");
        s.insertCol(2);
        QCOMPARE(s.cell(1,1).raw(), QString("A"));
        QVERIFY(!s.hasCell(1,2));
        QCOMPARE(s.cell(1,3).raw(), QString("B"));
        QCOMPARE(s.cell(1,4).raw(), QString("C"));
    }

    void testSortAscending() {
        Sheet s("Sheet1");
        s.setCell(1,1,"Banana");
        s.setCell(2,1,"Apple");
        s.setCell(3,1,"Cherry");
        s.sortRange(CellRange(1,1,3,1), 1, true);
        QCOMPARE(s.cell(1,1).raw(), QString("Apple"));
        QCOMPARE(s.cell(2,1).raw(), QString("Banana"));
        QCOMPARE(s.cell(3,1).raw(), QString("Cherry"));
    }

    void testSortDescending() {
        Sheet s("Sheet1");
        s.setCell(1,1,"30"); s.setCell(2,1,"10"); s.setCell(3,1,"20");
        s.sortRange(CellRange(1,1,3,1), 1, false);
        QCOMPARE(s.cell(1,1).raw(), QString("30"));
        QCOMPARE(s.cell(2,1).raw(), QString("20"));
        QCOMPARE(s.cell(3,1).raw(), QString("10"));
    }

    void testRowColProps() {
        Sheet s("Sheet1");
        s.rowProps(5).height = 40.0;
        s.colProps(3).width  = 120.0;
        QCOMPARE(s.rowProps(5).height, 40.0);
        QCOMPARE(s.colProps(3).width,  120.0);
    }

    void testFreezeRowCol() {
        Sheet s("Sheet1");
        s.setFreezeRow(2);
        s.setFreezeCol(1);
        QCOMPARE(s.freezeRow(), 2);
        QCOMPARE(s.freezeCol(), 1);
    }

    void testConditionalFormatting() {
        Sheet s("Sheet1");
        s.setCell(1,1,"100");
        ConditionalRule rule;
        rule.type   = ConditionalRule::Type::GreaterThan;
        rule.value1 = 50;
        rule.range  = CellRange(1,1,5,1);
        rule.applyFormat.bold = true;
        rule.applyFormat.foreground = Qt::red;
        s.addConditionalRule(rule);
        auto fmt = s.evalConditionalFormat(1, 1);
        QVERIFY(fmt.has_value());
        QVERIFY(fmt->bold);
        QCOMPARE(fmt->foreground, QColor(Qt::red));
    }

    void testCopyRange() {
        Sheet s("Sheet1");
        s.setCell(1,1,"X"); s.setCell(1,2,"Y");
        s.setCell(2,1,"Z"); s.setCell(2,2,"W");
        s.copyRange(CellRange(1,1,2,2), CellAddress{4,4});
        QCOMPARE(s.cell(4,4).raw(), QString("X"));
        QCOMPARE(s.cell(4,5).raw(), QString("Y"));
        QCOMPARE(s.cell(5,4).raw(), QString("Z"));
        QCOMPARE(s.cell(5,5).raw(), QString("W"));
    }

    void testNameChange() {
        Sheet s("OldName");
        QCOMPARE(s.name(), QString("OldName"));
        QSignalSpy spy(&s, &Sheet::nameChanged);
        s.setName("NewName");
        QCOMPARE(s.name(), QString("NewName"));
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(TestSheet)
#include "test_sheet.moc"
