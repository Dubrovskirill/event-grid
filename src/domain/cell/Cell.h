#pragma once

#include "domain/common/EntityId.h"
#include "domain/common/GridCoordinate.h"
#include "domain/map/MapTypes.h"

#include <QDateTime>
#include <QString>
#include <QVector>

class Cell
{
public:
    static Cell createNew(CellId id,
                          MapId mapId,
                          GridCoordinate coordinate,
                          QDateTime now = QDateTime::currentDateTimeUtc());

    Cell(CellId id,
         MapId mapId,
         GridCoordinate coordinate,
         CellState state,
         QVector<TagId> tagIds,
         QString note,
         QDateTime eventDate,
         QDateTime createdAt,
         QDateTime updatedAt);

    CellId id() const noexcept;
    MapId mapId() const noexcept;
    GridCoordinate coordinate() const noexcept;
    CellState state() const noexcept;
    bool isCompleted() const noexcept;
    bool isLocked() const noexcept;
    const QString &note() const noexcept;
    const QDateTime &eventDate() const noexcept;
    const QVector<TagId> &tagIds() const noexcept;

    bool complete(QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool reset(QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool lock(QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool unlock(QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool setNote(QString note, QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool setEventDate(QDateTime eventDate,
                      QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool addTag(TagId tagId, QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool removeTag(TagId tagId, QDateTime changedAt = QDateTime::currentDateTimeUtc());

    const QDateTime &createdAt() const noexcept;
    const QDateTime &updatedAt() const noexcept;

private:
    bool canApplyAt(const QDateTime &changedAt) const noexcept;
    void touch(QDateTime changedAt) noexcept;
    static void validate(CellId id,
                         MapId mapId,
                         const QVector<TagId> &tagIds,
                         const QDateTime &createdAt,
                         const QDateTime &updatedAt);

    CellId m_id;
    MapId m_mapId;
    GridCoordinate m_coordinate;
    CellState m_state = CellState::Empty;
    QVector<TagId> m_tagIds;
    QString m_note;
    QDateTime m_eventDate;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};
