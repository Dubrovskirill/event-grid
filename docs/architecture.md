# 1. Архитектурные слои

```text
┌────────────────────────────────────────────────────────────┐
│                       QML / UI                             │
│ Screens / Dialogs / Controls / Gesture handlers            │
└─────────────────────────────┬──────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────┐
│                    PRESENTATION                            │
│ ViewModels / QML Models / UI State                        │
└─────────────────────────────┬──────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────┐
│                    APPLICATION                            │
│ Use Cases / Commands / Events / DTO                        │
└─────────────────────────────┬──────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────┐
│                       DOMAIN                              │
│ Map / Cell / Tag / Region / Rules / Layout abstractions    │
└─────────────────────────────┬──────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────┐
│                  INFRASTRUCTURE                           │
│ SQLite / File / Network / Serialization / Cache            │
└────────────────────────────────────────────────────────────┘


Отдельная ветка:

Presentation
      ↓
Rendering
      ↓
Qt Quick Scene Graph / GPU
```

Главное правило зависимостей:

```text
UI → Presentation → Application → Domain
                                   ↑
                                   │
Infrastructure ───────────────────┘
```

То есть `Domain` ничего не знает ни о QML, ни о SQLite.

---

# 2. Главные пакеты UML

```text
domain
├── common
├── map
├── cell
├── tag
├── region
├── rules
└── layout

application
├── commands
├── usecases
├── dto
├── events
└── services

infrastructure
├── database
├── repositories
├── serialization
├── cache
└── sync

presentation
├── viewmodels
├── models
└── controllers

rendering
├── camera
├── viewport
├── grid
├── geometry
└── scenegraph
```

---

# 3. Domain: базовые Value Objects

Я рекомендую не передавать по системе голые `int`, `QString` и `QPoint`.

Используются Value Object.

## 3.1. GridCoordinate

```cpp
struct GridCoordinate
{
    qint64 row = 0;
    qint64 column = 0;

    bool operator==(const GridCoordinate&) const = default;
};
```

Ответственность:

* координата клетки;
* не знает ничего о Map;
* может использоваться как ключ.

---

## 3.2. GridRect

```cpp
struct GridRect
{
    qint64 top;
    qint64 left;
    qint64 bottom;
    qint64 right;

    bool contains(GridCoordinate coordinate) const;

    qint64 width() const;
    qint64 height() const;
};
```

Используется для:

* viewport;
* database queries;
* prefetch;
* culling.

---

## 3.3. MapId

```cpp
class MapId
{
public:
    static MapId generate();

    explicit MapId(const QUuid& value);

    QUuid value() const;

private:
    QUuid m_value;
};
```

Аналогично:

```text
CellId
TagId
RegionId
```

---

# 4. Enumerations

```cpp
enum class GridType
{
    FixedRectangle,
    Infinite,
    Calendar
};
```

```cpp
enum class MapMode
{
    Free,
    Game
};
```

```cpp
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
```

```cpp
enum class CellState
{
    Empty,
    Completed,
    Locked
};
```

---

# 5. Map — центральная Domain Entity

```cpp
class Map
{
public:
    Map(
        MapId id,
        QString name,
        GridType gridType,
        MapMode mode
    );

    MapId id() const;

    const QString& name() const;
    void rename(QString name);

    GridType gridType() const;
    MapMode mode() const;

    const MapConfiguration& configuration() const;
    void updateConfiguration(MapConfiguration configuration);

    QDateTime createdAt() const;
    QDateTime updatedAt() const;
};
```

### Map отвечает за

* идентичность карты;
* название;
* конфигурацию;
* тип карты;
* режим карты;
* временные атрибуты.

### Map НЕ отвечает за

* SQLite;
* рендеринг;
* QML;
* получение клеток;
* определение viewport;
* выполнение SQL.

---

# 6. MapConfiguration

```cpp
struct MapConfiguration
{
    GridType gridType = GridType::FixedRectangle;
    MapMode mode = MapMode::Free;

    qint64 rows = 0;
    qint64 columns = 0;

    bool infinite = false;

    CellUnit cellUnit = CellUnit::Custom;

    qint64 unitSize = 1;

    qint64 initialCompletedCells = 0;
};
```

Можно расширить:

```cpp
struct MapConfiguration
{
    ...
    bool showGridLines = true;
    bool showCoordinates = false;
    bool allowNotes = true;
    bool allowTags = true;
};
```

Но UI-настройки лучше со временем вынести в отдельную `MapAppearance`.

---

# 7. Cell как Aggregate Entity

Ключевое решение:

**Cell — не просто `struct` с полями.**

Она должна контролировать собственные допустимые изменения.

```cpp
class Cell
{
public:
    Cell(
        CellId id,
        MapId mapId,
        GridCoordinate coordinate
    );

    CellId id() const;
    MapId mapId() const;

    GridCoordinate coordinate() const;

    CellState state() const;

    bool isCompleted() const;
    bool isLocked() const;

    void complete();
    void reset();
    void lock();
    void unlock();

    const QString& note() const;
    void setNote(QString note);

    const QDateTime& eventDate() const;
    void setEventDate(QDateTime date);

    const QVector<TagId>& tagIds() const;
    void addTag(TagId id);
    void removeTag(TagId id);

    QDateTime createdAt() const;
    QDateTime updatedAt() const;
};
```

---

# 8. Почему Cell не должна знать Tag

Неправильно:

```cpp
class Cell
{
    QVector<Tag> tags;
};
```

Лучше:

```cpp
class Cell
{
    QVector<TagId> tagIds;
};
```

Причина:

```text
Cell
  ↓
TagId

TagRepository
  ↓
Tag
```

То есть Cell знает, **какие теги назначены**, но не обязана загружать весь объект Tag.

