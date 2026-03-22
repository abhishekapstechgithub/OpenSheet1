#include <QtTest>
#include "../office/engine/dependency_graph.h"

using namespace OpenSheet;

class TestDependencyGraph : public QObject {
    Q_OBJECT

    static CellKey K(int r, int c, int s = 0) { return {s, r, c}; }

private slots:

    void testAddEdge() {
        DependencyGraph g;
        g.addEdge(K(1,1), K(2,2));   // cell(1,1) depends on cell(2,2)
        QVERIFY(g.hasDependents(K(2,2)));
        QVERIFY(!g.hasDependents(K(1,1)));
    }

    void testDependentsOf() {
        DependencyGraph g;
        g.addEdge(K(1,1), K(5,5));
        g.addEdge(K(2,2), K(5,5));
        g.addEdge(K(3,3), K(5,5));
        auto deps = g.dependentsOf(K(5,5));
        QCOMPARE(deps.size(), 3);
    }

    void testTransitiveDependents() {
        DependencyGraph g;
        // A -> B -> C  (A depends on B, B depends on C)
        g.addEdge(K(1,1), K(2,1));   // A depends on B
        g.addEdge(K(2,1), K(3,1));   // B depends on C
        auto deps = g.dependentsOf(K(3,1));  // who depends on C?
        // Should include B (direct) and A (transitive)
        QCOMPARE(deps.size(), 2);
    }

    void testRemoveEdges() {
        DependencyGraph g;
        g.addEdge(K(1,1), K(2,2));
        g.addEdge(K(1,1), K(3,3));
        g.removeEdgesFor(K(1,1));
        // After removing, neither K(2,2) nor K(3,3) should have K(1,1) as dependent
        auto d1 = g.dependentsOf(K(2,2));
        auto d2 = g.dependentsOf(K(3,3));
        QCOMPARE(d1.size(), 0);
        QCOMPARE(d2.size(), 0);
    }

    void testTopoSortSimple() {
        DependencyGraph g;
        // C3 = A1 + B2  →  A1 and B2 must come before C3
        g.addEdge(K(3,3), K(1,1));
        g.addEdge(K(3,3), K(2,2));

        QVector<CellKey> order;
        bool ok = g.topoSort({K(3,3)}, order);
        QVERIFY(ok);
        // C3 should appear after A1 and B2 in order
        int idxC3 = order.indexOf(K(3,3));
        int idxA1 = order.indexOf(K(1,1));
        int idxB2 = order.indexOf(K(2,2));
        // A1 and B2 come before C3
        if (idxA1 >= 0) QVERIFY(idxA1 < idxC3);
        if (idxB2 >= 0) QVERIFY(idxB2 < idxC3);
    }

    void testTopoSortChain() {
        DependencyGraph g;
        // D4 depends on C3, C3 depends on B2, B2 depends on A1
        g.addEdge(K(2,2), K(1,1));  // B2 -> A1
        g.addEdge(K(3,3), K(2,2));  // C3 -> B2
        g.addEdge(K(4,4), K(3,3));  // D4 -> C3

        QVector<CellKey> order;
        bool ok = g.topoSort({K(4,4), K(3,3), K(2,2)}, order);
        QVERIFY(ok);
        // Order should be: A1, B2, C3, D4  (or variants that respect dependencies)
        int i1 = order.indexOf(K(1,1));
        int i2 = order.indexOf(K(2,2));
        int i3 = order.indexOf(K(3,3));
        int i4 = order.indexOf(K(4,4));
        if (i1 >= 0 && i2 >= 0) QVERIFY(i1 <= i2);
        if (i2 >= 0 && i3 >= 0) QVERIFY(i2 <= i3);
        if (i3 >= 0 && i4 >= 0) QVERIFY(i3 <= i4);
    }

    void testCircularDetection() {
        DependencyGraph g;
        // A1 depends on B2, B2 depends on A1  →  cycle
        g.addEdge(K(1,1), K(2,2));
        g.addEdge(K(2,2), K(1,1));

        QVector<CellKey> order;
        bool ok = g.topoSort({K(1,1), K(2,2)}, order);
        QVERIFY(!ok);  // should detect cycle
    }

    void testNoCycleInDiamond() {
        DependencyGraph g;
        // Diamond: D depends on B and C, both depend on A
        g.addEdge(K(2,1), K(1,1));  // B -> A
        g.addEdge(K(3,1), K(1,1));  // C -> A
        g.addEdge(K(4,1), K(2,1));  // D -> B
        g.addEdge(K(4,1), K(3,1));  // D -> C

        QVector<CellKey> order;
        bool ok = g.topoSort({K(4,1), K(3,1), K(2,1)}, order);
        QVERIFY(ok);  // diamond is NOT a cycle
    }

    void testClear() {
        DependencyGraph g;
        g.addEdge(K(1,1), K(2,2));
        g.addEdge(K(3,3), K(4,4));
        g.clear();
        QVERIFY(!g.hasDependents(K(2,2)));
        QVERIFY(!g.hasDependents(K(4,4)));
    }

    void testEmptyGraph() {
        DependencyGraph g;
        auto deps = g.dependentsOf(K(1,1));
        QCOMPARE(deps.size(), 0);

        QVector<CellKey> order;
        bool ok = g.topoSort({}, order);
        QVERIFY(ok);
        QCOMPARE(order.size(), 0);
    }

    void testMultiSheetEdges() {
        DependencyGraph g;
        // Cell on sheet 1 depends on cell on sheet 0
        g.addEdge(K(1,1,1), K(1,1,0));
        QVERIFY(g.hasDependents(K(1,1,0)));
        QVERIFY(!g.hasDependents(K(1,1,1)));
    }
};

QTEST_MAIN(TestDependencyGraph)
#include "test_dependency_graph.moc"
