#pragma once

#include "domain/common/EntityId.h"

#include <QColor>
#include <QString>

class Tag
{
public:
    Tag(TagId id, MapId mapId, QString name, QColor color);

    TagId id() const noexcept;
    MapId mapId() const noexcept;
    const QString &name() const noexcept;
    const QColor &color() const noexcept;

    bool rename(QString name);
    bool setColor(QColor color);

private:
    static QString normalizedName(QString name);
    static void validate(TagId id, MapId mapId, const QString &name, const QColor &color);

    TagId m_id;
    MapId m_mapId;
    QString m_name;
    QColor m_color;
};
