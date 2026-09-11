#pragma once

enum class GridType
{
    FixedRectangle,
    Infinite,
    Calendar
};

enum class MapMode
{
    Free,
    Game
};

enum class CellUnit
{
    Custom,
    Day,
    Week,
    Month,
    Year,
    Repetition,
    Exercise,
    Event
};

enum class CellState
{
    Empty,
    Completed,
    Locked
};
