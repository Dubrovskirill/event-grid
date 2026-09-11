#pragma once

#include <QHash>
#include <QUuid>

#include <utility>

// Tag types make identifiers for different domain entities incompatible.
template<typename Tag>
class EntityId final
{
public:
    static EntityId generate()
    {
        return EntityId {QUuid::createUuid()};
    }

    explicit EntityId(QUuid value) noexcept
        : m_value(std::move(value))
    {
    }

    const QUuid &value() const noexcept
    {
        return m_value;
    }

    bool isNull() const noexcept
    {
        return m_value.isNull();
    }

    bool operator==(const EntityId &other) const noexcept
    {
        return m_value == other.m_value;
    }

    bool operator!=(const EntityId &other) const noexcept
    {
        return !(*this == other);
    }

private:
    QUuid m_value;
};

template<typename Tag>
inline uint qHash(const EntityId<Tag> &id, uint seed = 0) noexcept
{
    return ::qHash(id.value(), seed);
}

struct MapIdTag {};
struct CellIdTag {};
struct TagIdTag {};
struct RegionIdTag {};

using MapId = EntityId<MapIdTag>;
using CellId = EntityId<CellIdTag>;
using TagId = EntityId<TagIdTag>;
using RegionId = EntityId<RegionIdTag>;