Это особенно важно для рендера.

---

# 9. Tag

```cpp
class Tag
{
public:
    Tag(
        TagId id,
        MapId mapId,
        QString name,
        QColor color
    );

    TagId id() const;
    MapId mapId() const;

    QString name() const;
    QColor color() const;

    void rename(QString name);
    void setColor(QColor color);
};
```

Tag принадлежит конкретной карте:

```text
Map
 │
 ├── Tag A
 ├── Tag B
 └── Tag C
```

---

# 10. Region

Регион является отдельной Entity.

```cpp
class Region
{
public:
    Region(
        RegionId id,
        MapId mapId,
        QString name
    );

    RegionId id() const;
    MapId mapId() const;

    QString name() const;
    void rename(QString name);

    bool isUnlocked() const;
    bool isCompleted() const;

    const QSet<GridCoordinate>& cells() const;

    void addCell(GridCoordinate coordinate);
    void removeCell(GridCoordinate coordinate);

    void unlock();
    void complete();
};
```

---

# 11. Почему Region хранит координаты, а не Cell*

Необходимо избежать:

```text
Region
 ↓
100 Cell objects
 ↓
Cell
 ↓
Region
```

что создает сложную связанность.

Вместо этого:

```text
Region
 ↓
GridCoordinate
```

А реальные клетки находятся отдельно.

```text
Region
   │
   ├── (10,20)
   ├── (10,21)
   ├── (11,21)
   └── (12,21)

CellRepository
   ↓
Cell data
```

---

# 12. Region dependencies

```cpp
class RegionDependency
{
public:
    RegionId regionId() const;
    RegionId requiredRegionId() const;
};
```

Repository:

```cpp
class IRegionDependencyRepository
{
public:
    virtual ~IRegionDependencyRepository() = default;

    virtual QVector<RegionId> dependencies(
        RegionId regionId
    ) = 0;
};
```

Игра может использовать граф:

```text
Region A ───────┐
                ▼
             Region C
                ▲
Region B ───────┘
```

---

# 13. Domain Service: RegionProgressService

Проверять completion региона лучше не внутри Region.

```cpp
class RegionProgressService
{
public:
    RegionProgress calculate(
        const Region& region,
        const ICellStateReader& cells
    ) const;
};
```

Результат:

```cpp
struct RegionProgress
{
    int totalCells = 0;
    int completedCells = 0;

    double percentage = 0.0;

    bool completed = false;
};
```

---

# 14. Map Rules

```cpp
class IMapRules
{
public:
    virtual ~IMapRules() = default;

    virtual bool canModifyCell(
        const Map& map,
        const Cell& cell
    ) const = 0;

    virtual bool canAccessRegion(
        const Map& map,
        const Region& region
    ) const = 0;
};
```

---

# 15. FreeMapRules

```cpp
class FreeMapRules : public IMapRules
{
public:
    bool canModifyCell(
        const Map&,
        const Cell&
    ) const override
    {
        return true;
    }

    bool canAccessRegion(
        const Map&,
        const Region&
    ) const override
    {
        return true;
    }
};
```

---

# 16. GameMapRules

```cpp
class GameMapRules : public IMapRules
{
public:
    bool canModifyCell(
        const Map& map,
        const Cell& cell
    ) const override;

    bool canAccessRegion(
        const Map& map,
        const Region& region
    ) const override;
};
```

Но `GameMapRules` не должен сам ходить в SQLite.

Если ему нужны данные:

```cpp
class IGameStateReader
{
public:
    virtual ~IGameStateReader() = default;

    virtual RegionProgress regionProgress(
        RegionId id
    ) const = 0;

    virtual bool isRegionUnlocked(
        RegionId id
    ) const = 0;
};
```

---

# 17. Grid Layout Strategy

Сетка не должна быть частью `Map`.

Создаем:

```cpp
class IGridLayoutStrategy
{
public:
    virtual ~IGridLayoutStrategy() = default;

    virtual QRectF cellRect(
        GridCoordinate coordinate
    ) const = 0;

    virtual GridCoordinate coordinateAt(
        QPointF worldPosition
    ) const = 0;

    virtual QRectF mapBounds() const = 0;

    virtual bool contains(
        GridCoordinate coordinate
    ) const = 0;
};
```

---

# 18. RectangleGridLayout

```cpp
class RectangleGridLayout : public IGridLayoutStrategy
{
public:
    RectangleGridLayout(
        GridDimensions dimensions,
        QSizeF cellSize
    );

    QRectF cellRect(
        GridCoordinate coordinate
    ) const override;

    GridCoordinate coordinateAt(
        QPointF worldPosition
    ) const override;

    QRectF mapBounds() const override;

    bool contains(
        GridCoordinate coordinate
    ) const override;
};
```

---

# 19. InfiniteGridLayout

```cpp
class InfiniteGridLayout : public IGridLayoutStrategy
{
public:
    explicit InfiniteGridLayout(
        QSizeF cellSize
    );

    QRectF cellRect(
        GridCoordinate coordinate
    ) const override;

    GridCoordinate coordinateAt(
        QPointF worldPosition
    ) const override;

    QRectF mapBounds() const override;

    bool contains(
        GridCoordinate
    ) const override;
};
```

Для infinite map:

```cpp
QRectF mapBounds() const
{
    return {};
}
```

То есть границ нет.

---

# 20. CalendarGridLayout

```cpp
class CalendarGridLayout : public IGridLayoutStrategy
{
public:
    CalendarGridLayout(
        QDate startDate,
        CalendarConfiguration configuration
    );

    QRectF cellRect(
        GridCoordinate coordinate
    ) const override;

    GridCoordinate coordinateAt(
        QPointF worldPosition
    ) const override;
};
```

Дополнительно:

