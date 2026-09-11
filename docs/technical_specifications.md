# 1. Общая информация

## 1.1. Наименование проекта

Рабочее название: **Event Grid / Interactive Maps**

## 1.2. Назначение системы

Приложение представляет собой универсальный визуальный движок интерактивных карт, состоящих из ячеек.

Ячейка является минимальной единицей информации и может одновременно выступать:

* визуальным элементом сетки;
* индикатором прогресса;
* календарным периодом;
* единицей выполненного действия;
* элементом дневника;
* элементом игровой карты;
* контейнером тегов, заметок и метаданных.

Система не должна быть жестко привязана к конкретному сценарию использования.

Один и тот же движок должен позволять создавать, например:

* Life Calendar;
* трекер привычек;
* дневник;
* трекер тренировок;
* трекер учебы;
* счетчик достижений;
* карту прогресса;
* геймифицированную карту прохождения;
* произвольную пользовательскую event-map.

---

# 2. Цели проекта

Основные цели:

1. Создать универсальный движок отображения больших сеток.
2. Разделить доменную логику, хранение и UI.
3. Обеспечить эффективную работу с тысячами и потенциально миллионами виртуальных ячеек.
4. Поддержать несколько типов сеток без изменения ядра.
5. Поддержать произвольные цветовые комбинации тегов внутри одной ячейки.
6. Предусмотреть режим игровой карты с регионами и зависимостями разблокировки.
7. Сделать слой хранения заменяемым.
8. Не привязывать архитектуру к SQLite.
9. Обеспечить возможность последующего подключения:

   * REST API;
   * удаленной БД;
   * облачного хранения;
   * синхронизации нескольких устройств.
10. Поддержать Android как основную платформу и Windows/Linux как дополнительные.

---

# 3. Границы системы

## 3.1. Входит в систему

В систему входят:

* создание и удаление карт;
* настройка карты;
* создание и управление тегами;
* работа с ячейками;
* хранение заметок;
* визуализация сеток;
* zoom/pan;
* virtualized rendering;
* long press;
* быстрые изменения состояния;
* регионы;
* разблокировка регионов;
* локальное хранение;
* импорт/экспорт данных;
* undo/redo;
* базовая аналитика по картам.

## 3.2. Не входит в MVP

В первую версию не обязательно включать:

* обязательную регистрацию;
* аккаунты;
* сервер;
* социальные функции;
* совместное редактирование;
* push-уведомления;
* сложную синхронизацию;
* многопользовательский режим.

Архитектура при этом должна позволять добавить их позже.

---

# 4. Платформы и технологии

## 4.1. Основной стек

Основной стек:

* C++;
* Qt 6.x;
* Qt Quick;
* QML;
* Qt Quick Scene Graph;
* SQLite.

UI строится преимущественно на QML.

Бизнес-логика и работа с данными реализуются на C++.

## 4.2. Целевые платформы

Приоритет:

1. Android;
2. Windows;
3. Linux.

Ядро должно быть максимально платформонезависимым.

Желательно разделить проект как:

```text
Application
    ↓
Presentation
    ↓
Application / Use Cases
    ↓
Domain
    ↓
Infrastructure
```

---

# 5. Архитектура системы

Рекомендуемая архитектура:

```text
┌──────────────────────────────────────┐
│               QML/UI                 │
│   Pages / Controls / Animations      │
└──────────────────┬───────────────────┘
                   │
                   ▼
┌──────────────────────────────────────┐
│          Presentation Layer          │
│ ViewModels / Models / Controllers    │
└──────────────────┬───────────────────┘
                   │
                   ▼
┌──────────────────────────────────────┐
│       Application / Use Cases        │
│ CreateMap / PaintCell / EditCell ... │
└──────────────────┬───────────────────┘
                   │
                   ▼
┌──────────────────────────────────────┐
│             Domain Core              │
│ Map / Cell / Tag / Region / Rules    │
└──────────────────┬───────────────────┘
                   │
                   ▼
┌──────────────────────────────────────┐
│       Repository Interfaces          │
│ IMapRepository / ICellRepository     │
└──────────────────┬───────────────────┘
                   │
          ┌────────┴─────────┐
          ▼                  ▼
┌─────────────────┐ ┌──────────────────┐
│ SQLiteStorage   │ │ RemoteStorage    │
│                 │ │ Future           │
└─────────────────┘ └──────────────────┘
```

Ключевой принцип:

**UI никогда не должен напрямую работать с SQLite.**

QML не должен содержать бизнес-правила.

Например, QML не должен самостоятельно решать:

> «Можно ли закрасить эту ячейку?»

QML сообщает:

```text
user tapped cell
        ↓
ViewModel
        ↓
PaintCellUseCase
        ↓
GameRules / MapRules
        ↓
CellRepository
        ↓
Model update
        ↓
Scene update
```

---

# 6. Доменная модель

## 6.1. Идентификаторы

Для долгосрочной совместимости рекомендуется использовать UUID:

```cpp
using MapId = QUuid;
using CellId = QUuid;
using TagId = QUuid;
using RegionId = QUuid;
```

При этом координата клетки также должна храниться явно.

Например:

```cpp
struct GridCoordinate
{
    qint64 row;
    qint64 column;
};
```

Это важно для бесконечных карт.

---

# 7. Сущность Map

```cpp
class Map
{
public:
    MapId id() const;

    QString name() const;

    GridType gridType() const;

    GridDimensions dimensions() const;

    CellSize cellSize() const;

    bool isInfinite() const;

    int initialProgress() const;

    MapMode mode() const;
};
```

## 7.1. GridType

```cpp
enum class GridType
{
    FixedRectangle,
    Infinite,
    Calendar
};
```

В будущем можно добавить:

```cpp
enum class GridType
{
    FixedRectangle,
    Infinite,
    Calendar,
    Timeline,
    Custom
};
```

---

# 8. Конфигурация карты

```cpp
struct MapConfiguration
{
    QString name;

    GridType gridType;

    GridDimensions dimensions;

    CellUnit unit;

    qint64 unitSize;

    MapMode mode;
};
```

`GridType` — единственный источник истины для типа сетки. В частности,
бесконечная карта определяется через `gridType == GridType::Infinite`; отдельный
изменяемый флаг `infinite` не хранится.

Например:

```text
Life Calendar

rows = 52
columns = 80
unit = Week
```

или:

```text
Workout

rows = 100
columns = 10
unit = PullUp
```

---

# 9. Сущность Cell

Ключевая сущность системы:

```cpp
class Cell
{
public:
    CellId id() const;

    MapId mapId() const;

    GridCoordinate coordinate() const;

    bool isCompleted() const;

    QVector<TagId> tags() const;

    QString note() const;

    QDateTime createdAt() const;

    QDateTime updatedAt() const;
};
```

