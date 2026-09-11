#pragma once

#include "GridCoordinate.h"

#include <QtGlobal>

struct GridDimensions
{
    qint64 rows = 0;
    qint64 columns = 0;

    constexpr bool isValid() const noexcept
    {
        return rows > 0 && columns > 0;
    }

    constexpr bool contains(GridCoordinate coordinate) const noexcept
    {
        return isValid()
            && coordinate.row >= 0
            && coordinate.row < rows
            && coordinate.column >= 0
            && coordinate.column < columns;
    }
};