```cpp
class CalendarMapper
{
public:
    GridCoordinate dateToCoordinate(QDate date) const;
    QDate coordinateToDate(GridCoordinate coordinate) const;
};
```

---

# 21. Domain UML — основные сущности

```text
┌──────────────────────┐
│        Map           │
├──────────────────────┤
│ MapId id             │
│ QString name         │
│ GridType gridType    │
│ MapMode mode         │
│ MapConfiguration cfg │
└──────────┬───────────┘
           │
           │ owns configuration
           ▼
┌──────────────────────┐
│ MapConfiguration     │
└──────────────────────┘


Map 1 ─────────────── * Cell

Map 1 ─────────────── * Tag

Map 1 ─────────────── * Region

Cell * ────────────── * Tag
          via TagId

Region * ──────────── * Cell
       via coordinates
```

---

# 22. Application Layer

Здесь находятся реальные пользовательские операции.

Главное правило:

```text
Controller ≠ UseCase
```

Controller принимает UI-событие.

Use Case выполняет бизнес-операцию.

---

# 23. CreateMapUseCase

```cpp
class CreateMapUseCase
{
public:
    CreateMapUseCase(
        IMapRepository& mapRepository
    );

    Result<MapId> execute(
        const CreateMapCommand& command
    );

private:
    IMapRepository& m_mapRepository;
};
```

Command:

```cpp
struct CreateMapCommand
{
    QString name;
    MapConfiguration configuration;
};
```

---

# 24. ToggleCellUseCase

```cpp
class ToggleCellUseCase
{
public:
    ToggleCellUseCase(
        IMapRepository& mapRepository,
        ICellRepository& cellRepository,
        IMapRules& rules
    );

    Result<void> execute(
        MapId mapId,
        GridCoordinate coordinate
    );
};
```

Алгоритм:

```text
MapRepository
     ↓
Map

CellRepository
     ↓
Cell

MapRules
     ↓
canModify?

       ↓ yes

Cell.toggle
     ↓
save
     ↓
publish CellChangedEvent
```

---

# 25. EditCellUseCase

```cpp
struct EditCellCommand
{
    MapId mapId;
    GridCoordinate coordinate;

    QString note;
    QDateTime date;

    QVector<TagId> tags;
};
```

```cpp
class EditCellUseCase
{
public:
    Result<void> execute(
        const EditCellCommand& command
    );
};
```

---

# 26. AssignTagUseCase

```cpp
class AssignTagUseCase
{
public:
    Result<void> execute(
        CellId cellId,
        TagId tagId
    );
};
```

Этот Use Case отвечает именно за связь:

```text
Cell ↔ Tag
```

---

# 27. CompleteRegionUseCase

```cpp
class CompleteRegionUseCase
{
public:
    Result<void> execute(
        RegionId regionId
    );
};
```

На практике часть работы будет происходить автоматически после изменения клетки.

---

# 28. Cell change pipeline

```text
User Tap
   │
   ▼
MapViewModel
   │
   ▼
ToggleCellUseCase
   │
   ├── MapRepository
   ├── CellRepository
   ├── IMapRules
   │
   ▼
Cell changed
   │
   ├── CellChangedEvent
   │
   └── RegionProgressService
          │
          └── RegionCompletedEvent
```

---

# 29. Application Events

Создать event types:

```cpp
struct CellChangedEvent
{
    MapId mapId;
    GridCoordinate coordinate;
};
```

```cpp
struct TagChangedEvent
{
    TagId tagId;
};
```

```cpp
struct MapChangedEvent
{
    MapId mapId;
};
```

```cpp
struct RegionCompletedEvent
{
    MapId mapId;
    RegionId regionId;
};
```

```cpp
struct RegionUnlockedEvent
{
    MapId mapId;
    RegionId regionId;
};
```

---

# 30. Event Bus

```cpp
class IEventBus
{
public:
    virtual ~IEventBus() = default;

    template<typename Event, typename Handler>
    Subscription subscribe(Handler handler);

    template<typename Event>
    void publish(const Event& event);
};
```

В production можно реализовать типизированный event dispatcher.

Важно: Event Bus не должен превращаться в глобальный singleton, доступный отовсюду.

Лучше передавать его через dependency injection.

---

# 31. Repository Interfaces

## Map

```cpp
class IMapRepository
{
public:
    virtual ~IMapRepository() = default;

    virtual std::optional<Map> find(
        MapId id
    ) = 0;

    virtual QVector<Map> findAll() = 0;

    virtual void save(
        const Map& map
    ) = 0;

    virtual void remove(
        MapId id
    ) = 0;
};
```

---

# 32. Cell Repository

Ключевой метод:

```cpp
virtual QVector<Cell> findInArea(
    MapId mapId,
    GridRect area
) = 0;
```

Но для rendering лучше позже заменить передачу тяжелых объектов на специализированный read API:

```cpp
class ICellReadRepository
{
public:
    virtual QVector<CellSnapshot> findSnapshots(
        MapId mapId,
        GridRect area
    ) = 0;
};
```

---

# 33. CellSnapshot

```cpp
struct CellSnapshot
{
    GridCoordinate coordinate;

    bool completed = false;

    QVector<TagId> tags;

    bool hasNote = false;

    QDateTime eventDate;
};
```

Snapshot предназначен для чтения и renderer.

Это важное разделение:

```text
Cell
  = Domain Entity

CellSnapshot
  = Read Model / DTO
```

---

# 34. Tag Repository

```cpp
class ITagRepository
{
public:
    virtual ~ITagRepository() = default;

    virtual std::optional<Tag> find(
        TagId id
    ) = 0;

    virtual QVector<Tag> findByMap(
        MapId mapId
    ) = 0;

    virtual void save(
        const Tag& tag
    ) = 0;

    virtual void remove(
        TagId id
    ) = 0;
};
```

