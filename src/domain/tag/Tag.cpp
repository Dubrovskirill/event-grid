#include "Tag.h"

#include <stdexcept>
#include <utility>

Tag::Tag(TagId id, MapId mapId, QString name, QColor color)
    : m_id(std::move(id))
    , m_mapId(std::move(mapId))
    , m_name(normalizedName(std::move(name)))
    , m_color(std::move(color))
{
    validate(m_id, m_mapId, m_name, m_color);
}

TagId Tag::id() const noexcept { return m_id; }
MapId Tag::mapId() const noexcept { return m_mapId; }
const QString &Tag::name() const noexcept { return m_name; }
const QColor &Tag::color() const noexcept { return m_color; }

bool Tag::rename(QString name)
{
    name = normalizedName(std::move(name));
    if (name.isEmpty()) {
        return false;
    }

    m_name = std::move(name);
    return true;
}

bool Tag::setColor(QColor color)
{
    if (!color.isValid()) {
        return false;
    }

    m_color = std::move(color);
    return true;
}

QString Tag::normalizedName(QString name)
{
    return name.trimmed();
}

void Tag::validate(TagId id, MapId mapId, const QString &name, const QColor &color)
{
    if (id.isNull() || mapId.isNull()) {
        throw std::invalid_argument("Tag and map identifiers must not be null");
    }
    if (name.isEmpty()) {
        throw std::invalid_argument("Tag name must not be empty");
    }
    if (!color.isValid()) {
        throw std::invalid_argument("Tag color must be valid");
    }
}
