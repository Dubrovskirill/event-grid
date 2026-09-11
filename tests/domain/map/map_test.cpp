#include "domain/map/Map.h"

#include <QtTest>

#include <stdexcept>

namespace {

MapConfiguration fixedConfiguration()
{
    MapConfiguration configuration;
    configuration.dimensions = {52, 80};
    configuration.cellUnit = CellUnit::Week;
    return configuration;
}

QDateTime timestamp(int seconds)
{
    return QDateTime::fromSecsSinceEpoch(seconds, Qt::UTC);
}

} // namespace

class MapTest final : public QObject
{
    Q_OBJECT

private slots:
    void newMapRetainsItsDomainData();
    void mapNameIsTrimmed();
    void invalidMapIsRejected();
    void renameUpdatesTimestamp();
    void renameRejectsInvalidNameAndPastTimestamp();
    void configurationUpdateRejectsInvalidConfiguration();
};

void MapTest::newMapRetainsItsDomainData()
{
    const MapId id = MapId::generate();
    const QDateTime now = timestamp(1'700'000'000);
    const Map map = Map::createNew(id, QStringLiteral("Life Calendar"), fixedConfiguration(), now);

    QCOMPARE(map.id(), id);
    QCOMPARE(map.name(), QStringLiteral("Life Calendar"));
    QCOMPARE(map.gridType(), GridType::FixedRectangle);
    QCOMPARE(map.mode(), MapMode::Free);
    QCOMPARE(map.createdAt(), now);
    QCOMPARE(map.updatedAt(), now);
}

void MapTest::mapNameIsTrimmed()
{
    const Map map = Map::createNew(MapId::generate(),
                                   QStringLiteral("  Reading  "),
                                   fixedConfiguration(),
                                   timestamp(1));

    QCOMPARE(map.name(), QStringLiteral("Reading"));
}

void MapTest::invalidMapIsRejected()
{
    QVERIFY_EXCEPTION_THROWN(
        Map::createNew(MapId {QUuid {}}, QStringLiteral("Valid"), fixedConfiguration(), timestamp(1)),
        std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(
        Map::createNew(MapId::generate(), QStringLiteral("   "), fixedConfiguration(), timestamp(1)),
        std::invalid_argument);
}

void MapTest::renameUpdatesTimestamp()
{
    Map map = Map::createNew(MapId::generate(), QStringLiteral("Old"), fixedConfiguration(), timestamp(1));

    QVERIFY(map.rename(QStringLiteral("New"), timestamp(2)));
    QCOMPARE(map.name(), QStringLiteral("New"));
    QCOMPARE(map.updatedAt(), timestamp(2));
    QCOMPARE(map.createdAt(), timestamp(1));
}

void MapTest::renameRejectsInvalidNameAndPastTimestamp()
{
    Map map = Map::createNew(MapId::generate(), QStringLiteral("Map"), fixedConfiguration(), timestamp(2));

    QVERIFY(!map.rename(QStringLiteral("  "), timestamp(3)));
    QVERIFY(!map.rename(QStringLiteral("Older"), timestamp(1)));
    QCOMPARE(map.name(), QStringLiteral("Map"));
    QCOMPARE(map.updatedAt(), timestamp(2));
}

void MapTest::configurationUpdateRejectsInvalidConfiguration()
{
    Map map = Map::createNew(MapId::generate(), QStringLiteral("Map"), fixedConfiguration(), timestamp(1));
    MapConfiguration invalidConfiguration;

    QVERIFY(!map.updateConfiguration(invalidConfiguration, timestamp(2)));
    QCOMPARE(map.configuration().dimensions.rows, qint64 {52});
    QCOMPARE(map.updatedAt(), timestamp(1));
}

QTEST_APPLESS_MAIN(MapTest)

#include "map_test.moc"
