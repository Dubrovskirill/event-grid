#include "domain/common/GridCoordinate.h"
#include "domain/common/GridRect.h"

#include <QHash>
#include <QtTest>

class GridGeometryTest final : public QObject
{
    Q_OBJECT

private slots:
    void coordinatesCompareByValue();
    void coordinateCanBeUsedAsQHashKey();
    void rectangleContainsItsInclusiveBounds();
    void invalidRectangleIsEmpty();
    void rectangleReportsInclusiveDimensions();
};

void GridGeometryTest::coordinatesCompareByValue()
{
    const GridCoordinate origin {0, 0};
    const GridCoordinate sameOrigin {0, 0};
    const GridCoordinate other {0, 1};

    QVERIFY(origin == sameOrigin);
    QVERIFY(origin != other);
}

void GridGeometryTest::coordinateCanBeUsedAsQHashKey()
{
    QHash<GridCoordinate, QString> labels;
    labels.insert({-12, 42}, QStringLiteral("saved cell"));

    QCOMPARE(labels.value({-12, 42}), QStringLiteral("saved cell"));
}

void GridGeometryTest::rectangleContainsItsInclusiveBounds()
{
    const GridRect area {2, 4, 6, 8};

    QVERIFY(area.contains({2, 4}));
    QVERIFY(area.contains({6, 8}));
    QVERIFY(!area.contains({1, 4}));
    QVERIFY(!area.contains({2, 9}));
}

void GridGeometryTest::invalidRectangleIsEmpty()
{
    const GridRect area {6, 8, 2, 4};

    QVERIFY(!area.isValid());
    QVERIFY(!area.contains({4, 6}));
    QCOMPARE(area.width(), qint64 {0});
    QCOMPARE(area.height(), qint64 {0});
}

void GridGeometryTest::rectangleReportsInclusiveDimensions()
{
    const GridRect area {-2, 10, 2, 12};

    QCOMPARE(area.width(), qint64 {3});
    QCOMPARE(area.height(), qint64 {5});
}

QTEST_APPLESS_MAIN(GridGeometryTest)

#include "grid_geometry_test.moc"
