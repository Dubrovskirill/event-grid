#pragma once

#include "domain/common/GridDimensions.h"
#include "MapTypes.h"

#include <QtGlobal>

struct MapConfiguration
{
    GridType gridType = GridType::FixedRectangle;
    MapMode mode = MapMode::Free;
    GridDimensions dimensions;
    CellUnit cellUnit = CellUnit::Custom;
    qint64 unitSize = 1;
    qint64 initialCompletedCells = 0;

    constexpr bool isInfinite() const noexcept
    {
        return gridType == GridType::Infinite;
    }

    constexpr bool isValid() const noexcept
    {
        if (unitSize <= 0 || initialCompletedCells < 0) {
            return false;
        }

        return isInfinite() || dimensions.isValid();
    }
};
