#include "domain/map/MapConfiguration.h"

#include <QtTest>

class MapConfigurationTest final : public QObject
{
    Q_OBJECT

private slots:
    void fixedGridRequiresValidDimensions();
    void infiniteGridDoesNotRequireDimensions();
    void unitSizeMustBePositive();
    void initialCompletedCellsCannotBeNegative();
};

void MapConfigurationTest::fixedGridRequiresValidDimensions()
{
    MapConfiguration configuration;

    QVERIFY(!configuration.isValid());

    configuration.dimensions = {52, 80};
    QVERIFY(configuration.isValid());
    QVERIFY(!configuration.isInfinite());
}

void MapConfigurationTest::infiniteGridDoesNotRequireDimensions()
{
    MapConfiguration configuration;
    configuration.gridType = GridType::Infinite;

    QVERIFY(configuration.isValid());
    QVERIFY(configuration.isInfinite());
}

void MapConfigurationTest::unitSizeMustBePositive()
{
    MapConfiguration configuration;
    configuration.dimensions = {10, 10};
    configuration.unitSize = 0;

    QVERIFY(!configuration.isValid());
}

void MapConfigurationTest::initialCompletedCellsCannotBeNegative()
{
    MapConfiguration configuration;
    configuration.dimensions = {10, 10};
    configuration.initialCompletedCells = -1;

    QVERIFY(!configuration.isValid());
}

QTEST_APPLESS_MAIN(MapConfigurationTest)

#include "map_configuration_test.moc"
