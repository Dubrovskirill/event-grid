#pragma once

#include "GridCoordinate.h"

#include <QtGlobal>

// GridRect uses inclusive bounds: [top, bottom] × [left, right].
struct GridRect
{
    qint64 top = 0;
    qint64 left = 0;
    qint64 bottom = -1;
    qint64 right = -1;

    constexpr bool isValid() const noexcept
    {
        return top <= bottom && left <= right;
    }

    constexpr bool contains(GridCoordinate coordinate) const noexcept
    {
        return isValid()
            && coordinate.row >= top
            && coordinate.row <= bottom
            && coordinate.column >= left
            && coordinate.column <= right;
    }

    constexpr qint64 width() const noexcept
    {
        return isValid() ? right - left + 1 : 0;
    }

    constexpr qint64 height() const noexcept
    {
        return isValid() ? bottom - top + 1 : 0;
    }
};