---

# 35. Region Repository

```cpp
class IRegionRepository
{
public:
    virtual ~IRegionRepository() = default;

    virtual std::optional<Region> find(
        RegionId id
    ) = 0;

    virtual QVector<Region> findByMap(
        MapId mapId
    ) = 0;

    virtual QVector<Region> findContaining(
        MapId mapId,
        GridCoordinate coordinate
    ) = 0;

    virtual void save(
        const Region& region
    ) = 0;
};
```

---

# 36. Storage Abstraction

Вместо того чтобы раздавать repositories по всему приложению, удобно иметь Unit of Work:

```cpp
class IUnitOfWork
{
public:
    virtual ~IUnitOfWork() = default;

    virtual IMapRepository& maps() = 0;
    virtual ICellRepository& cells() = 0;
    virtual ITagRepository& tags() = 0;
    virtual IRegionRepository& regions() = 0;

    virtual void begin() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};
```

---

# 37. SQLite implementation

```text
IUnitOfWork
      ▲
      │
SQLiteUnitOfWork
      │
      ├── SQLiteMapRepository
      ├── SQLiteCellRepository
      ├── SQLiteTagRepository
      └── SQLiteRegionRepository
```

---

# 38. SQLiteDatabase

```cpp
class SQLiteDatabase
{
public:
    explicit SQLiteDatabase(
        QString databasePath
    );

    void open();
    void close();

    QSqlDatabase connection();

    void execute(QString sql);
};
```

Но бизнес-код не должен получать `QSqlDatabase`.

---

# 39. SQLite Repository

```cpp
class SQLiteCellRepository : public ICellRepository
{
public:
    explicit SQLiteCellRepository(
        SQLiteDatabase& database
    );

    ...
};
```

Слой:

```text
Application
   ↓
ICellRepository
   ↓
SQLiteCellRepository
   ↓
SQLiteDatabase
   ↓
SQLite
```

---

# 40. Presentation Layer

Presentation получает Application services и предоставляет их QML.

Основные ViewModel:

```text
MapsViewModel
MapViewModel
CellEditorViewModel
TagManagerViewModel
RegionViewModel
```

---

# 41. MapsViewModel

```cpp
class MapsViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        int count
        READ count
        NOTIFY countChanged
    )

public:
    Q_INVOKABLE void createMap(...);
    Q_INVOKABLE void deleteMap(QString id);

signals:
    void mapCreated(QString id);
    void mapDeleted(QString id);
};
```

Он не должен выполнять SQL.

---

# 42. MapViewModel

```cpp
class MapViewModel : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void toggleCell(
        qint64 row,
        qint64 column
    );

    Q_INVOKABLE void openCellEditor(
        qint64 row,
        qint64 column
    );

signals:
    void cellChanged(
        qint64 row,
        qint64 column
    );

    void editCellRequested(
        qint64 row,
        qint64 column
    );
};
```

---

# 43. Почему MapViewModel не должен хранить всю карту

Не нужно:

```cpp
QVector<Cell> m_allCells;
```

для огромной карты.

ViewModel работает с:

```text
current map
current camera
visible data
```

а не со всей картой.

---

# 44. Rendering Layer

Это отдельный подсистемный блок.

```text
MapViewModel
      ↓
GridController
      ↓
CameraController
      ↓
ViewportController
      ↓
VisibleCellProvider
      ↓
RenderData
      ↓
GridRenderer
      ↓
QSGNode
```

---

# 45. CameraController

```cpp
class CameraController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        qreal zoom
        READ zoom
        NOTIFY zoomChanged
    )

public:
    QPointF offset() const;
    qreal zoom() const;

    void pan(QPointF delta);

    void zoomAt(
        qreal factor,
        QPointF screenPosition
    );

    void fitToBounds(
        QRectF worldBounds,
        QSizeF viewportSize
    );

    QPointF screenToWorld(
        QPointF screen
    ) const;

    QPointF worldToScreen(
        QPointF world
    ) const;
};
```

---

# 46. CameraController — критически важный класс

Он является единой точкой преобразования:

```text
Screen
  ↕
World
  ↕
Grid
```

То есть:

```text
screen position
   ↓
CameraController
   ↓
world position
   ↓
GridLayoutStrategy
   ↓
GridCoordinate
```

Это предотвращает дублирование математики в QML, ViewModel и Renderer.

---

# 47. ViewportController

```cpp
class ViewportController
{
public:
    GridRect visibleGrid(
        const CameraState& camera,
        const IGridLayoutStrategy& layout,
        QSizeF viewportSize
    ) const;

    GridRect prefetchGrid(
        const GridRect& visible,
        int margin
    ) const;
};
```

---

# 48. VisibleCellProvider

```cpp
class VisibleCellProvider
{
public:
    QVector<CellSnapshot> load(
        MapId mapId,
        GridRect visibleArea
    );
};
```

Но для infinite grid используется sparse retrieval:

```text
viewport
    ↓
GridRect
    ↓
SQLite query
    ↓
only existing cells
```

Пустые клетки вообще не возвращаются из БД.

---

# 49. RenderDataBuilder

Этот класс преобразует domain/read model в то, что нужен GPU.

```cpp
class CellRenderDataBuilder
{
public:
    CellRenderData build(
        const CellSnapshot& cell,
        const QVector<Tag>& tags
    ) const;
};
```

---

# 50. CellRenderData

```cpp
struct CellRenderData
{
    GridCoordinate coordinate;

    CellVisualState state;

    QVector<CellSegment> segments;

    bool hasNote = false;
};
```

---

# 51. CellSegment

```cpp
struct CellSegment
{
    QColor color;

    float startAngle = 0.0f;
    float endAngle = 0.0f;
};
```

