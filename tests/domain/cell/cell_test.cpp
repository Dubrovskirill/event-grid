#include "domain/cell/Cell.h"

#include <QtTest>

namespace {

QDateTime timestamp(int seconds)
{
    return QDateTime::fromSecsSinceEpoch(seconds, Qt::UTC);
}

Cell newCell(int createdAt = 1)
{
    return Cell::createNew(CellId::generate(),
                           MapId::generate(),
                           {-8, 12},
                           timestamp(createdAt));
}

} // namespace

class CellTest final : public QObject
{
    Q_OBJECT

private slots:
    void newCellHasEmptyState();
    void completeAndResetUpdateStateAndTimestamp();
    void lockedCellRejectsCompletionAndReset();
    void unlockReturnsCellToEmptyState();
    void tagsAreUniqueAndCanBeRemoved();
    void contentChangesUpdateTimestamp();
    void changesRejectPastTimestamps();
};

void CellTest::newCellHasEmptyState()
{
    const Cell cell = newCell();

    QCOMPARE(cell.coordinate(), GridCoordinate({-8, 12}));
    QCOMPARE(cell.state(), CellState::Empty);
    QVERIFY(cell.tagIds().isEmpty());
    QVERIFY(cell.note().isEmpty());
    QVERIFY(!cell.eventDate().isValid());
    QCOMPARE(cell.createdAt(), timestamp(1));
    QCOMPARE(cell.updatedAt(), timestamp(1));
}

void CellTest::completeAndResetUpdateStateAndTimestamp()
{
    Cell cell = newCell();

    QVERIFY(cell.complete(timestamp(2)));
    QVERIFY(cell.isCompleted());
    QCOMPARE(cell.updatedAt(), timestamp(2));

    QVERIFY(cell.reset(timestamp(3)));
    QCOMPARE(cell.state(), CellState::Empty);
    QCOMPARE(cell.updatedAt(), timestamp(3));
}

void CellTest::lockedCellRejectsCompletionAndReset()
{
    Cell cell = newCell();
    QVERIFY(cell.lock(timestamp(2)));

    QVERIFY(!cell.complete(timestamp(3)));
    QVERIFY(!cell.reset(timestamp(3)));
    QVERIFY(cell.isLocked());
    QCOMPARE(cell.updatedAt(), timestamp(2));
}

void CellTest::unlockReturnsCellToEmptyState()
{
    Cell cell = newCell();
    QVERIFY(cell.complete(timestamp(2)));
    QVERIFY(cell.lock(timestamp(3)));

    QVERIFY(cell.unlock(timestamp(4)));
    QCOMPARE(cell.state(), CellState::Empty);
    QCOMPARE(cell.updatedAt(), timestamp(4));
}

void CellTest::tagsAreUniqueAndCanBeRemoved()
{
    Cell cell = newCell();
    const TagId tagId = TagId::generate();

    QVERIFY(cell.addTag(tagId, timestamp(2)));
    QVERIFY(cell.addTag(tagId, timestamp(3)));
    QCOMPARE(cell.tagIds().size(), 1);
    QCOMPARE(cell.updatedAt(), timestamp(2));

    QVERIFY(cell.removeTag(tagId, timestamp(4)));
    QVERIFY(cell.tagIds().isEmpty());
    QCOMPARE(cell.updatedAt(), timestamp(4));
}

void CellTest::contentChangesUpdateTimestamp()
{
    Cell cell = newCell();
    const QDateTime eventDate = timestamp(100);

    QVERIFY(cell.setNote(QStringLiteral("A note"), timestamp(2)));
    QVERIFY(cell.setEventDate(eventDate, timestamp(3)));
    QCOMPARE(cell.note(), QStringLiteral("A note"));
    QCOMPARE(cell.eventDate(), eventDate);
    QCOMPARE(cell.updatedAt(), timestamp(3));
}

void CellTest::changesRejectPastTimestamps()
{
    Cell cell = newCell(2);

    QVERIFY(!cell.setNote(QStringLiteral("Ignored"), timestamp(1)));
    QVERIFY(!cell.addTag(TagId::generate(), timestamp(1)));
    QVERIFY(cell.note().isEmpty());
    QVERIFY(cell.tagIds().isEmpty());
}

QTEST_APPLESS_MAIN(CellTest)

#include "cell_test.moc"