При этом важный архитектурный момент:

## Необходимо разделить понятия

```text
CellDefinition
CellState
CellContent
```

Например:

### CellDefinition

Определяет существование клетки:

```text
map
row
column
region
```

### CellState

```text
completed
locked
progress
```

### CellContent

```text
tags
note
date
metadata
```

Это позволит не хранить огромное количество лишних данных.

---

# 10. Sparse-модель бесконечной сетки

Для бесконечной сетки нельзя создавать миллион объектов `Cell`.

Вместо этого используется принцип:

```text
Virtual Cell
```

Клетка существует логически по координатам:

```text
(row=100000, column=500000)
```

но объект в БД создается только при наличии пользовательских данных.

То есть:

```text
empty cell
    ↓
не хранится физически

user interacts
    ↓
Cell создается
    ↓
записывается в storage
```

Это критически важно для infinite grid.

---

# 11. Теги

```cpp
class Tag
{
public:
    TagId id() const;

    QString name() const;

    QColor color() const;
};
```

Требования:

* имя тега не пустое;
* UUID уникален;
* цвет хранится в RGBA;
* тег может использоваться на множестве ячеек;
* ячейка может иметь несколько тегов.

---

# 12. Цветовое представление ячейки

Если у клетки один тег:

```text
████████
```

Если два:

```text
████
████
```

или:

```text
◢◣
```

Для N тегов используется секторное разделение.

Алгоритм:

```text
center = center of cell

angle = 360° / tagCount
```

Каждый тег получает:

```text
[startAngle, endAngle]
```

Пример:

```text
2 tags → 180° + 180°

3 tags → 120° + 120° + 120°

4 tags → 90° × 4
```

При дальнейшем расширении можно добавить:

```text
5+
```

с использованием более сложного распределения.

---

# 13. Рекомендуемый Render Model

UI не должен получать полноценные C++ объекты `Cell` для каждой видимой ячейки.

Вместо этого создается компактная структура:

```cpp
struct CellRenderData
{
    qint64 row;
    qint64 column;

    bool completed;
    bool locked;

    QVector<QColor> colors;

    bool hasNote;
};
```

Она предназначена исключительно для отрисовки.

Таким образом:

```text
Domain Cell
     ↓
CellRenderData
     ↓
Scene Graph
```

---

# 14. Регионы

Для game mode вводится:

```cpp
class Region
{
public:
    RegionId id() const;

    QString name() const;

    QVector<GridCoordinate> cells() const;

    QVector<RegionId> prerequisites() const;

    bool isUnlocked() const;

    bool isCompleted() const;
};
```

Регион может включать:

```text
20 cells
...
100 cells
```

---

# 15. Форма региона

Регион не должен быть ограничен прямоугольником.

Варианты представления:

### Вариант A — список координат

```cpp
QSet<GridCoordinate>
```

Преимущества:

* просто;
* удобно для произвольной формы;
* идеально для игрового режима.

### Вариант B — Polygon

```cpp
QPolygonF geometry;
```

Этот вариант нужен преимущественно для визуализации.

Рекомендуется хранить логическую форму через набор ячеек, а polygon использовать как производную визуальную геометрию.

---

# 16. Game Mode

```cpp
enum class MapMode
{
    Free,
    Game
};
```

В Free Mode:

```text
cell.locked = false
```

В Game Mode:

```text
Region 1 → unlocked
Region 2 → locked
Region 3 → locked
```

После полного completion Region 1:

```text
Region 2 → unlocked
```

---

# 17. Game Rules

Игровые правила не должны находиться в `Region` или `Cell`.

Создается отдельный сервис:

```cpp
class IMapRules
{
public:
    virtual ~IMapRules() = default;

    virtual bool canModifyCell(
        const Map& map,
        const Cell& cell) const = 0;

    virtual void onCellChanged(
        Map& map,
        Cell& cell) = 0;
};
```

Конкретная реализация:

```cpp
class FreeMapRules : public IMapRules
{
};

class GameMapRules : public IMapRules
{
};
```

Это соответствует Strategy Pattern и позволяет добавлять новые типы поведения.

---

# 18. Application Layer

На уровне application располагаются Use Cases.

Примеры:

```text
CreateMap
DeleteMap
RenameMap
CreateTag
DeleteTag

PaintCell
ResetCell
EditCell
AssignTag
RemoveTag

CreateRegion
UnlockRegion

Undo
Redo

ExportMap
ImportMap
```

Например:

```cpp
class PaintCellUseCase
{
public:
    Result execute(
        MapId mapId,
        GridCoordinate coordinate);
};
```

Use Case:

1. загружает карту;
2. проверяет правила;
3. изменяет Cell;
4. сохраняет изменение;
5. публикует Domain Event;
6. UI получает обновление.

---

# 19. Repository Interfaces

Главное архитектурное требование проекта — полная абстракция хранения.

## 19.1. Map Repository

```cpp
class IMapRepository
{
public:
    virtual ~IMapRepository() = default;

    virtual std::optional<Map> find(MapId id) = 0;

    virtual QVector<Map> findAll() = 0;

    virtual void save(const Map& map) = 0;

    virtual void remove(MapId id) = 0;
};
```

## 19.2. Cell Repository

```cpp
class ICellRepository
{
public:
    virtual ~ICellRepository() = default;

    virtual std::optional<Cell> find(CellId id) = 0;

    virtual std::optional<Cell> findByCoordinate(
        MapId mapId,
        GridCoordinate coordinate) = 0;

    virtual QVector<Cell> findInRegion(
        MapId mapId,
        GridRect area) = 0;

    virtual void save(const Cell& cell) = 0;

    virtual void remove(CellId id) = 0;
};
```

## 19.3. Tag Repository

```cpp
class ITagRepository
{
public:
    virtual ~ITagRepository() = default;

    virtual QVector<Tag> findAll(MapId mapId) = 0;

    virtual void save(const Tag& tag) = 0;

    virtual void remove(TagId id) = 0;
};
```

---

# 20. Почему Repository недостаточно

Для масштабирования желательно добавить отдельный слой:

```cpp
class IMapStorage
{
public:
    virtual ~IMapStorage() = default;

    virtual std::unique_ptr<IMapRepository> maps() = 0;

    virtual std::unique_ptr<ICellRepository> cells() = 0;

    virtual std::unique_ptr<ITagRepository> tags() = 0;

    virtual std::unique_ptr<IRegionRepository> regions() = 0;

    virtual void transactionBegin() = 0;

    virtual void transactionCommit() = 0;

    virtual void transactionRollback() = 0;
};
```

