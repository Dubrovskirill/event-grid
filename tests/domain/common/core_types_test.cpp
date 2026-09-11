#include "domain/common/EntityId.h"
#include "domain/common/GridDimensions.h"

#include <QHash>
#include <QtTest>

class CoreTypesTest final : public QObject
{
    Q_OBJECT

private slots:
    void generatedIdentifierIsNotNull();
    void identifiersCompareByUuidValue();
    void identifierCanBeUsedAsQHashKey();
    void dimensionsContainCoordinatesFromZero();
    void dimensionsRejectInvalidSizes();
};

void CoreTypesTest::generatedIdentifierIsNotNull()
{
    const MapId id = MapId::generate();

    QVERIFY(!id.isNull());
}

void CoreTypesTest::identifiersCompareByUuidValue()
{
    const QUuid uuid = QUuid::createUuid();
    const MapId first {uuid};
    const MapId second {uuid};
    const MapId other = MapId::generate();

    QVERIFY(first == second);
    QVERIFY(first != other);
}

void CoreTypesTest::identifierCanBeUsedAsQHashKey()
{
    const MapId id = MapId::generate();
    QHash<MapId, QString> names;
    names.insert(id, QStringLiteral("Life Calendar"));

    QCOMPARE(names.value(id), QStringLiteral("Life Calendar"));
}

void CoreTypesTest::dimensionsContainCoordinatesFromZero()
{
    const GridDimensions dimensions {3, 5};

    QVERIFY(dimensions.isValid());
    QVERIFY(dimensions.contains({0, 0}));
    QVERIFY(dimensions.contains({2, 4}));
    QVERIFY(!dimensions.contains({3, 4}));
    QVERIFY(!dimensions.contains({2, 5}));
    QVERIFY(!dimensions.contains({-1, 0}));
}

void CoreTypesTest::dimensionsRejectInvalidSizes()
{
    const GridDimensions noRows {0, 5};
    const GridDimensions noColumns {5, 0};
    const GridDimensions negativeRows {-1, 5};

    QVERIFY(!noRows.isValid());
    QVERIFY(!noColumns.isValid());
    QVERIFY(!negativeRows.isValid());
}

QTEST_APPLESS_MAIN(CoreTypesTest)

#include "core_types_test.moc"