Например:

```text
2 tags

Segment 1:
0° → 180°

Segment 2:
180° → 360°
```

---

# 52. GridRenderer

```cpp
class GridRenderer
{
public:
    void setCells(
        QVector<CellRenderData> data
    );

    void render(
        QSGNode* parent,
        const CameraState& camera
    );
};
```

Но в Qt Quick корректнее реализовать интеграцию через собственный `QQuickItem`.

---

# 53. InteractiveGrid

```cpp
class InteractiveGrid : public QQuickItem
{
    Q_OBJECT

public:
    explicit InteractiveGrid(
        QQuickItem* parent = nullptr
    );

protected:
    QSGNode* updatePaintNode(
        QSGNode* oldNode,
        UpdatePaintNodeData*
    ) override;

    void mousePressEvent(
        QMouseEvent*
    ) override;

    void mouseMoveEvent(
        QMouseEvent*
    ) override;

    void touchEvent(
        QTouchEvent*
    ) override;
};
```

Однако я бы не превращал `InteractiveGrid` в God Object.

---

# 54. InteractiveGrid должен делегировать

```text
InteractiveGrid
│
├── CameraController
├── GestureController
├── ViewportController
└── GridRenderer
```

Тогда:

```cpp
InteractiveGrid
{
    CameraController m_camera;
    GestureController m_gestures;
    ViewportController m_viewport;
    GridRenderer m_renderer;
};
```

---

# 55. GestureController

```cpp
enum class GestureState
{
    Idle,
    Pressing,
    Tapping,
    LongPressing,
    Panning,
    Pinching
};
```

```cpp
class GestureController
{
public:
    void press(QPointF position);
    void move(QPointF position);
    void release(QPointF position);

    GestureState state() const;
};
```

Signals:

```text
tap
longPress
pan
pinch
```

---

# 56. Rendering UML

```text
┌───────────────────────┐
│    InteractiveGrid    │
└───────────┬───────────┘
            │
     ┌──────┼───────────────┐
     ▼      ▼               ▼
┌────────┐ ┌──────────────┐ ┌─────────────────┐
│ Camera │ │   Gesture    │ │    Renderer     │
│Controller│ │ Controller │ │                 │
└────┬───┘ └──────────────┘ └────────┬────────┘
     │                               │
     ▼                               ▼
┌──────────────┐              ┌───────────────┐
│   Viewport   │              │ Scene Graph   │
│ Controller   │              │ QSGGeometry   │
└──────┬───────┘              └───────────────┘
       │
       ▼
┌────────────────────┐
│ VisibleCellProvider│
└─────────┬──────────┘
          ▼
      CellSnapshot
```

---

# 57. Основной Data Flow

При открытии карты:

```text
MapsScreen
   ↓
MapViewModel
   ↓
GetMapUseCase
   ↓
IMapRepository
   ↓
SQLite
```

При отображении:

```text
MapViewModel
   ↓
InteractiveGrid
   ↓
Camera
   ↓
Viewport
   ↓
VisibleCellProvider
   ↓
ICellReadRepository
   ↓
SQLite
```

---

# 58. Tap Sequence Diagram

```text
User
 │
 │ tap
 ▼
InteractiveGrid
 │
 │ grid coordinate
 ▼
MapViewModel
 │
 │ toggleCell(row,col)
 ▼
ToggleCellUseCase
 │
 ├──► IMapRepository
 │
 ├──► ICellRepository
 │
 ├──► IMapRules
 │
 └──► IUnitOfWork
          │
          ▼
       SQLite
          │
          ▼
       commit
 │
 ▼
IEventBus
 │
 ▼
CellChangedEvent
 │
 ├──────────────► MapViewModel
 │
 └──────────────► VisibleCellProvider
                         │
                         ▼
                       Renderer
```

---

# 59. Long Press Sequence

```text
User
 │
 │ hold
 ▼
InteractiveGrid
 │
 ▼
GestureController
 │
 │ timeout
 ▼
longPress
 │
 ▼
MapViewModel
 │
 ▼
CellEditor
 │
 ▼
CellEditorViewModel
```

---

# 60. Save Cell Sequence

```text
CellEditorViewModel
          │
          ▼
    EditCellUseCase
          │
          ├──► CellRepository
          │
          ├──► TagRepository
          │
          └──► UnitOfWork
                    │
                    ▼
                  commit
                    │
                    ▼
              CellChangedEvent
```

---

# 61. Game Mode Sequence

```text
User taps cell
       │
       ▼
ToggleCellUseCase
       │
       ▼
GameMapRules
       │
       ├── locked?
       │
       └── unlocked?
              │
              ▼
        Cell modification
              │
              ▼
     RegionProgressService
              │
              ▼
      region = 100%
              │
              ▼
      RegionCompletedEvent
              │
              ▼
    RegionUnlockService
              │
              ▼
      RegionUnlockedEvent
              │
       ┌──────┴───────┐
       ▼              ▼
     ViewModel      Animation
```

---

# 62. RegionUnlockService

```cpp
class RegionUnlockService
{
public:
    QVector<RegionId> unlockAvailableRegions(
        MapId mapId
    );
};
```

Он проверяет:

```text
Region.dependencies
        ↓
completed?
        ↓
unlock
```

---

# 63. Commands для Undo/Redo

```cpp
class ICommand
{
public:
    virtual ~ICommand() = default;

    virtual Result<void> execute() = 0;
    virtual Result<void> undo() = 0;
};
```

Примеры:

```cpp
class ToggleCellCommand : public ICommand;
class EditCellCommand : public ICommand;
class AssignTagCommand : public ICommand;
class RemoveTagCommand : public ICommand;
```

---

# 64. CommandManager