Тогда приложение получает единый механизм хранения.

---

# 21. SQLite

Для MVP рекомендуется SQLite.

Причины:

* локальная работа;
* отсутствие сервера;
* транзакции;
* индексы;
* хорошая работа с большими объемами данных;
* возможность работы на Android/Windows/Linux.

---

# 22. Предлагаемая структура БД

## maps

```text
maps
-----
id
name
grid_type
mode
rows
columns
is_infinite
unit
unit_size
created_at
updated_at
```

## cells

```text
cells
-----
id
map_id
row
column
completed
note
date_value
created_at
updated_at
```

Индекс:

```text
(map_id, row, column)
```

Это один из самых важных индексов проекта.

---

# 23. cell_tags

Связь many-to-many:

```text
cell_tags
---------
cell_id
tag_id
```

Уникальный индекс:

```text
(cell_id, tag_id)
```

---

# 24. tags

```text
tags
----
id
map_id
name
color
created_at
updated_at
```

---

# 25. regions

```text
regions
-------
id
map_id
name
sort_order
locked
completed
```

## region_cells

```text
region_cells
------------
region_id
row
column
```

Таким образом регион фактически является набором координат.

---

# 26. region_dependencies

```text
region_dependencies
-------------------
region_id
required_region_id
```

Это позволит перейти от простой последовательности:

```text
A → B → C
```

к графу:

```text
A ──┐
    ├── C
B ──┘
```

---

# 27. Метаданные

Вместо добавления столбца БД под каждый будущий тип данных можно иметь:

```text
cell_metadata
-------------
cell_id
key
value
value_type
```

Например:

```text
weight = 74.5
duration = 38
mood = 4
location = ...
```

При этом metadata не должна превращаться в основной способ хранения критичных данных.

Для часто используемых полей лучше иметь типизированные столбцы.

---

# 28. Будущая синхронизация

Даже без сервера с самого начала следует использовать:

```cpp
createdAt
updatedAt
version
UUID
```

Также желательно:

```cpp
DeviceId
```

Позже можно добавить:

```text
SyncState
ConflictState
Revision
```

Например:

```cpp
enum class SyncState
{
    LocalOnly,
    PendingUpload,
    Synced,
    Conflict
};
```

Это позволит позже подключить:

```text
SQLite
   ↕
Sync Engine
   ↕
REST / GraphQL
   ↕
Cloud Backend
```

без переписывания Domain Layer.

---

# 29. Синхронизация

В будущем:

```cpp
class ISyncService
{
public:
    virtual ~ISyncService() = default;

    virtual SyncResult synchronize() = 0;
};
```

и:

```cpp
class IRemoteStorage
{
public:
    virtual ~IRemoteStorage() = default;

    virtual RemoteChanges pull() = 0;

    virtual UploadResult push(
        const QVector<Change>& changes) = 0;
};
```

SQLite при этом останется локальным source of truth для UI.

---

# 30. Undo / Redo

Поскольку приложение предполагает много быстрых действий, Undo/Redo рекомендуется заложить в архитектуру с самого начала.

Использовать Command Pattern:

```cpp
class ICommand
{
public:
    virtual ~ICommand() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
};
```

Примеры:

```text
PaintCellCommand
ResetCellCommand
AddTagCommand
RemoveTagCommand
EditCellNoteCommand
```

Для пользователя это позволит:

```text
Tap
Tap
Tap
Tap
Ctrl+Z
```

а на Android можно использовать собственный UI Undo.

---

# 31. Domain Events

Полезно реализовать события:

```text
CellChanged
MapChanged
TagChanged
RegionUnlocked
RegionCompleted
```

Например:

```cpp
struct CellChangedEvent
{
    CellId cellId;
    GridCoordinate coordinate;
};
```

Это позволит UI перерисовывать только изменившийся участок.

---

# 32. Presentation Layer

Для QML не следует напрямую экспортировать всю Domain Model.

Рекомендуется использовать ViewModel.

Например:

```cpp
class MapViewModel : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void paintCell(
        qint64 row,
        qint64 column);

    Q_INVOKABLE void resetCell(
        qint64 row,
        qint64 column);

    Q_INVOKABLE void editCell(
        qint64 row,
        qint64 column);
};
```

---

# 33. Grid View

Основной компонент:

```text
InteractiveGrid
```

Рекомендуется реализовать как:

```cpp
class GridView : public QQuickItem
{
    Q_OBJECT
};
```

а не как тысячи QML:

```qml
Rectangle {
    Repeater {
        model: 100000
    }
}
```

Такой подход для больших сеток будет плохо масштабироваться.

---

# 34. Scene Graph

`GridView` должен самостоятельно определить:

```text
visible world rectangle
```

например:

```text
row 100 ... 135
column 200 ... 245
```

и генерировать render data только для этого диапазона.

То есть:

```text
1000000 cells
       ↓
viewport
       ↓
37 × 45 visible cells
       ↓
~1665 rendered cells
```

---

# 35. Viewport Culling

На каждом кадре определяется:

```cpp
GridRect visibleCells()
```

Алгоритм:

```text
screen coordinates
        ↓
inverse transformation
        ↓
world coordinates
        ↓
grid coordinates
        ↓
visible rows/columns
```

Отрисовываются только видимые клетки.

---

# 36. Zoom

Состояние камеры:

```cpp
struct CameraState
{
    QPointF offset;
    qreal zoom;
};
```

Трансформация:

```text
screen = world * zoom + offset
```

Диапазон zoom рекомендуется ограничить:

```text
0.05 ... 100x
```

но реальные значения должны настраиваться после performance-тестов.

---

# 37. Pinch Zoom

На Android поддержать:

```text
PinchHandler
```

или собственный input controller.

Очень важное UX-требование:

при zoom центр масштабирования должен находиться под пальцами.

Нельзя делать:

```text
zoom around screen center
```

Правильно:

```text
finger position = fixed world point
```

во время масштабирования.

---

# 38. Pan

Поддержать:

```text
one-finger drag
```

при необходимости с отличением от tap.

Для desktop:

```text
mouse drag
wheel
```

---

# 39. Long Press

Сценарий:

```text
tap
    ↓
quick action

long press
    ↓
edit cell
```

Рекомендуемая задержка:

```text
400–600 ms
```

точное значение определяется UX-тестированием.

---

# 40. Quick Action

Одиночный тап:

Если:

```text
cell.completed == false
```

то:

```text
completed = true
```

Если:

```text
cell.completed == true
```

то:

```text
completed = false
```

Однако это поведение должно быть вынесено в Use Case.

QML только сообщает:

