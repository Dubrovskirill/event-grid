#include "domain/tag/Tag.h"

#include <QtTest>

#include <stdexcept>

class TagTest final : public QObject
{
    Q_OBJECT

private slots:
    void tagRetainsItsDataAndTrimsName();
    void invalidTagIsRejected();
    void renameRejectsBlankName();
    void colorMustBeValid();
};

void TagTest::tagRetainsItsDataAndTrimsName()
{
    const TagId id = TagId::generate();
    const MapId mapId = MapId::generate();
    const QColor color {137, 180, 250, 200};
    const Tag tag {id, mapId, QStringLiteral("  Work  "), color};

    QVERIFY(tag.id() == id);
    QVERIFY(tag.mapId() == mapId);
    QCOMPARE(tag.name(), QStringLiteral("Work"));
    QCOMPARE(tag.color(), color);
}

void TagTest::invalidTagIsRejected()
{
    const TagId validTagId = TagId::generate();
    const MapId validMapId = MapId::generate();
    const QColor validColor {137, 180, 250};

    QVERIFY_EXCEPTION_THROWN(
        Tag(TagId {QUuid {}}, validMapId, QStringLiteral("Work"), validColor),
        std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(
        Tag(validTagId, MapId {QUuid {}}, QStringLiteral("Work"), validColor),
        std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(
        Tag(validTagId, validMapId, QStringLiteral("  "), validColor),
        std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(
        Tag(validTagId, validMapId, QStringLiteral("Work"), QColor {}),
        std::invalid_argument);
}

void TagTest::renameRejectsBlankName()
{
    Tag tag {TagId::generate(), MapId::generate(), QStringLiteral("Work"), QColor {1, 2, 3}};

    QVERIFY(!tag.rename(QStringLiteral("  ")));
    QCOMPARE(tag.name(), QStringLiteral("Work"));
    QVERIFY(tag.rename(QStringLiteral("  Study ")));
    QCOMPARE(tag.name(), QStringLiteral("Study"));
}

void TagTest::colorMustBeValid()
{
    Tag tag {TagId::generate(), MapId::generate(), QStringLiteral("Work"), QColor {1, 2, 3}};
    const QColor initialColor = tag.color();

    QVERIFY(!tag.setColor(QColor {}));
    QCOMPARE(tag.color(), initialColor);
    QVERIFY(tag.setColor(QColor {4, 5, 6, 128}));
    QCOMPARE(tag.color(), QColor(4, 5, 6, 128));
}

QTEST_APPLESS_MAIN(TagTest)

#include "tag_test.moc"
