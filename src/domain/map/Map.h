#pragma once

#include "domain/common/EntityId.h"
#include "MapConfiguration.h"

#include <QDateTime>
#include <QString>

class Map
{
public:
    static Map createNew(MapId id,
                         QString name,
                         MapConfiguration configuration,
                         QDateTime now = QDateTime::currentDateTimeUtc());

    Map(MapId id,
        QString name,
        MapConfiguration configuration,
        QDateTime createdAt,
        QDateTime updatedAt);

    MapId id() const noexcept;
    const QString &name() const noexcept;
    const MapConfiguration &configuration() const noexcept;
    GridType gridType() const noexcept;
    MapMode mode() const noexcept;
    const QDateTime &createdAt() const noexcept;
    const QDateTime &updatedAt() const noexcept;

    bool rename(QString name, QDateTime changedAt = QDateTime::currentDateTimeUtc());
    bool updateConfiguration(MapConfiguration configuration,
                             QDateTime changedAt = QDateTime::currentDateTimeUtc());

private:
    static QString normalizedName(QString name);
    static void validate(MapId id,
                         const QString &name,
                         const MapConfiguration &configuration,
                         const QDateTime &createdAt,
                         const QDateTime &updatedAt);

    MapId m_id;
    QString m_name;
    MapConfiguration m_configuration;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};