```cpp
class CommandManager
{
public:
    Result<void> execute(
        std::unique_ptr<ICommand> command
    );

    Result<void> undo();
    Result<void> redo();

    bool canUndo() const;
    bool canRedo() const;
};
```

UI:

```qml
Button {
    enabled: commandManager.canUndo
}
```

---

# 65. Read Model для карты

Для списка карт не нужно загружать полноценные `Map`.

Создаем:

```cpp
struct MapListItem
{
    MapId id;
    QString name;

    int completedCells = 0;
    int totalCells = 0;

    double progress = 0.0;

    QDateTime lastModified;
};
```

Repository:

```cpp
QVector<MapListItem> findMapList();
```

Это разновидность CQRS-подхода:

```text
Write Model
    ↓
Domain Entities

Read Model
    ↓
optimized DTOs
```

Полный CQRS здесь не нужен, но разделение read/write очень полезно.

---

# 66. DTO / Query Models

Рекомендуемый набор:

```text
MapDto
MapListItem
CellSnapshot
CellRenderData
RegionSnapshot
TagSnapshot
```

Разделение:

```text
Entity
↓
DTO
↓
Presentation
```

---

# 67. Storage Architecture

```text
                 ┌──────────────────┐
                 │   Application     │
                 └────────┬─────────┘
                          │
                   repository API
                          │
                ┌─────────▼─────────┐
                │ Repository Layer   │
                └─────────┬─────────┘
                          │
             ┌────────────┴────────────┐
             ▼                         ▼
┌────────────────────────┐  ┌──────────────────────┐
│ SQLite repositories    │  │ Remote repositories  │
│                        │  │ future               │
└────────────┬───────────┘  └──────────┬───────────┘
             ▼                         ▼
          SQLite                   REST/API
```

---

# 68. Sync Architecture

Когда появится облако:

```text
                  Application
                       │
                  Local storage
                       │
                 Sync Engine
                  /         \
                 /           \
           Pull changes    Push changes
                │               │
                └──────┬────────┘
                       ▼
                  Remote API
```

---

# 69. Change Record

Для будущей синхронизации можно заложить:

```cpp
struct ChangeRecord
{
    QUuid changeId;

    QUuid deviceId;

    qint64 sequence;

    EntityType entityType;

    QUuid entityId;

    ChangeOperation operation;

    QDateTime timestamp;
};
```

---

# 70. Entity version

Каждая синхронизируемая entity в будущем может иметь:

```cpp
struct VersionInfo
{
    qint64 version = 0;

    QUuid lastModifiedBy;

    QDateTime modifiedAt;
};
```

Сейчас можно просто хранить:

```text
version
updated_at
```

---

# 71. Conflict Resolver

В будущем:

```cpp
class IConflictResolver
{
public:
    virtual ResolutionResult resolve(
        const LocalChange& local,
        const RemoteChange& remote
    ) = 0;
};
```

Но этот интерфейс **не нужен для MVP реализации**, его достаточно предусмотреть на уровне архитектурных границ.

---

# 72. Dependency Injection

Composition Root:

```cpp
class ApplicationCompositionRoot
{
public:
    void build();
};
```

Пример:

```text
SQLiteDatabase
      ↓
SQLiteUnitOfWork
      ↓
Repositories
      ↓
UseCases
      ↓
ViewModels
      ↓
QML
```

---

# 73. Конкретная цепочка зависимостей

```text
main.cpp
   │
   ▼
ApplicationCompositionRoot
   │
   ├── SQLiteDatabase
   ├── SQLiteUnitOfWork
   ├── FreeMapRules
   ├── GameMapRules
   ├── EventBus
   ├── Use Cases
   ├── ViewModels
   └── QML engine
```

`main.cpp` должен быть одним из немногих мест, где известны конкретные реализации.

---

# 74. Что может знать каждый слой

## Domain знает

```text
Map
Cell
Tag
Region
Rules
Grid abstractions
Value Objects
```

## Domain не знает

```text
QML
QObject
SQLite
QSqlDatabase
REST
Android
```

---

## Application знает

```text
Domain
Repository interfaces
Events
Commands
DTO
```

Но не знает:

```text
SQLite
QML controls
Android
```

---

## Infrastructure знает

```text
SQLite
Qt SQL
JSON
HTTP
filesystem
```

---

## Presentation знает

```text
QObject
Q_PROPERTY
Q_INVOKABLE
Application services
DTO
```

---

## Rendering знает

```text
Qt Quick
QQuickItem
QSGNode
QSGGeometry
Camera
RenderData
```

Но не знает:

```text
SQLite
MapRepository
GameRules
```

---

# 75. Полная UML-связь

```text
                           ┌───────────────┐
                           │      Map      │
                           └───────┬───────┘
                                   │
                 ┌─────────────────┼───────────────────┐
                 │                 │                   │
                 ▼                 ▼                   ▼
             ┌───────┐         ┌───────┐          ┌────────┐
             │ Cell  │         │  Tag  │          │ Region │
             └───┬───┘         └───────┘          └───┬────┘
                 │                                     │
                 │ TagId                               │
                 └─────────────────┐                   │
                                   ▼                   │
                                 Tag                    │
                                                       │
                                               coordinates
                                                       │
                                                       ▼
                                                     Cell


Map
 │
 ├── MapConfiguration
 ├── GridType
 └── MapMode


Map
 │
 └── IGridLayoutStrategy
         │
         ├── RectangleGridLayout
         ├── InfiniteGridLayout
         └── CalendarGridLayout
```

---

# 76. Dependency UML