```text
cellTapped(row, column)
```

---

# 41. Редактор клетки

При long press открывается:

```text
CellEditor
```

UI:

```text
┌─────────────────────────────┐
│         Cell #1024          │
│                             │
│  Status: ✓ Completed        │
│                             │
│  Tags                       │
│  [Work] [Sport] [+]         │
│                             │
│  Note                       │
│  ┌───────────────────────┐  │
│  │ Text...               │  │
│  └───────────────────────┘  │
│                             │
│  Date                       │
│  10 Sep 2026                │
│                             │
│       [Cancel] [Save]       │
└─────────────────────────────┘
```

---

# 42. Сценарий создания карты

Пользователь:

```text
Create Map
```

выбирает:

```text
Name
Type
Rows
Columns
Infinite
Unit
Mode
```

Пример:

```text
Life

Grid:
52 × 80

Unit:
1 week

Mode:
Free
```

После создания открывается Grid View.

---

# 43. Начальное заполнение

Параметр:

```text
initialCompletedCells
```

может быть:

```text
0
100
500
```

Но хранить каждую стартовую клетку не обязательно.

Для больших карт можно хранить компактное состояние:

```text
initialProgress
```

а фактические пользовательские изменения хранить отдельно.

Для MVP допустимо физически создать начальные клетки.

---

# 44. Calendar Mode

Calendar Grid должен иметь специализированный адаптер:

```cpp
class IGridLayout
{
public:
    virtual GridCoordinate coordinateFor(
        const QVariant& logicalValue) const = 0;

    virtual QVariant logicalValueFor(
        GridCoordinate coordinate) const = 0;
};
```

Например:

```text
2026-01-01 → row=0,column=0
2026-01-02 → row=0,column=1
...
```

Это позволит иметь разные логические системы координат при едином renderer.

---

# 45. Layout Strategy

Рекомендуется Strategy Pattern:

```cpp
class IGridLayoutStrategy
{
public:
    virtual ~IGridLayoutStrategy() = default;

    virtual GridGeometry geometry() const = 0;

    virtual QRectF cellRect(
        GridCoordinate coordinate) const = 0;
};
```

Реализации:

```text
RectangleGridLayout
CalendarGridLayout
InfiniteGridLayout
```

Позже:

```text
TimelineLayout
HexGridLayout
CustomGridLayout
```

---

# 46. Renderer

Архитектурно лучше разделить:

```text
GridController
      ↓
VisibleCellProvider
      ↓
CellRenderData
      ↓
GridRenderer
```

Renderer ничего не должен знать про:

* SQLite;
* Use Cases;
* Tag Repository;
* Game Rules.

Он знает только:

```text
geometry
colors
state
```

---

# 47. Рендеринг ячеек

Основная стратегия:

```text
QQuickItem
   +
QSGNode
   +
QSGGeometry
```

Для обычных квадратов желательно использовать максимально мало Scene Graph Nodes.

Не рекомендуется делать:

```text
1 cell = 1 QML Item
```

при тысячах объектов.

Лучше:

```text
множество cell instances
        ↓
батч / geometry
        ↓
Scene Graph
```

---

# 48. Стратегия визуализации тегов

Есть два основных режима.

## MVP

Для каждой видимой клетки генерируется geometry:

```text
1 tag   → rectangle
2 tags  → 2 triangles
3 tags  → 3 triangles
4 tags  → 4 sectors
```

## Оптимизация

При дальнейшем масштабировании можно перейти к:

```text
instanced rendering
+
shader
```

где GPU получает:

```text
cell position
cell size
tag colors
tag count
state
```

и самостоятельно собирает визуальную геометрию.

---

# 49. Semantic Zoom

Это важное требование для карты.

На большом zoom:

```text
┌───┬───┬───┐
│ A │ B │ C │
├───┼───┼───┤
│ D │ E │ F │
└───┴───┴───┘
```

При сильном отдалении необязательно рисовать границы каждой клетки.

Можно переходить к:

```text
цветовым блокам
```

или:

```text
агрегированной статистике региона
```

Пример:

```text
Zoom 100%
→ full cells

Zoom 20%
→ cells without labels

Zoom 5%
→ aggregated blocks
```

Это значительно снижает нагрузку на GPU.

---

# 50. Максимальное отдаление

Для конечной карты:

```text
Fit to Canvas
```

должно позволять показать всю карту.

Алгоритм:

```text
map bounds
        ↓
viewport size
        ↓
fit scale
        ↓
center
```

Для бесконечной карты понятия:

```text
"показать всю карту"
```

не существует.

Там необходимо использовать:

```text
Fit to Content
```

то есть показать область, в которой существуют пользовательские данные.

---

# 51. Бесконечная карта

Infinite Map должна использовать:

```text
virtual coordinates
```

например:

```text
(-1000000, -1000000)
...
(1000000, 1000000)
```

без физического создания клеток.

При движении камеры запрашивается только:

```text
visible region
```

---

# 52. Prefetching

Чтобы не было подгрузки непосредственно во время движения камеры, можно использовать область вокруг viewport:

```text
viewport
+
prefetch margin
```

Например:

```text
visible
100×100

prefetch
120×120
```

Из БД подготавливается немного больше данных.

---

# 53. Cache

Необходимо минимум два кэша.

## Memory Cell Cache

```cpp
CellCache
```

Хранит недавно использованные клетки.

## Render Cache

Хранит:

```cpp
CellRenderData
```

для текущей области.

Кэш должен поддерживать LRU или похожую стратегию.

---

# 54. Multithreading

Нельзя блокировать UI thread во время:

```text
SQLite query
```

или тяжелой обработки.

Рекомендуемая структура:

```text
UI Thread
   │
   ├── ViewModel
   │
   └── Render
       
Worker Thread
   │
   └── Repository / SQLite
```

При этом Scene Graph операции выполняются согласно правилам Qt Quick и не переносятся произвольно в фоновые потоки.

---

# 55. Async API

Вместо:

```cpp
QVector<Cell> findInRegion(...)
```

в Presentation Layer желательно постепенно перейти к async-паттерну:

```cpp
void requestVisibleCells(GridRect area);
```

результат приходит позже:

```text
request
   ↓
background query
   ↓
result
   ↓
update cache
   ↓
update()
```

Для Qt это можно реализовать через:

* `QFuture`;
* собственный Worker;
* signal/slot;
* task queue.

---

# 56. Game Map UI

В Game Mode экран должен отображать:

```text
┌─────────────────────────────┐
│ Progress: 43%               │
│ Region: Forest               │
├─────────────────────────────┤
│                             │
│       interactive map       │
│                             │
│  [█████]                    │
│  [█████████]                │
│  [████████████]             │
│       🔒                     │
│                             │
└─────────────────────────────┘
```

