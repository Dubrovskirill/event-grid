#include "Cell.h"

#include <QSet>

#include <stdexcept>
#include <utility>

Cell Cell::createNew(CellId id, MapId mapId, GridCoordinate coordinate, QDateTime now)
{
    return Cell {std::move(id),
                 std::move(mapId),
                 coordinate,
                 CellState::Empty,
                 {},
                 {},
                 {},
                 now,
                 now};
}

Cell::Cell(CellId id,
           MapId mapId,
           GridCoordinate coordinate,
           CellState state,
           QVector<TagId> tagIds,
           QString note,
           QDateTime eventDate,
           QDateTime createdAt,
           QDateTime updatedAt)
    : m_id(std::move(id))
    , m_mapId(std::move(mapId))
    , m_coordinate(coordinate)
    , m_state(state)
    , m_tagIds(std::move(tagIds))
    , m_note(std::move(note))
    , m_eventDate(std::move(eventDate))
    , m_createdAt(std::move(createdAt))
    , m_updatedAt(std::move(updatedAt))
{
    validate(m_id, m_mapId, m_tagIds, m_createdAt, m_updatedAt);
}

CellId Cell::id() const noexcept { return m_id; }
MapId Cell::mapId() const noexcept { return m_mapId; }
GridCoordinate Cell::coordinate() const noexcept { return m_coordinate; }
CellState Cell::state() const noexcept { return m_state; }
bool Cell::isCompleted() const noexcept { return m_state == CellState::Completed; }
bool Cell::isLocked() const noexcept { return m_state == CellState::Locked; }
const QString &Cell::note() const noexcept { return m_note; }
const QDateTime &Cell::eventDate() const noexcept { return m_eventDate; }
const QVector<TagId> &Cell::tagIds() const noexcept { return m_tagIds; }

bool Cell::complete(QDateTime changedAt)
{
    if (isLocked() || !canApplyAt(changedAt)) {
        return false;
    }
    if (isCompleted()) {
        return true;
    }

    m_state = CellState::Completed;
    touch(std::move(changedAt));
    return true;
}

bool Cell::reset(QDateTime changedAt)
{
    if (isLocked() || !canApplyAt(changedAt)) {
        return false;
    }
    if (m_state == CellState::Empty) {
        return true;
    }

    m_state = CellState::Empty;
    touch(std::move(changedAt));
    return true;
}

bool Cell::lock(QDateTime changedAt)
{
    if (!canApplyAt(changedAt)) {
        return false;
    }
    if (isLocked()) {
        return true;
    }

    m_state = CellState::Locked;
    touch(std::move(changedAt));
    return true;
}

bool Cell::unlock(QDateTime changedAt)
{
    if (!canApplyAt(changedAt)) {
        return false;
    }
    if (!isLocked()) {
        return true;
    }

    m_state = CellState::Empty;
    touch(std::move(changedAt));
    return true;
}

bool Cell::setNote(QString note, QDateTime changedAt)
{
    if (!canApplyAt(changedAt)) {
        return false;
    }
    if (note == m_note) {
        return true;
    }

    m_note = std::move(note);
    touch(std::move(changedAt));
    return true;
}

bool Cell::setEventDate(QDateTime eventDate, QDateTime changedAt)
{
    if (!canApplyAt(changedAt)) {
        return false;
    }
    if (eventDate == m_eventDate) {
        return true;
    }

    m_eventDate = std::move(eventDate);
    touch(std::move(changedAt));
    return true;
}

bool Cell::addTag(TagId tagId, QDateTime changedAt)
{
    if (tagId.isNull() || !canApplyAt(changedAt)) {
        return false;
    }
    if (m_tagIds.contains(tagId)) {
        return true;
    }

    m_tagIds.append(std::move(tagId));
    touch(std::move(changedAt));
    return true;
}

bool Cell::removeTag(TagId tagId, QDateTime changedAt)
{
    if (tagId.isNull() || !canApplyAt(changedAt)) {
        return false;
    }

    const int index = m_tagIds.indexOf(tagId);
    if (index < 0) {
        return true;
    }

    m_tagIds.removeAt(index);
    touch(std::move(changedAt));
    return true;
}

const QDateTime &Cell::createdAt() const noexcept { return m_createdAt; }
const QDateTime &Cell::updatedAt() const noexcept { return m_updatedAt; }

bool Cell::canApplyAt(const QDateTime &changedAt) const noexcept
{
    return changedAt.isValid() && changedAt >= m_updatedAt;
}

void Cell::touch(QDateTime changedAt) noexcept
{
    m_updatedAt = std::move(changedAt);
}

void Cell::validate(CellId id,
                    MapId mapId,
                    const QVector<TagId> &tagIds,
                    const QDateTime &createdAt,
                    const QDateTime &updatedAt)
{
    if (id.isNull() || mapId.isNull()) {
        throw std::invalid_argument("Cell and map identifiers must not be null");
    }
    if (!createdAt.isValid() || !updatedAt.isValid() || updatedAt < createdAt) {
        throw std::invalid_argument("Cell timestamps are invalid");
    }

    QSet<TagId> uniqueTags;
    for (const TagId &tagId : tagIds) {
        if (tagId.isNull() || uniqueTags.contains(tagId)) {
            throw std::invalid_argument("Cell tags must be non-null and unique");
        }
        uniqueTags.insert(tagId);
    }
}
