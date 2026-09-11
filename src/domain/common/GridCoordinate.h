#pragma once

#include <QHash>
#include <QtGlobal>

struct GridCoordinate
{
    qint64 row = 0;
    qint64 column = 0;

    constexpr bool operator==(const GridCoordinate &other) const noexcept
    {
        return row == other.row && column == other.column;
    }

    constexpr bool operator!=(const GridCoordinate &other) const noexcept
    {
        return !(*this == other);
    }
};

inline uint qHash(const GridCoordinate &coordinate, uint seed = 0) noexcept
{
    const uint rowHash = ::qHash(coordinate.row, seed);
    const uint columnHash = ::qHash(coordinate.column, seed ^ 0x9e3779b9U);
    return rowHash ^ (columnHash + 0x9e3779b9U + (rowHash << 6U) + (rowHash >> 2U));
}