Заблокированный регион может иметь:

* затемнение;
* pattern overlay;
* lock icon;
* отсутствие реакции на tap.

---

# 57. Разблокировка

Для региона:

```text
required regions completed
```

тогда:

```text
RegionState = Unlocked
```

Событие:

```cpp
RegionUnlockedEvent
```

может инициировать:

* animation;
* sound;
* vibration;
* notification.

Это должно быть Presentation concern, а не Domain concern.

---

# 58. Use Cases

Минимальный набор:

## Maps

```text
CreateMap
UpdateMap
DeleteMap
DuplicateMap
GetMaps
GetMap
```

## Cells

```text
GetCell
PaintCell
ResetCell
ToggleCell
UpdateCell
```

## Tags

```text
CreateTag
RenameTag
DeleteTag
AssignTag
RemoveTag
```

## Regions

```text
CreateRegion
UpdateRegion
DeleteRegion
UnlockRegion
GetRegionProgress
```

## Data

```text
ExportMap
ImportMap
Backup
Restore
```

## History

```text
Undo
Redo
```

---

# 59. Use Case: PaintCell

Псевдологика:

```text
PaintCell
    ↓
load map
    ↓
load cell
    ↓
MapRules::canModifyCell()
    ↓
if false:
    reject
    ↓
if true:
    cell.completed = true
    ↓
repository.save()
    ↓
check region completion
    ↓
unlock next region
    ↓
publish events
```

---

# 60. Use Case: Long Press

```text
Pointer Down
    ↓
start timer
    ↓
Pointer Move
    ├── movement > threshold → cancel
    ↓
Timer
    ↓
longPress
    ↓
Open CellEditor
```

Важно иметь threshold по расстоянию, чтобы drag не считался long press.

---

# 61. Use Case: Pan vs Tap

Необходимо определить:

```text
Tap
LongPress
Pan
Pinch
```

как разные жесты.

Рекомендуемые параметры:

```text
movement threshold:
8–16 px
```

с учетом DPI.

В коде нельзя жестко привязываться к физическим пикселям.

---

# 62. Accessibility

Необходимо предусмотреть:

* достаточный размер интерактивных элементов;
* поддержку keyboard navigation на Desktop;
* корректные accessible names;
* контраст;
* масштабируемый текст.

Для самой карты желательно иметь альтернативное представление состояния:

```text
Cell 15,20
Completed
Tags: Sport, Work
Note available
```

---

# 63. Экран приложения

Рекомендуемая структура:

```text
App
│
├── MapsScreen
│
├── MapEditorScreen
│   ├── Toolbar
│   ├── GridViewport
│   └── MapInfo
│
├── CellEditorDialog
│
├── TagManagerScreen
│
├── RegionEditorScreen
│
├── MapSettingsScreen
│
└── AppSettingsScreen
```

---

# 64. MapsScreen

Показывает:

```text
My Maps

[Life Calendar]
[Workout]
[Reading]
[Spanish]
[Game Map]

                [+]
```

Для карты отображать:

* название;
* тип;
* процент выполнения;
* количество клеток;
* последнюю активность.

---

# 65. Map Settings

Настройки:

```text
Name

Grid type

Rows
Columns

Infinite

Cell unit

Default tag

Game mode

Initial progress

Theme
```

Некоторые свойства после начала использования карты могут стать immutable.

Например, изменение:

```text
cell unit
```

может потребовать явного подтверждения.

---

# 66. Требования к данным

Все пользовательские изменения должны быть транзакционными.

Например:

```text
PaintCell
```

должно либо:

```text
полностью сохраниться
```

либо:

```text
не сохраниться вообще.
```

---

# 67. Crash Safety

SQLite должна использовать транзакции.

После аварийного завершения:

* данные не должны повреждаться;
* последняя завершенная транзакция должна сохраняться;
* незавершенная операция должна откатываться.

---

# 68. Backup

На уровне MVP желательно предоставить:

```text
Export Map
```

в собственный формат.

Например:

```text
.mapdata
```

Фактически это может быть ZIP:

```text
mapdata/
    manifest.json
    cells.json
    tags.json
    regions.json
```

или SQLite snapshot.

Рекомендуется собственный versioned export format.

---

# 69. Версионирование формата

Файл должен содержать:

```json
{
    "formatVersion": 1
}
```

При изменении структуры:

```text
version 2
version 3
...
```

Migration Service:

```cpp
class IDataMigration
{
public:
    virtual int fromVersion() const = 0;
    virtual int toVersion() const = 0;

    virtual void migrate(...) = 0;
};
```

---

# 70. SOLID

## Single Responsibility

Не должно существовать:

```cpp
MegaMapManager
```

который одновременно:

* рисует;
* хранит;
* применяет правила;
* работает с SQLite;
* управляет QML.

Вместо него:

```text
MapRepository
MapService
GridRenderer
GameRules
MapViewModel
```

---

# 71. Open/Closed

Добавление:

```text
TimelineMap
```

не должно требовать переписывать:

```text
Cell
Map
SQLiteRepository
GridRenderer
```

Добавляется новая:

```cpp
IGridLayoutStrategy
```

---

# 72. Liskov

Все реализации:

```text
IMapRepository
```

должны быть взаимозаменяемыми:

```text
SQLiteMapRepository
RemoteMapRepository
InMemoryMapRepository
```

---

# 73. Interface Segregation

Не создавать:

```cpp
IGodRepository
```

с десятками методов.

Лучше:

```text
IMapRepository
ICellRepository
ITagRepository
IRegionRepository
```

---

# 74. Dependency Inversion

Application Layer зависит от:

```cpp
ICellRepository
```

а не от:

```cpp
SQLiteCellRepository
```

Конкретная реализация передается через Dependency Injection.

---

# 75. Dependency Injection

Можно использовать собственный lightweight composition root.

Например:

```cpp
class Application
{
public:
    void registerDependencies();
};
```

В production:

```text
SQLiteMapRepository
```

В тестах:

```text
InMemoryMapRepository
```

---

# 76. In-Memory Repository

Это принципиально важно для тестирования.

Например:

```cpp
class InMemoryCellRepository :
    public ICellRepository
{
};
```

Тогда Domain / Application можно тестировать без Android, QML и SQLite.

---

# 77. Unit Testing

Минимально протестировать:

## Domain

* Cell;
* Map;
* Tag;
* Region;
* progress calculation.

## Rules

* нельзя менять locked region;
* можно менять unlocked region;
* completion региона;
* автоматическое unlock.

## Repository