```text
┌───────────────────────────┐
│       Presentation        │
│                           │
│ MapViewModel              │
│ CellEditorViewModel       │
└─────────────┬─────────────┘
              │
              ▼
┌───────────────────────────┐
│       Application         │
│                           │
│ UseCases                  │
│ CommandManager            │
│ EventBus                  │
└─────────────┬─────────────┘
              │
              ▼
┌───────────────────────────┐
│          Domain           │
│                           │
│ Map / Cell / Tag / Region │
│ Rules / Layout interfaces │
└─────────────▲─────────────┘
              │
              │ implements
┌─────────────┴─────────────┐
│      Infrastructure       │
│                           │
│ SQLiteRepositories        │
│ SQLiteUnitOfWork          │
└───────────────────────────┘
```

---

# 77. Отдельная архитектура Rendering

```text
                       QML
                        │
                        ▼
                ┌───────────────┐
                │ InteractiveGrid│
                └───────┬───────┘
                        │
        ┌───────────────┼────────────────┐
        ▼               ▼                ▼
┌──────────────┐ ┌───────────────┐ ┌───────────────┐
│GestureControl│ │CameraController│ │GridRenderer  │
└──────────────┘ └───────┬────────┘ └───────┬───────┘
                         │                  │
                         ▼                  ▼
                 ViewportController   CellRenderData
                         │                  │
                         ▼                  ▼
                 VisibleCellProvider    QSGGeometry
                         │
                         ▼
                 CellReadRepository
```

---

# 78. Почему Rendering выделен отдельно

Это принципиально.

Например, в будущем может понадобиться:

```text
Qt Quick
      ↓
OpenGL
```

или:

```text
Qt Quick
      ↓
RHI / Vulkan
```

или desktop-specific optimization.

Если renderer изолирован от Domain, это не затрагивает бизнес-логику.

---

# 79. Необходимый набор интерфейсов

Минимальный список:

```text
IMapRepository
ICellRepository
ITagRepository
IRegionRepository

IUnitOfWork
IEventBus

IMapRules

IGridLayoutStrategy

ISyncService        // future
IRemoteStorage      // future
IConflictResolver   // future
```

---

# 80. Необходимый набор сущностей

```text
Map
Cell
Tag
Region

MapConfiguration

GridCoordinate
GridRect
GridDimensions
```

---

# 81. Необходимый набор сервисов

```text
RegionProgressService
RegionUnlockService
CellRenderDataBuilder
VisibleCellProvider
ViewportController
CameraController
GestureController
CommandManager
```

---

# 82. Необходимый набор Use Cases

```text
CreateMapUseCase
UpdateMapUseCase
DeleteMapUseCase

CreateTagUseCase
UpdateTagUseCase
DeleteTagUseCase

ToggleCellUseCase
EditCellUseCase
AssignTagUseCase
RemoveTagUseCase

CreateRegionUseCase
UpdateRegionUseCase
DeleteRegionUseCase

UndoUseCase
RedoUseCase

ExportMapUseCase
ImportMapUseCase
```

---

# 83. Очень важное разделение: Map ≠ Grid

Нельзя делать:

```cpp
class Map
{
    QVector<Cell> cells;
};
```

для infinite/large maps.

Лучше:

```text
Map
 │
 ├── configuration
 │
 └── metadata

CellRepository
 │
 └── sparse cells
```

Это одно из ключевых решений всей архитектуры.

---

# 84. Очень важное разделение: Domain Cell ≠ Render Cell

```text
Cell
 ├── note
 ├── tagIds
 ├── date
 └── state

        ↓ transform

CellRenderData
 ├── geometry
 ├── sectors
 ├── colors
 └── visual state
```

Это позволяет Renderer не таскать за собой Domain.

---

# 85. Очень важное разделение: Input ≠ Business Logic

Неправильно:

```text
QML:
if locked then reject
```

Правильно:

```text
QML
 ↓
Input
 ↓
ViewModel
 ↓
UseCase
 ↓
Rules
```

---

# 86. Один конкретный пример

Пусть пользователь нажал:

```text
row = 15
column = 27
```

Путь:

```text
Touch
 ↓
InteractiveGrid
 ↓
GestureController
 ↓
GridCoordinate(15,27)
 ↓
MapViewModel::toggleCell()
 ↓
ToggleCellUseCase
 ↓
ICellRepository::findByCoordinate()
 ↓
Cell
 ↓
IMapRules::canModifyCell()
 ↓
Cell::complete()
 ↓
IUnitOfWork::commit()
 ↓
CellChangedEvent
 ↓
VisibleCellProvider
 ↓
CellRenderDataBuilder
 ↓
GridRenderer
 ↓
QSGGeometry
 ↓
GPU
```

Это и есть основная end-to-end цепочка проекта.

---

# 87. Рекомендуемая файловая структура

```text
src/
│
├── domain/
│   ├── common/
│   │   ├── MapId.h
│   │   ├── CellId.h
│   │   ├── TagId.h
│   │   ├── RegionId.h
│   │   ├── GridCoordinate.h
│   │   └── GridRect.h
│   │
│   ├── map/
│   │   ├── Map.h
│   │   ├── Map.cpp
│   │   ├── MapConfiguration.h
│   │   └── MapTypes.h
│   │
│   ├── cell/
│   │   ├── Cell.h
│   │   ├── Cell.cpp
│   │   └── CellState.h
│   │
│   ├── tag/
│   │   ├── Tag.h
│   │   └── Tag.cpp
│   │
│   ├── region/
│   │   ├── Region.h
│   │   ├── Region.cpp
│   │   └── RegionProgress.h
│   │
│   ├── rules/
│   │   ├── IMapRules.h
│   │   ├── FreeMapRules.h
│   │   └── GameMapRules.h
│   │
│   └── layout/
│       ├── IGridLayoutStrategy.h
│       ├── RectangleGridLayout.h
│       ├── InfiniteGridLayout.h
│       └── CalendarGridLayout.h
│
├── application/
│   ├── commands/
│   ├── dto/
│   ├── events/
│   ├── services/
│   └── usecases/
│
├── infrastructure/
│   ├── database/
│   │   ├── SQLiteDatabase.h
│   │   ├── SQLiteUnitOfWork.h
│   │   └── migrations/
│   │
│   ├── repositories/
│   │   ├── IMapRepository.h
│   │   ├── ICellRepository.h
│   │   ├── ITagRepository.h
│   │   └── IRegionRepository.h
│   │
│   ├── sqlite/
│   │   ├── SQLiteMapRepository.h
│   │   ├── SQLiteCellRepository.h
│   │   ├── SQLiteTagRepository.h
│   │   └── SQLiteRegionRepository.h
│   │
│   └── serialization/
│
├── presentation/
│   ├── viewmodels/
│   │   ├── MapsViewModel.h
│   │   ├── MapViewModel.h
│   │   ├── CellEditorViewModel.h
│   │   └── TagManagerViewModel.h
│   │
│   └── models/
│
├── rendering/
│   ├── camera/
│   ├── gestures/
│   ├── viewport/
│   ├── grid/
│   ├── geometry/
│   └── scenegraph/
│
└── app/
    ├── ApplicationCompositionRoot.h
    └── main.cpp
```

