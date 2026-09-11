#include "Map.h"

#include <stdexcept>
#include <utility>

Map Map::createNew(MapId id,
                   QString name,
                   MapConfiguration configuration,
                   QDateTime now)
{
    return Map {id, std::move(name), std::move(configuration), now, now};
}

Map::Map(MapId id,
         QString name,
         MapConfiguration configuration,
         QDateTime createdAt,
         QDateTime updatedAt)
    : m_id(std::move(id))
    , m_name(normalizedName(std::move(name)))
    , m_configuration(std::move(configuration))
    , m_createdAt(std::move(createdAt))
    , m_updatedAt(std::move(updatedAt))
{
    validate(m_id, m_name, m_configuration, m_createdAt, m_updatedAt);
}

MapId Map::id() const noexcept
{
    return m_id;
}

const QString &Map::name() const noexcept
{
    return m_name;
}

const MapConfiguration &Map::configuration() const noexcept
{
    return m_configuration;
}

GridType Map::gridType() const noexcept
{
    return m_configuration.gridType;
}

MapMode Map::mode() const noexcept
{
    return m_configuration.mode;
}

const QDateTime &Map::createdAt() const noexcept
{
    return m_createdAt;
}

const QDateTime &Map::updatedAt() const noexcept
{
    return m_updatedAt;
}

bool Map::rename(QString name, QDateTime changedAt)
{
    name = normalizedName(std::move(name));
    if (name.isEmpty() || !changedAt.isValid() || changedAt < m_updatedAt) {
        return false;
    }

    if (name == m_name) {
        return true;
    }

    m_name = std::move(name);
    m_updatedAt = std::move(changedAt);
    return true;
}

bool Map::updateConfiguration(MapConfiguration configuration, QDateTime changedAt)
{
    if (!configuration.isValid() || !changedAt.isValid() || changedAt < m_updatedAt) {
        return false;
    }

    m_configuration = std::move(configuration);
    m_updatedAt = std::move(changedAt);
    return true;
}

QString Map::normalizedName(QString name)
{
    return name.trimmed();
}

void Map::validate(MapId id,
                   const QString &name,
                   const MapConfiguration &configuration,
                   const QDateTime &createdAt,
                   const QDateTime &updatedAt)
{
    if (id.isNull()) {
        throw std::invalid_argument("Map id must not be null");
    }
    if (name.isEmpty()) {
        throw std::invalid_argument("Map name must not be empty");
    }
    if (!configuration.isValid()) {
        throw std::invalid_argument("Map configuration is invalid");
    }
    if (!createdAt.isValid() || !updatedAt.isValid() || updatedAt < createdAt) {
        throw std::invalid_argument("Map timestamps are invalid");
    }
}