* CRUD;
* transactions;
* queries;
* indexes.

## Use Cases

* PaintCell;
* ToggleCell;
* EditCell;
* AssignTag.

---

# 78. Rendering Tests

Отдельно проверить:

```text
100 cells
1,000 cells
10,000 cells
100,000 logical cells
1,000,000 logical cells
```

При этом размер данных и количество видимых элементов нужно рассматривать отдельно.

Основной критерий:

> производительность должна зависеть преимущественно от количества видимых/агрегированных объектов, а не от общего количества логических клеток карты.

---

# 79. Performance Requirements

Целевые требования:

### UI

```text
60 FPS
```

на поддерживаемом среднем Android-устройстве.

### Input latency

Tap → visual feedback:

```text
желательно < 50 ms
```

### Scroll/Pan

Не должно быть обязательных полных запросов всей карты.

### Zoom

Не должен приводить к пересозданию всей модели.

### Memory

Количество загруженных Cell объектов должно быть ограничено viewport/cache policy.

---

# 80. Ключевое performance-правило

Нельзя делать:

```text
Map
 ↓
1,000,000 Cell objects
 ↓
QML Model
 ↓
1,000,000 delegates
```

Нужно:

```text
Map metadata
        +
Sparse persisted cells
        +
Camera
        ↓
VisibleRect
        ↓
VisibleCellProvider
        ↓
few thousand render instances
```

---

# 81. Полезный набор C++ классов

## Domain

```text
Map
Cell
Tag
Region

GridCoordinate
GridRect
GridDimensions

MapConfiguration
CellMetadata
```

## Application

```text
CreateMapUseCase
PaintCellUseCase
EditCellUseCase
AssignTagUseCase
CreateRegionUseCase
UndoUseCase
ExportMapUseCase
```

## Domain Services

```text
MapRules
FreeMapRules
GameMapRules

ProgressService
RegionService
```

## Repository

```text
IMapRepository
ICellRepository
ITagRepository
IRegionRepository
```

## Infrastructure

```text
SQLiteDatabase
SQLiteMapRepository
SQLiteCellRepository
SQLiteTagRepository
SQLiteRegionRepository

DatabaseMigrator
Transaction
```

## Presentation

```text
MapViewModel
CellEditorViewModel
TagViewModel
RegionViewModel
MapsViewModel
```

## Rendering

```text
GridView
GridRenderer
CellRenderData
CameraController
ViewportController
GridLayoutStrategy
```

---

# 82. Пример структуры проекта

```text
src/
│
├── domain/
│   ├── map/
│   │   ├── Map.h
│   │   ├── Map.cpp
│   │   ├── MapConfiguration.h
│   │   └── MapTypes.h
│   │
│   ├── cell/
│   │   ├── Cell.h
│   │   ├── Cell.cpp
│   │   └── CellTypes.h
│   │
│   ├── tag/
│   │   ├── Tag.h
│   │   └── Tag.cpp
│   │
│   ├── region/
│   │   ├── Region.h
│   │   └── Region.cpp
│   │
│   └── rules/
│       ├── IMapRules.h
│       ├── FreeMapRules.h
│       └── GameMapRules.h
│
├── application/
│   ├── usecases/
│   │   ├── CreateMapUseCase.h
│   │   ├── PaintCellUseCase.h
│   │   ├── EditCellUseCase.h
│   │   └── ...
│   │
│   └── events/
│
├── infrastructure/
│   ├── repositories/
│   │   ├── IMapRepository.h
│   │   ├── ICellRepository.h
│   │   └── ...
│   │
│   ├── sqlite/
│   │   ├── SQLiteDatabase.h
│   │   ├── SQLiteMapRepository.h
│   │   ├── SQLiteCellRepository.h
│   │   └── ...
│   │
│   └── migrations/
│
├── presentation/
│   ├── viewmodels/
│   └── models/
│
├── rendering/
│   ├── GridView.h
│   ├── GridRenderer.h
│   ├── CameraController.h
│   ├── CellRenderData.h
│   └── layouts/
│
└── main.cpp

qml/
│
├── screens/
├── components/
├── dialogs/
├── grid/
└── main.qml

tests/
├── domain/
├── application/
├── repositories/
└── rendering/
```

---

# 83. Use Cases

## UC-01 — Создание карты

**Актор:** пользователь.

### Preconditions

Приложение запущено.

### Main Flow

1. Пользователь нажимает `+`.
2. Открывается Create Map.
3. Вводит название.
4. Выбирает тип сетки.
5. Настраивает размеры.
6. Указывает единицу клетки.
7. Выбирает режим.
8. Нажимает Create.
9. Use Case создает Map.
10. Map сохраняется.
11. Пользователь попадает на карту.

### Postcondition

Карта существует и доступна для редактирования.

---

# 84. UC-02 — Быстрая отметка клетки

1. Пользователь открывает карту.
2. Нажимает клетку.
3. Система определяет координату.
4. Запускается `ToggleCellUseCase`.
5. Проверяются правила.
6. Состояние изменяется.
7. Изменение сохраняется.
8. CellRenderData обновляется.
9. Renderer перерисовывает клетку.

---

# 85. UC-03 — Подробное редактирование

1. Пользователь удерживает клетку.
2. Система распознает long press.
3. Открывается Cell Editor.
4. Пользователь меняет теги.
5. Вводит заметку.
6. Выбирает дату.
7. Нажимает Save.
8. Use Case валидирует данные.
9. Транзакция сохраняет изменения.
10. UI получает событие изменения.

---

# 86. UC-04 — Добавление нескольких тегов

1. Открыть Cell Editor.
2. Выбрать несколько тегов.
3. Система сохраняет связи `cell_tags`.
4. Renderer получает список цветов.
5. Cell рисуется секторами.

---

# 87. UC-05 — Прохождение региона

1. Открыть Game Map.
2. Region 1 открыт.
3. Пользователь заполняет клетки.
4. Progress достигает 100%.
5. `RegionCompleted`.
6. Проверяются зависимости.
7. Следующий Region становится unlocked.
8. Проигрывается unlock animation.

---

# 88. UC-06 — Панорамирование

1. Пользователь начинает drag.
2. Gesture Controller переключается в Pan.
3. Изменяется Camera.offset.
4. Определяется новый viewport.
5. VisibleCellProvider запрашивает новые клетки.
6. Renderer отображает новый участок.

---

# 89. UC-07 — Zoom

1. Пользователь делает pinch.
2. Рассчитывается новый zoom.
3. Координата под пальцами фиксируется.
4. Изменяется Camera.
5. Пересчитывается viewport.
6. Меняется detail level.
7. Renderer использует новый LOD.