---

# 88. Границы ответственности

| Класс                   | Главная ответственность          |
| ----------------------- | -------------------------------- |
| `Map`                   | состояние и идентичность карты   |
| `Cell`                  | состояние одной клетки           |
| `Tag`                   | описание тега                    |
| `Region`                | состав и состояние региона       |
| `IMapRules`             | разрешение бизнес-операций       |
| `IGridLayoutStrategy`   | геометрия сетки                  |
| `UseCase`               | пользовательская бизнес-операция |
| `Repository`            | persistence                      |
| `ViewModel`             | адаптация Application к QML      |
| `CameraController`      | экран ↔ world                    |
| `ViewportController`    | определение видимой области      |
| `VisibleCellProvider`   | получение данных видимой области |
| `CellRenderDataBuilder` | Domain → GPU-friendly data       |
| `GridRenderer`          | Scene Graph rendering            |
| `GestureController`     | распознавание взаимодействий     |

---

# 89. Что в итоге должно быть singleton, а что нет

Не делать singleton:

```text
Map
Cell
Tag
Region
UseCase
Repository
ViewModel
Camera
```

Допустимо иметь единственный экземпляр на Application Scope:

```text
EventBus
ApplicationCompositionRoot
```

но даже они предпочтительно передаются через DI.

---

# 90. Модель памяти для большой карты

Например:

```text
Карта:
10 000 × 10 000
= 100 000 000 logical cells
```

В памяти:

```text
Map
 └── metadata

SQLite
 └── only changed cells

Viewport
 └── 100 × 70 visible

Render cache
 └── ~7 000 CellRenderData
```

То есть количество логических клеток и количество объектов C++ в памяти никак не обязаны совпадать.

---

# 91. Финальная архитектурная схема

```text
                                   ┌─────────────────────┐
                                   │        QML          │
                                   │ Screens / Dialogs   │
                                   └──────────┬──────────┘
                                              │
                                              ▼
                              ┌────────────────────────────┐
                              │       Presentation         │
                              │                            │
                              │ ViewModels / Controllers   │
                              └─────────────┬──────────────┘
                                            │
                        ┌───────────────────┴───────────────────┐
                        │                                       │
                        ▼                                       ▼
              ┌────────────────────┐                  ┌──────────────────┐
              │    Application     │                  │    Rendering     │
              │                    │                  │                  │
              │ UseCases           │                  │ Camera           │
              │ Commands           │                  │ Viewport         │
              │ Events             │                  │ Gestures         │
              │ Services           │                  │ Renderer         │
              └─────────┬──────────┘                  └────────┬─────────┘
                        │                                      │
                        ▼                                      ▼
              ┌────────────────────┐                  ┌──────────────────┐
              │      Domain       │                  │ Qt Scene Graph   │
              │                    │                  │      / GPU       │
              │ Map                │                  └──────────────────┘
              │ Cell               │
              │ Tag                │
              │ Region             │
              │ Rules              │
              │ Layout             │
              └─────────┬──────────┘
                        │
                        ▼
              ┌────────────────────┐
              │ Repository         │
              │ Interfaces         │
              └─────────┬──────────┘
                        │
              ┌─────────┴─────────┐
              ▼                   ▼
      ┌───────────────┐    ┌────────────────┐
      │ SQLite        │    │ Remote / Cloud │
      │ Infrastructure│    │ Future         │
      └───────────────┘    └────────────────┘
```

# 92. Итоговое проектное решение

Основная архитектурная идея проекта должна быть такой:

```text
                     APPLICATION
                          │
              ┌───────────┴───────────┐
              ▼                       ▼
         DOMAIN MODEL             RENDER MODEL
              │                       │
      Map / Cell / Tag          CellRenderData
      Region / Rules                   │
              │                        ▼
              │                   Scene Graph
              │
              ▼
        REPOSITORIES
              │
       ┌──────┴───────┐
       ▼              ▼
    SQLite          Cloud
```

При этом:

**Domain отвечает за смысл данных.**

**Application отвечает за операции.**

**Infrastructure отвечает за сохранение.**

**Presentation отвечает за связь с QML.**

**Rendering отвечает за визуализацию.**

И самое важное для этого конкретного проекта:

```text
ОГРОМНАЯ КАРТА
      ≠
ОГРОМНОЕ КОЛИЧЕСТВО QML ITEMS
      ≠
ОГРОМНОЕ КОЛИЧЕСТВО C++ OBJECTS
```

Карта должна быть **логически огромной, но физически sparse**, а Renderer должен работать преимущественно с **видимой областью**.