---

# 90. UC-08 — Работа без интернета

Приложение запускается без подключения к интернету.

Все базовые операции:

```text
create
edit
delete
paint
tag
region
```

должны работать локально.

---

# 91. UC-09 — Будущая синхронизация

Архитектура должна позволять:

```text
Phone
   ↓
Local SQLite
   ↓
Sync Engine
   ↓
Cloud
   ↓
Desktop
```

без изменения:

```text
Cell
Map
Tag
Region
GameRules
```

---

# 92. Нефункциональные требования

## NFR-01 — Производительность

Не менее 60 FPS при обычной навигации по карте на целевых устройствах.

## NFR-02 — Масштабируемость

Должны поддерживаться карты с:

```text
10,000+
100,000+
1,000,000+
```

логических клеток.

Для infinite grid количество логических координат не должно ограничиваться количеством созданных объектов.

## NFR-03 — Надежность

Критичные изменения сохраняются атомарно.

## NFR-04 — Offline

Основной функционал доступен без сети.

## NFR-05 — Расширяемость

Добавление новых видов сеток не должно требовать переписывания core.

## NFR-06 — Testability

Domain и Application должны тестироваться без Android и QML.

## NFR-07 — Portability

Domain/Application не должны зависеть от Android API.

---

# 93. План реализации

## Этап 0 — Архитектурный bootstrap

Создать репозиторий.

Настроить:

```text
CMake
Qt
C++ standard
warnings
tests
```

Разделить проект:

```text
domain
application
infrastructure
presentation
rendering
qml
tests
```

Создать Composition Root.

---

# 94. Этап 1 — Core Types

Реализовать:

```text
MapId
CellId
TagId
RegionId

GridCoordinate
GridRect
GridDimensions

GridType
MapMode
CellUnit
```

Написать unit tests.

---

# 95. Этап 2 — Domain Model

Реализовать:

```text
Map
Cell
Tag
Region
MapConfiguration
```

Без SQLite.

Без QML.

Цель:

```text
чистое domain ядро.
```

---

# 96. Этап 3 — Map Rules

Реализовать:

```text
IMapRules
FreeMapRules
GameMapRules
```

Проверить:

```text
locked cell
region completion
unlock
dependencies
```

---

# 97. Этап 4 — Repository Interfaces

Создать:

```text
IMapRepository
ICellRepository
ITagRepository
IRegionRepository
```

Пока без SQLite.

---

# 98. Этап 5 — In-Memory Repository

Создать:

```text
InMemoryMapRepository
InMemoryCellRepository
...
```

Это позволит уже на этом этапе начать тестировать Use Cases.

---

# 99. Этап 6 — Application Use Cases

Реализовать:

```text
CreateMap
PaintCell
ResetCell
ToggleCell
EditCell
AssignTag
CreateTag
```

После каждого Use Case — unit tests.

---

# 100. Этап 7 — SQLite

Создать:

```text
database schema
migrations
repositories
transactions
indexes
```

Особое внимание:

```text
(map_id, row, column)
```

---

# 101. Этап 8 — Persistence Tests

Проверить:

```text
create
read
update
delete
many-to-many
transactions
rollback
migration
```

---

# 102. Этап 9 — Basic QML UI

Сначала реализовать:

```text
MapsScreen
CreateMapScreen
MapSettings
```

Без оптимизированного renderer.

---

# 103. Этап 10 — MVP Grid

Сначала реализовать простую сетку:

```text
GridView
```

только для:

```text
100 × 100
```

Цель этапа:

* правильная геометрия;
* click;
* long press;
* zoom;
* pan.

---

# 104. Этап 11 — Cell Editor

Добавить:

```text
CellEditorDialog
Tag selector
Note editor
Date editor
```

---

# 105. Этап 12 — Custom Scene Graph Renderer

После проверки UX заменить простой вариант на:

```text
QQuickItem
+
QSGNode
+
QSGGeometry
```

Добавить:

```text
viewport culling
```

---

# 106. Этап 13 — Camera System

Отдельно сделать:

```text
CameraController
```

с:

```text
zoom
offset
fitToCanvas
fitToContent
worldToScreen
screenToWorld
```

Это должен быть независимый переиспользуемый компонент.

---

# 107. Этап 14 — Infinite Grid

Перевести Grid на sparse architecture:

```text
virtual cells
```

и загружать только:

```text
visible area
+
prefetch
```

---

# 108. Этап 15 — Cache

Добавить:

```text
CellCache
RenderDataCache
```

Провести benchmark.

---

# 109. Этап 16 — Multi-tag Rendering

Добавить:

```text
1 tag → rectangle

2 tags → 2 sectors

3 tags → 3 sectors

4 tags → 4 sectors
```

Затем протестировать:

```text
5+
```

---

# 110. Этап 17 — Calendar Layout

Добавить:

```text
CalendarGridLayout
```

и поддержку:

```text
day
week
month
year
```

с четким определением правил календарного отображения.

---

# 111. Этап 18 — Game Mode

Добавить:

```text
Region
Region Editor
Region dependencies
Progress calculation
Unlock logic
```

---

# 112. Этап 19 — Game Rendering

Добавить:

* locked state;
* region borders;
* region labels;
* progress;
* unlock animations.

---

# 113. Этап 20 — Semantic Zoom

Добавить несколько уровней:

```text
LOD 0
Full details

LOD 1
Cells without secondary data

LOD 2
Aggregated blocks

LOD 3
Region overview
```

---

# 114. Этап 21 — Undo/Redo

Подключить:

```text
Command Pattern
```

к Use Cases.

---

# 115. Этап 22 — Export/Import

Создать:

```text
Map Export Format v1
```

с versioning.

---

# 116. Этап 23 — Performance Benchmark

Автоматически тестировать:

```text
10k
100k
1M
10M logical cells
```

и измерять:

```text
FPS
frame time
CPU
GPU
RAM
database query time
input latency
```

---

# 117. Этап 24 — Android Optimization

Проверить:

* touch;
* pinch;
* memory;
* application lifecycle;
* background/foreground;
* sleep/resume;
* Android storage;
* backup;
* screen density;
* low-end devices.

---

# 118. Этап 25 — Windows/Linux

Проверить:

```text
mouse
wheel
keyboard
window resize
HiDPI
```

и добавить desktop-specific shortcuts:

```text
Ctrl+Z
Ctrl+Shift+Z
Ctrl+S
F
```

---

# 119. Этап 26 — Preparation for Cloud

Добавить:

```text
revision
version
deviceId
syncState
change log
```

Но **не реализовывать сервер преждевременно**.

Сначала должен стабилизироваться локальный model/storage API.

---

# 120. Приоритет MVP

Для первой рабочей версии я бы жестко ограничил MVP следующим набором:

```text
1. Create Map
2. Fixed Grid
3. Infinite Grid
4. SQLite
5. Tags
6. Multi-tag cell rendering
7. Note
8. Tap
9. Long Press
10. Pan
11. Zoom
12. Viewport Culling
13. Undo/Redo
14. Android
15. Basic Windows/Linux
```

Game Mode оставить следующим крупным этапом.

---

# 121. Что нельзя делать в MVP архитектуре

Не рекомендуется:

### 1. QML-модель на миллион объектов

```qml
Repeater {
    model: hugeModel
}
```

### 2. SQLite напрямую из QML

```qml
database.query(...)
```

### 3. Game rules в QML

```qml
if (region.complete)
```

### 4. Хранить все пустые клетки

Особенно для infinite grid.

### 5. Сделать одну огромную `MapManager`

Это приведет к архитектурному монолиту.

### 6. Привязывать core к Android

Например:

```cpp
#ifdef Q_OS_ANDROID
```

в Domain Layer — плохой архитектурный признак.

---

# 122. Рекомендуемая итоговая архитектура

Итоговая система:

```text
                         ┌──────────────┐
                         │     QML      │
                         │     UI       │
                         └──────┬───────┘
                                │
                         ┌──────▼───────┐
                         │ Presentation │
                         │ ViewModels   │
                         └──────┬───────┘
                                │
                         ┌──────▼───────┐
                         │ Application  │
                         │  Use Cases   │
                         └──────┬───────┘
                                │
                ┌───────────────▼──────────────┐
                │           Domain             │
                │                              │
                │ Map Cell Tag Region Rules    │
                └───────────────┬──────────────┘
                                │
                   ┌────────────▼────────────┐
                   │ Repository Interfaces  │
                   └────────────┬────────────┘
                                │
                  ┌─────────────▼─────────────┐
                  │      Infrastructure      │
                  │                           │
                  │ SQLite / Remote / Files   │
                  └───────────────────────────┘


                         Rendering branch

MapViewModel
      ↓
CameraController
      ↓
VisibleCellProvider
      ↓
RenderData
      ↓
GridRenderer
      ↓
Qt Quick Scene Graph
      ↓
GPU
```

---

# 123. Главные архитектурные решения

### Решение №1

**Ячейка является доменным объектом, но не является UI-объектом.**

---

### Решение №2

**Бесконечная сетка является sparse/virtual grid.**

Пустые клетки физически не создаются.

---

### Решение №3

**Количество отрисованных элементов зависит от viewport, а не от размера карты.**

---

### Решение №4

**SQLite является только реализацией Storage Layer.**

Для Application/Domain SQLite не существует.

---

### Решение №5

**Правила Game Mode находятся в отдельном Strategy/Rules Service.**

---

### Решение №6

**Layout сетки также является Strategy.**

Это позволит добавлять:

```text
Rectangle
Calendar
Timeline
Hex
Custom
```

без переделки Domain.

---

### Решение №7

**Renderer работает с Render Data, а не с полноценными сущностями.**

---

### Решение №8

**UUID + revision/version следует использовать с самого начала.**

Это резко упростит будущую синхронизацию.

---

# 124. Критические технические риски

## Риск №1 — Scene Graph performance

Основная опасность проекта — не SQLite, а неправильная архитектура renderer.

Если сделать:

```text
1 Cell = 1 QML Item
```

производительность начнет резко падать.

---

## Риск №2 — Infinite Grid

Нужно с самого начала определить:

```text
logical coordinate system
```

и не использовать `int`, если предполагаются очень большие координаты.

Лучше:

```cpp
qint64
```

---

## Риск №3 — Zoom

При больших диапазонах zoom могут возникать:

* floating-point precision issues;
* дрожание объектов;
* проблемы с очень большими координатами.

При extreme zoom придется продумать camera origin rebasing.

---

## Риск №4 — Tags

Количество комбинаций:

```text
Tag A
Tag B
Tag C
...
```

может быть огромным.

Поэтому rendering должен работать с компактным представлением только видимой области.

---

## Риск №5 — Game Regions

Произвольные регионы нельзя жестко закладывать как прямоугольники.

Основной источник истины должен быть:

```text
set of cell coordinates
```

---

# 125. Архитектурный принцип, который я считаю самым важным

Этот проект лучше воспринимать не как:

> «Приложение с сеткой»

а как:

> **движок пространственно организованных пользовательских событий.**

То есть:

```text
Grid
+
Cell
+
Tags
+
Content
+
Rules
+
Camera
+
Renderer
```

образуют ядро, а:

```text
Life Calendar
Workout
Habit Tracker
Diary
Game Map
```

являются уже различными конфигурациями или приложениями поверх этого ядра.

Поэтому конечная архитектура должна позволять представить новый сценарий примерно так:

```text
MapConfiguration
+
GridLayout
+
Rules
+
Renderer configuration
```

без переписывания движка.

---

# 126. Рекомендуемый порядок разработки

Если отбросить второстепенные функции, оптимальная последовательность выглядит так:

```text
Core Types
    ↓
Domain Model
    ↓
Rules
    ↓
Repository Interfaces
    ↓
In-Memory Repository
    ↓
Use Cases
    ↓
SQLite
    ↓
Basic QML
    ↓
Grid View
    ↓
Camera
    ↓
Custom Scene Graph Renderer
    ↓
Viewport Culling
    ↓
Tags
    ↓
Cell Editor
    ↓
Infinite Grid
    ↓
Performance Optimization
    ↓
Calendar Layout
    ↓
Game Mode
    ↓
Regions
    ↓
Undo/Redo
    ↓
Export/Import
    ↓
Cloud-ready Sync Layer
```

Это дает возможность уже довольно рано получить работающий продукт, но при этом не загнать проект в архитектурный тупик.

# 127. Итог

В качестве базовой архитектурной формулы проекта рекомендуется использовать:

```text
C++ Domain Core
+
Use Cases
+
Repository Interfaces
+
SQLite Infrastructure
+
Qt/QML Presentation
+
Custom QQuickItem Renderer
+
Viewport Culling
+
Sparse Infinite Grid
+
Strategy-based Grid Layout
+
Strategy-based Map Rules
```

При такой структуре один и тот же код ядра сможет обслуживать:

```text
Life Calendar
Habit Tracker
Diary
Workout Tracker
Progress Map
Game Map
Custom User Map
```

а последующее подключение:

```text
Cloud
REST API
Synchronization
Desktop
Multiple Devices
```

не потребует переписывать доменную модель и UI-логику.
