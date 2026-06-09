# 阶段7：秘密基地、童心情绪、昼夜天气与玩具玩法开发计划

## Context

根据 `doc/技术栈.md`，阶段7原定目标是“秘密基地与玩具”：建造模式、家具系统、玩具收藏图鉴与小游戏、基地与好感联动事件。新的怀旧主题介绍把项目主轴进一步明确为 **“六一儿童节 + 8090/00 后童年怀旧”**，核心冲突从单纯“恋爱建造”升级为 **“童心 vs 长大”**：主角星愿依靠童心值、公主羁绊、童年玩具和 9 大领域记忆，对抗长大王国与童年阴影拟人化的 BOSS。

因此阶段7不再只做家园建造，而要把“战斗掉落金币”扩展为“回到秘密基地、整理童年物件、恢复童心、触发公主事件、再出发冒险”的生活闭环。同时，阶段6已有但仍偏原型的情绪系统、昼夜循环和天气系统，需要在阶段7优化到能服务剧情、角色状态和场景氛围。

当前项目现状：

- 已有：`RegionManager` / `MapRegion` / `MapTileManager` / `TileMap` / `DecorationGen`，可支持独立区域、室内区域和瓦片物理同步。
- 已有：`TimeSystem`，支持 0-24 小时循环、天数、环境光、天空色和时间段判断，但还缺少剧情日程、室内外差异和任务触发。
- 已有：`WeatherSystem`，支持晴天、多云、雨、大雨、雾、雪，以及粒子、能见度、移动速度和光照修正，但还缺少区域/剧情权重、室内过滤、天气预告和主题化特殊天气。
- 已有：`SaveSystem`，包含 `coins`、`completedQuests`、`collectedItems`、区域差异保存、`grievance` 等字段，可扩展保存家具、玩具、童心值、天气时间和任务状态。
- 已有：`DialogueTree`、`Princess`、`EmotionSystem`，可承接基地事件、好感度奖励和委屈宣泄；但当前 `EmotionSystem` 仍是 0-100 的 `grievance/joy/stress`，需要升级为“短期情绪 + 长期童心值”的结构。
- 已有：`Draw2D`、`DialogueUI`、`MiniMap`、粒子和 SDF shader，可继续用纯代码方式绘制家具、玩具和建造 UI。
- 缺失：`Building`、`Toy`、`Inventory`、`Quest`、`Shop/Economy` 独立模块；当前金币主要是战斗掉落和存档字段，还没有消费闭环。
- 缺失：9 位公主的角色数据、童心值 0-1000 主机制、场景/天气/昼夜对情绪和剧情的统一事件接口。

阶段7目标不是一次性做完整模拟经营，而是先形成一个可玩闭环：

1. 玩家进入“秘密基地”室内区域。
2. 玩家用金币购买或解锁家具/玩具，把“童年物件”带回基地。
3. 玩家进入建造模式，把家具按网格放置、移动、拆除。
4. 家具布置可保存/读取，并影响基地舒适度、童心恢复和公主事件。
5. 玩具可收藏、展示，并提供至少一个轻量小游戏或交互。
6. 主角拥有 0-1000 的童心值，旧 `grievance` 转为短期委屈/压力，二者共同影响移动、后处理、对话和结局风险。
7. 昼夜循环、天气系统和基地事件联动：夜晚适合恢复与谈心，雨/雾/雪改变探索体验，特殊天气可服务 9 大领域与 BOSS 机制。

---

## 1. 阶段7功能目标

### 1.1 P0：秘密基地最小闭环

- 新增 `home_base` 室内区域，作为玩家之家、童年收藏室和公主羁绊事件空间。
- 在新手村的 `player_home` POI 附近按 `E` 进入基地；在基地门口按 `E` 返回上一区域。
- 基地使用室内瓦片：墙、地板、门、可放置区域。
- 基地内支持摄像机、小地图或迷你室内图可选关闭。
- 基地内默认暂停或弱化天气粒子，保留窗外光照/雨声/夜色提示，避免室内下雨下雪。

### 1.2 P0：建造模式

- 按 `B` 或在基地内按 `Tab` 进入/退出建造模式。
- 建造模式下：
  - 鼠标显示家具预览。
  - 左键放置/确认移动。
  - 右键取消。
  - `Q/E` 旋转。
  - `Delete` 拆除选中家具。
- 所有家具吸附到网格。
- 家具放置时要进行占用检测，不能穿墙、不能重叠、不能堵住出口。
- 使用 `MapTileManager` 或独立占用层同步物理阻挡，避免视觉和碰撞不同步。
- 建造模式只在 `home_base` 生效，后续阶段再扩展到其他室内场景。

### 1.3 P0：家具数据与渲染

- 家具以 Lua 配置定义：尺寸、价格、分类、占用格、舒适度、童心恢复、好感加成、绘制类型。
- 家具由 C++ 几何和 `Draw2D` 绘制，不引入图片资源。
- 第一批家具：
  - 小木床：休息、短期委屈恢复、夜间童心恢复加成。
  - 书桌：日记、任务入口和 9 大领域线索整理。
  - 花盆：公主好感事件装饰。
  - 星愿灯：夜间光效，降低低童心时的泛灰强度。
  - 玩具架：展示玩具，提升童心上限或每日恢复。
  - 地毯：纯装饰，提高舒适度。
  - 童年海报：对应 9 大领域的收藏墙，后续可绑定章节进度。

### 1.4 P0：童心值与人物情绪系统升级

- 在 `EmotionSystem` 中新增主角长期资源 `childlikeHeart`，范围 0-1000，初始建议 950。
- 保留现有 `grievance` 作为短期委屈/压力，范围仍为 0-100，继续驱动暗角、减速和宣泄动画。
- 童心值规则：
  - 被敌人、BOSS、长大王国事件打击时降低。
  - 完成玩具小游戏、基地休息、公主谈心、主线章节结算时恢复。
  - 低于 200 时启用全屏泛灰、移动速度下降、对话文本偏消极。
  - 归零时先触发阶段7的“坏结局警告/强制回基地”保护流程，真正坏结局留到主线阶段实现。
- 人物情绪规则：
  - 主角：`grievance`、`joy`、`stress` 转为可见调试状态，并与童心值换算后处理强度。
  - 公主：在现有 `Princess::affection` 基础上增加心情枚举或数值，如 `Calm/Happy/Worried/Inspired`，影响基地对话和跟随表现。
  - NPC：阶段7不做完整 NPC 情绪 AI，只提供接口和 Lua 字段，供后续主线扩展。

### 1.5 P1：昼夜循环优化

- 当前 `TimeSystem` 已支持小时、天数、环境光和天空色；阶段7优化重点是“可被玩法读取”。
- 增加时间事件接口：
  - 整点事件：NPC 日程、商店刷新、基地自然恢复。
  - 日切事件：每日童心恢复、每日公主互动次数、天气重新抽样。
  - 时间段事件：清晨、白天、黄昏、夜晚影响对话、灯光、敌人强度。
- 基地内昼夜表现：
  - 夜晚自动增强 `star_lamp` 等灯具亮度。
  - 深夜休息可恢复委屈和少量童心。
  - 黄昏/夜晚更容易触发公主谈心事件。
- 地图探索表现：
  - 夜晚能见度降低但特殊玩具/星愿灯可提供小范围照明。
  - 9 大领域可拥有独立时间倾向，例如校园操场白天明亮、长大魔王城堡长期灰夜。

### 1.6 P1：天气系统优化

- 当前 `WeatherSystem` 已支持天气类型、粒子、能见度、移动速度和光照修正；阶段7优化重点是“区域化、剧情化、可预期”。
- 新增天气配置或权重：
  - 普通野外：晴/多云/雨/雾/雪按权重随机。
  - 室内和基地：不生成雨雪粒子，只保留外部环境提示。
  - 童谣村庄：可出现“噪音风暴/节拍雨”作为第六章噪音公爵伏笔。
  - 长大王国相关区域：灰雾、冷雨、低饱和光照。
- 天气对玩法影响：
  - 雨天降低移动或摩擦，但在基地内听雨可降低 `grievance`。
  - 雾天降低能见度，增强迷路和探索感。
  - 雪天降低速度但提高童话/怀旧氛围，可触发特定公主台词。
  - 大雨/特殊天气可与 BOSS 机制或任务目标绑定。
- 增加天气调试入口：Debug 构建下可切换天气、时间和童心值，方便手动验证。

### 1.7 P1：经济、背包与商店

- 新增金币消费入口：基地商店或快捷家具目录。
- 新增简单 `Inventory`：记录已购买家具数量和已收集玩具。
- 未购买家具不可无限放置；已购买家具放置后占用库存，拆除后返还库存。
- 金币字段复用 `GameState::coinsCollected` 与 `SaveData::player.coins`。
- 商品以“童年物件”为主：红白机、模型车、贴纸、海报、星愿灯等，避免现实氪金语义。

### 1.8 P1：玩具系统

- 新增 `ToySystem`：
  - 收集玩具。
  - 展示到玩具架。
  - 触发互动。
- 首个玩具小游戏建议实现“模型车计时赛”：
  - 在基地地面生成简单赛道。
  - 玩家按方向键控制小车。
  - 计时完成后获得金币或公主好感奖励。
- 玩具数据也由 Lua 配置，便于未来扩展。
- 玩具不只是图鉴条目，应作为童心恢复和章节回忆的入口。

### 1.9 P1：公主与情感联动

- 基地舒适度影响：
  - 公主跟随时进入基地会触发短事件。
  - 舒适度达到阈值提高每日好感收益。
  - 委屈宣泄后恢复效果随基地评分提升。
  - 羁绊技充能可在基地内通过互动获得少量补充。
- 阶段7先保留当前“小夏/单公主”运行链路，同时在数据层按 9 公主扩展，后续逐步替换为琪琪、露露、艾琳等怀旧主题角色。

### 1.10 P2：任务系统最小版

- 新增轻量 `QuestSystem`，先只支持：
  - 已接任务。
  - 已完成任务。
  - 目标计数。
  - 奖励金币/家具/玩具/好感。
- 第一批任务：
  - “整理秘密基地”：放置床、桌、灯。
  - “给公主的花”：放置花盆并与公主对话。
  - “星愿玩具架”：获得玩具架并展示第一个玩具。
  - “雨夜谈心”：雨天夜晚在基地与公主对话，降低委屈并恢复童心。
  - “童心警报”：童心值低于 200 时回基地休息，完成后解除低童心惩罚。

---

## 2. 建议目录结构

新增/修改文件：

```text
src/Game/Building/
  Furniture.h
  Furniture.cpp
  BuildingSystem.h
  BuildingSystem.cpp
  BaseRating.h
  BaseRating.cpp

src/Game/Toy/
  ToySystem.h
  ToySystem.cpp
  MiniCarGame.h
  MiniCarGame.cpp

src/Game/Inventory/
  Inventory.h
  Inventory.cpp

src/Game/Quest/
  QuestSystem.h
  QuestSystem.cpp

src/Game/Emotion/
  EmotionSystem.h          # 修改：新增 childlikeHeart 与人物心情字段
  EmotionSystem.cpp        # 修改：童心/委屈/后处理/恢复规则

src/Game/World/
  TimeSystem.h             # 修改：增加日切、时间段、休息相关接口
  TimeSystem.cpp
  WeatherSystem.h          # 修改：增加区域权重、室内过滤、特殊天气
  WeatherSystem.cpp
  EnvironmentRules.h       # 新增可选：承接时间/天气/区域规则，避免 main.cpp 膨胀
  EnvironmentRules.cpp

assets/scripts/
  furniture.lua
  toys.lua
  quests.lua
  time_events.lua
  weather_config.lua
  childhood_config.lua
```

注意：

- 新增 `.cpp` 后必须显式加入 `CMakeLists.txt`。
- `EmotionSystem`、`TimeSystem`、`WeatherSystem` 已存在，阶段7优先做兼容扩展，不要重建一套平行系统。
- `EnvironmentRules` 是可选薄封装，只负责“当前区域是否室内、当前天气是否允许粒子、某时间段触发什么事件”等规则；如果实现量过小，也可以先用静态函数或 Lua 配置替代。

---

## 3. 核心系统设计

### 3.1 阶段7系统边界

阶段7新增系统较多，但主循环只应该做少量调度：

```cpp
// main.cpp 中只保留类似调用，不把规则散落进输入/渲染/碰撞分支
timeSystem.update(dt);
weatherSystem.update(dt, camera);
emotionSystem.update(dt, environmentSnapshot);
buildingSystem.update(dt, inputSnapshot, mouseWorld);
toySystem.update(dt, inputSnapshot);
questSystem.dispatchQueuedEvents();
```

建议在系统之间传递轻量事件或快照，而不是互相直接持有大量对象：

- `EnvironmentSnapshot`：当前区域 id、是否室内、时间、天气、基地舒适度。
- `GameEvent`：`EnterRegion`、`PlaceFurniture`、`CollectToy`、`TalkPrincess`、`RestAtBase`、`WeatherChanged`、`HourChanged`。
- `RewardBundle`：金币、家具解锁、玩具解锁、好感、童心恢复、委屈降低。

这样可以让建造、玩具、任务、情绪、天气联动，但不继续扩大 `main.cpp` 的分支复杂度。

### 3.2 Furniture 数据结构

```cpp
enum class FurnitureCategory : uint8_t {
    Bed,
    Table,
    Storage,
    Light,
    Decor,
    ToyDisplay,
    Poster
};

struct FurnitureDef {
    std::string id;
    std::string displayName;
    FurnitureCategory category;
    glm::ivec2 size;          // 网格尺寸
    int price = 0;
    int comfort = 0;
    int childlikeHeartRestore = 0;
    int affectionBonus = 0;
    float lightRadius = 0.0f;
    bool blocksMovement = true;
    bool nightBonus = false;
    std::string nostalgiaTag;  // game/anime/hero/fairy/school/song/comic/science/music
    std::string drawStyle;    // bed/table/lamp/rug/toy_shelf...
};

struct FurnitureInstance {
    int instanceId = -1;
    std::string defId;
    glm::ivec2 tilePos;
    int rotation = 0;         // 0/90/180/270
};
```

设计原则：

- `FurnitureDef` 来自 Lua，不直接写死在 C++。
- `FurnitureInstance` 是存档数据，只保存 id、位置、旋转。
- 物理占用由 `BuildingSystem` 根据 `FurnitureDef::blocksMovement` 创建/销毁 Box2D 静态刚体。
- `nostalgiaTag` 对齐 9 大领域，阶段7先用于任务和评分，后续可用于章节收藏墙。

### 3.3 BuildingSystem

职责：

- 加载家具定义。
- 管理当前基地内的家具实例。
- 网格吸附、碰撞检测、放置/移动/拆除。
- 渲染家具和建造预览。
- 计算基地舒适度、童心恢复加成、夜间灯光加成。
- 向 `SaveSystem` 暴露家具实例列表。

关键接口：

```cpp
class BuildingSystem {
public:
    bool loadDefinitions(LuaVM& lua, const char* path);
    void setWorld(b2WorldId world);
    void setBaseMap(const TileMap* map);

    bool canPlace(const std::string& defId, glm::ivec2 tile, int rotation) const;
    int place(const std::string& defId, glm::ivec2 tile, int rotation);
    bool move(int instanceId, glm::ivec2 tile, int rotation);
    bool remove(int instanceId);

    void enterBuildMode();
    void exitBuildMode();
    bool isBuildMode() const;
    void update(float dt, const glm::vec2& mouseWorld);
    void render(const glm::mat4& viewProj);

    int computeComfort() const;
    int computeChildlikeHeartBonus() const;
    float computeNightLightBonus(float hour) const;
    bool hasRequiredFurniture(const std::string& defId, int count) const;
    const std::vector<FurnitureInstance>& getInstances() const;
    void loadInstances(const std::vector<FurnitureInstance>& instances);
};
```

基地评分建议先做简单可解释规则：

- `comfort`：家具舒适度总和，控制委屈自然恢复和好感事件门槛。
- `nostalgiaScore`：不同 `nostalgiaTag` 的收藏覆盖度，控制童心恢复。
- `lightScore`：夜间灯具数量和位置，控制夜间泛灰缓解。
- `blockedExitPenalty`：出口保护失败时禁止放置，不通过扣分处理。

### 3.4 EmotionSystem：童心值与短期情绪

当前 `EmotionSystem` 只有 `grievance/joy/stress`。阶段7建议兼容升级：

```cpp
enum class CharacterMood : uint8_t {
    Calm,
    Happy,
    Worried,
    Inspired,
    Tired
};

struct EmotionState {
    float childlikeHeart = 950.0f; // 0-1000，主线长期资源
    float grievance = 0.0f;        // 0-100，短期委屈/压力
    float joy = 50.0f;
    float stress = 0.0f;
    CharacterMood mood = CharacterMood::Calm;
};
```

关键行为：

- `addGrievance/reduceGrievance/setGrievance` 保留，保证阶段4-6逻辑不失效。
- 新增 `addChildlikeHeart/reduceChildlikeHeart/setChildlikeHeart`，统一 clamp 到 0-1000。
- `getPostProcessIntensity()` 同时参考 `grievance >= 70` 和 `childlikeHeart < 200`。
- `getSpeedMultiplier()` 取委屈减速、低童心减速、天气减速中的乘积或最小值，规则要集中，避免主循环多处重复。
- `vent()` 不再只清零委屈：在基地舒适度足够时额外恢复少量童心。
- `update(dt, snapshot)` 读取是否在基地、是否夜晚、是否雨天、基地评分，决定自然恢复。

阶段7先实现主角情绪和公主心情入口。NPC 情绪只保留 Lua 字段和事件接口，不做全量 AI。

### 3.5 TimeSystem：昼夜玩法化

当前 `TimeSystem` 已能推进小时和返回光照。阶段7扩展重点是“事件化”：

```cpp
enum class TimePeriod : uint8_t {
    Dawn,
    Morning,
    Afternoon,
    Dusk,
    Night,
    LateNight
};

struct TimeSnapshot {
    int day = 1;
    float hour = 10.0f;
    TimePeriod period = TimePeriod::Morning;
    bool isNewDay = false;
    bool isHourChanged = false;
};
```

建议新增接口：

- `TimePeriod getPeriod() const`：统一时间段判断，避免系统各自写阈值。
- `bool consumeNewDayFlag()` 或事件回调：用于每日恢复、商店刷新、天气抽样。
- `void restUntil(float hour)`：基地床铺休息，推进到早上或指定时间。
- `float getNightPenalty() const`：给探索能见度和低童心后处理使用。

阶段7不需要真实日程系统，但要把 `onHourChange` 接入任务和天气/基地恢复事件。

### 3.6 WeatherSystem：区域化与室内过滤

当前 `WeatherSystem` 是全局随机天气。阶段7建议增加区域规则：

```cpp
struct WeatherRule {
    std::string regionId;
    bool allowParticles = true;
    bool indoor = false;
    std::unordered_map<WeatherType, int> weights;
};
```

建议新增接口：

- `void setRegionContext(const std::string& regionId, bool indoor)`：进入基地或室内时关闭雨雪粒子。
- `bool loadWeatherConfig(LuaVM& lua, const char* path)`：读取区域权重和特殊天气。
- `void setStoryWeather(WeatherType type, float duration)`：剧情/任务临时天气，优先级高于随机天气。
- `float getMoodModifier() const`：返回天气对 `grievance/joy/stress` 的影响倾向。
- `bool shouldEmitParticles() const`：渲染层只看这个结果，不在 `main.cpp` 写室内判断。

阶段7特殊天气不必全部实现，先预留枚举或配置名：

- `NoiseStorm`：童谣村庄/噪音公爵伏笔。
- `GrayFog`：长大王国、低童心表现。
- `StarShower`：基地或剧情恢复童心的正向天气。

如果暂时不扩展 `WeatherType` 枚举，也可以用 `WeatherType + weatherThemeTag` 表达特殊天气，避免一次改动过大。

### 3.7 Inventory

最小实现：

- `coins`
- `std::unordered_map<std::string, int> furnitureOwned`
- `std::unordered_set<std::string> toysCollected`
- `std::unordered_set<std::string> unlockedFurniture`
- `std::unordered_set<std::string> unlockedToys`

后续可扩展为道具背包。

阶段7要避免把金币同时维护在多个地方。建议：

- 运行时以 `Inventory::coins` 为准。
- 初始化时从当前 `GameState::coinsCollected` 或 `SaveData::player.coins` 同步。
- 保存时再写回 `SaveData::player.coins`。

### 3.8 ToySystem

最小实现：

- `ToyDef`：id、名称、类型、怀旧标签、解锁条件、展示家具类型、童心奖励。
- `ToyInstance`：放在哪个玩具架/格子。
- `MiniCarGame`：独立状态机，进入后暂停主战斗系统，只保留小游戏输入和 UI。

模型车计时赛建议流程：

1. 站到玩具架旁按 `E`。
2. 进入小游戏模式。
3. 地面显示赛道线、起点、终点和计时器。
4. 完成一圈后记录最佳时间。
5. 首次完成奖励金币；刷新纪录奖励少量好感或玩具碎片。
6. 每日首次游玩恢复少量童心，防止无限刷取。

### 3.9 QuestSystem

阶段7只做数据和事件驱动，不做复杂任务编辑器。

```cpp
enum class QuestState : uint8_t {
    Hidden,
    Available,
    Active,
    Completed,
    Rewarded
};

struct QuestObjective {
    std::string type;     // place_furniture / collect_toy / talk / enter_region / weather_time / childlike_low
    std::string targetId;
    int required = 1;
    int current = 0;
};
```

事件来源：

- `BuildingSystem::place/remove`
- `ToySystem::collect/play`
- `DialogueTree::choose/end`
- `RegionManager::transitionTo`
- `TimeSystem::onHourChange/onNewDay`
- `WeatherSystem::onWeatherChanged`
- `EmotionSystem::onEmotionChange`

任务奖励：

- 金币
- 家具定义解锁
- 玩具解锁
- 公主好感
- 委屈值降低
- 童心值恢复
- 剧情天气或时间推进

---

## 4. 存档扩展

`SaveData` 建议升级到 `version = 3`。

新增：

```cpp
struct BaseData {
    std::string regionId = "home_base";
    std::vector<FurnitureInstance> furniture;
    int comfort = 0;
    int nostalgiaScore = 0;
    int lightScore = 0;
    float lastRestHour = -1.0f;
};

struct InventoryData {
    int coins = 0;
    std::unordered_map<std::string, int> furnitureOwned;
    std::vector<std::string> unlockedFurniture;
    std::vector<std::string> toysCollected;
    std::vector<std::string> unlockedToys;
};

struct QuestData {
    std::vector<QuestSaveEntry> quests;
};

struct EmotionSaveData {
    float childlikeHeart = 950.0f;
    float grievance = 0.0f;
    float joy = 50.0f;
    float stress = 0.0f;
};

struct EnvironmentSaveData {
    int day = 1;
    float hour = 10.0f;
    std::string weather = "Clear";
    float weatherIntensity = 0.0f;
    std::string storyWeatherTag;
};
```

兼容策略：

- 读取 `version <= 2` 的旧存档时，创建空基地数据。
- `coins` 继续从 `player.coins` 读取，避免破坏现有字段。
- 旧存档只有 `emotion.grievance/joy` 时，`childlikeHeart` 默认 950，`stress` 默认 0。
- 旧存档没有时间/天气时，沿用当前初始化值：第 1 天 10:00，随机天气开启。
- 旧存档没有库存时，根据 `player.progress.collectedItems` 尝试迁移已收集玩具；迁移不了的字段保持空。
- 家具实例保存为纯数据，不保存 Box2D body；加载后由 `BuildingSystem` 重建物理体。
- 天气粒子不进存档，只保存天气类型、强度和剧情天气标记；读档后由 `WeatherSystem` 重新生成效果。

JSON 建议结构：

```json
{
  "version": 3,
  "player": {
    "coins": 120
  },
  "emotion": {
    "childlikeHeart": 950,
    "grievance": 15,
    "joy": 55,
    "stress": 10
  },
  "environment": {
    "day": 2,
    "hour": 21.5,
    "weather": "Rain",
    "weatherIntensity": 0.8,
    "storyWeatherTag": ""
  },
  "base": {
    "regionId": "home_base",
    "comfort": 28,
    "nostalgiaScore": 3,
    "lightScore": 2,
    "furniture": []
  },
  "inventory": {
    "furnitureOwned": {},
    "unlockedFurniture": ["simple_bed", "star_lamp"],
    "toysCollected": ["mini_car"],
    "unlockedToys": ["mini_car"]
  },
  "quests": []
}
```

---

## 5. Lua 配置格式

### 5.1 `assets/scripts/furniture.lua`

```lua
return {
  {
    id = "simple_bed",
    name = "小木床",
    category = "Bed",
    size = {2, 1},
    price = 80,
    comfort = 10,
    childlikeHeartRestore = 5,
    affectionBonus = 1,
    lightRadius = 0,
    nightBonus = true,
    nostalgiaTag = "home",
    blocksMovement = true,
    drawStyle = "bed"
  },
  {
    id = "star_lamp",
    name = "星愿灯",
    category = "Light",
    size = {1, 1},
    price = 120,
    comfort = 8,
    childlikeHeartRestore = 2,
    affectionBonus = 2,
    lightRadius = 4.0,
    nightBonus = true,
    nostalgiaTag = "star",
    blocksMovement = true,
    drawStyle = "lamp"
  },
  {
    id = "game_poster_4399",
    name = "4399 像素海报",
    category = "Poster",
    size = {1, 1},
    price = 60,
    comfort = 2,
    childlikeHeartRestore = 3,
    affectionBonus = 0,
    lightRadius = 0,
    nightBonus = false,
    nostalgiaTag = "game",
    blocksMovement = false,
    drawStyle = "poster"
  }
}
```

### 5.2 `assets/scripts/toys.lua`

```lua
return {
  {
    id = "mini_car",
    name = "模型赛车",
    type = "MiniGame",
    nostalgiaTag = "game",
    price = 150,
    displayOn = "toy_shelf",
    miniGame = "mini_car_race",
    firstClearRewards = {
      coins = 30,
      childlikeHeart = 20,
      affection = 5
    },
    dailyRewards = {
      childlikeHeart = 5
    }
  }
}
```

### 5.3 `assets/scripts/quests.lua`

```lua
return {
  {
    id = "base_first_setup",
    name = "整理秘密基地",
    objectives = {
      { type = "place_furniture", target = "simple_bed", required = 1 },
      { type = "place_furniture", target = "star_lamp", required = 1 }
    },
    rewards = {
      coins = 50,
      affection = 10,
      childlikeHeart = 20,
      unlockFurniture = { "toy_shelf" }
    }
  },
  {
    id = "rain_night_talk",
    name = "雨夜谈心",
    objectives = {
      { type = "weather_time", target = "Rain:Night", required = 1 },
      { type = "talk", target = "active_princess", required = 1 }
    },
    rewards = {
      grievance = -20,
      childlikeHeart = 30,
      affection = 15
    }
  }
}
```

### 5.4 `assets/scripts/childhood_config.lua`

```lua
return {
  childlikeHeart = {
    initial = 950,
    min = 0,
    max = 1000,
    lowThreshold = 200,
    lowSpeedMultiplier = 0.7,
    badEndingWarningThreshold = 0
  },
  grievance = {
    depressedThreshold = 70,
    extremeThreshold = 90,
    homeRecoveryPerMinute = 1,
    ventCooldown = 5
  },
  princessMood = {
    default = "Calm",
    lowHeartMood = "Worried",
    baseNightTalkMood = "Inspired"
  }
}
```

### 5.5 `assets/scripts/time_events.lua`

```lua
return {
  periods = {
    { id = "Dawn",      start = 5,  finish = 7 },
    { id = "Morning",   start = 7,  finish = 12 },
    { id = "Afternoon", start = 12, finish = 17 },
    { id = "Dusk",      start = 17, finish = 19 },
    { id = "Night",     start = 19, finish = 23 },
    { id = "LateNight", start = 23, finish = 5 }
  },
  baseRecovery = {
    nightBonusStart = 19,
    restUntilHour = 7,
    childlikeHeartPerRest = 20
  }
}
```

### 5.6 `assets/scripts/weather_config.lua`

```lua
return {
  defaultInterval = 300,
  regions = {
    default = {
      indoor = false,
      weights = {
        Clear = 40,
        Cloudy = 20,
        Rain = 15,
        Fog = 10,
        HeavyRain = 10,
        Snow = 5
      }
    },
    home_base = {
      indoor = true,
      allowParticles = false,
      weights = {
        Clear = 55,
        Cloudy = 20,
        Rain = 15,
        Fog = 5,
        Snow = 5
      }
    },
    nursery_rhyme_village = {
      indoor = false,
      specialTag = "NoiseStorm",
      weights = {
        Clear = 30,
        Rain = 20,
        HeavyRain = 20,
        Fog = 10,
        Cloudy = 20
      }
    }
  },
  mood = {
    Rain = { grievanceAtBase = -1, stressOutside = 1 },
    Fog = { stressOutside = 2 },
    Snow = { joy = 1 },
    HeavyRain = { stressOutside = 3 }
  }
}
```

---

## 6. UI 与输入设计

### 6.1 基地内普通模式

- `E`：交互家具/玩具/门/公主。
- `Tab`：进入建造模式。
- `Esc`：关闭面板或退出小游戏。
- 靠近床铺时 `E`：休息或宣泄；如果是夜晚，额外恢复少量童心。
- 靠近星愿灯时 `E`：开/关灯；阶段7可先只改变灯具绘制亮度和基地 `lightScore`。
- 靠近玩具架时 `E`：打开玩具展示或进入可用小游戏。
- 靠近公主时 `E`：根据时间、天气、童心值和基地舒适度选择基地对话。

### 6.2 HUD 与状态提示

阶段7不要求完整 UI 框架，但需要把核心状态从窗口标题逐步迁移到可见 HUD：

- 左上：生命、魔力、金币。
- 左侧或底部：童心值条，范围 0-1000；低于 200 时使用灰色/闪烁边框。
- 童心值旁：短期委屈图标或数值，范围 0-100；高于 70 时显示暗角强度提示。
- 右上：第几天、时间、时间段、天气名称。
- 基地内额外显示：舒适度、童年收藏分、灯光分。
- 公主跟随时显示：好感等级、心情状态、羁绊技充能。

没有稳定字体时，阶段7可以先用几何图标和颜色表达：

- 童心值：星形/糖果形能量条。
- 委屈值：暗色水滴或小云图标。
- 天气：太阳、云、雨线、雾线、雪点。
- 时间段：小太阳、夕阳、月亮。

### 6.3 建造模式

- 左侧竖向家具分类栏：床、桌、灯、装饰、玩具。
- 底部显示当前家具预览、价格、库存数量、舒适度、童心恢复和怀旧标签。
- 鼠标在地图上显示半透明预览：
  - 绿色：可放置。
  - 红色：不可放置。
- 黄色：可放置但没有足够金币或库存。
- 灯具在夜晚预览光照半径。
- 海报/玩具显示对应 9 大领域标签，便于后续收藏墙扩展。
- 不依赖字体时，先用图形图标表达分类；已有 `DialogueUI` 后可逐步加入文本。

### 6.4 小游戏模式

- 进入小游戏后隐藏战斗 HUD。
- 显示计时器、最佳成绩、退出提示。
- 小游戏结束后恢复基地状态，不影响主世界敌人和投射物。
- 小游戏完成结算显示金币、童心、好感奖励。
- 每日奖励领取状态需要有提示，防止玩家误以为重复游玩没有效果是 Bug。

### 6.5 Debug 输入与手动验证

Debug 构建下建议加入临时快捷键，Release 可禁用：

- `F5/F6`：降低/恢复童心值，用于测试低童心后处理和强制回基地。
- `F7`：增加委屈值，用于测试宣泄和暗角。
- `F8`：循环天气。
- `F9`：推进到下一个时间段。
- `F10`：切换室内/室外天气粒子过滤检查。

这些快捷键必须集中在 Debug 控制区，避免和正式输入混在一起。

---

## 7. 实现顺序

### Milestone 7.1：童心情绪与环境基础升级

1. 扩展 `EmotionState`，新增 `childlikeHeart` 和 `CharacterMood`，保留 `grievance/joy/stress` 兼容旧逻辑。
2. 加载 `childhood_config.lua`，把童心阈值、委屈阈值、低童心速度倍率配置化。
3. 更新 `getPostProcessIntensity()` 和 `getSpeedMultiplier()`，同时考虑委屈值和低童心。
4. 扩展 `TimeSystem` 的时间段判断和日切/整点事件标记。
5. 扩展 `WeatherSystem` 的区域上下文、室内粒子过滤和天气配置读取。
6. Debug 构建下加入童心、委屈、时间、天气的临时调试输入。

### Milestone 7.2：基地区域与进入/退出

1. 在 `RegionManager` 中增加 `home_base` 固定区域。
2. 生成室内地图：墙、地板、门、可放置区。
3. 在新手村 `player_home` POI 上触发进入基地。
4. 返回门触发回到原区域入口。
5. 进入基地时设置 `WeatherSystem` 室内上下文，关闭雨雪粒子但保留光照/天气状态。
6. 验证区域切换、玩家传送、物理体重建、存档不崩溃。

### Milestone 7.3：家具定义与基础建造

1. 新建 `FurnitureDef/FurnitureInstance`。
2. 从 `furniture.lua` 加载定义。
3. 实现建造模式、预览、网格吸附、旋转。
4. 实现 `canPlace/place/remove`。
5. 家具渲染用 `Draw2D` 完成第一批样式。
6. 实现舒适度、童年收藏分、夜间灯光分。
7. 把床、星愿灯、玩具架的交互事件接入 `EmotionSystem`。

### Milestone 7.4：库存、金币与商店

1. 新建 `Inventory`。
2. 金币接入当前 `coinsCollected`。
3. 家具目录消耗金币购买。
4. 拆除家具返还库存，不返还金币。
5. 商品数据使用“童年物件”命名和怀旧标签。
6. 存档保存家具库存和解锁状态。

### Milestone 7.5：存档扩展

1. `SaveData` 升级 version 3。
2. 保存/读取童心、委屈、时间、天气。
3. 保存/读取基地家具实例、基地评分、库存、玩具、任务。
4. 加载后重建家具物理体和室内天气上下文。
5. 旧存档兼容：`version <= 2` 时创建默认童心值和空基地数据。

### Milestone 7.6：玩具系统与首个小游戏

1. 加载 `toys.lua`。
2. 实现玩具架展示。
3. 实现 `MiniCarGame` 状态机和简单赛道。
4. 完成奖励结算：金币、童心、好感。
5. 实现每日首次奖励限制，防止无限刷童心。

### Milestone 7.7：任务、公主与环境联动

1. 新建轻量 `QuestSystem`。
2. 实现 5 个基地任务：整理秘密基地、给公主的花、星愿玩具架、雨夜谈心、童心警报。
3. 任务奖励接入金币、家具解锁、玩具解锁、好感、童心恢复、委屈降低。
4. 基地舒适度影响公主互动、宣泄效果和夜间谈心事件。
5. 先保留当前单公主链路，同时把任务/存档字段按 9 公主扩展。

### Milestone 7.8：打磨与性能

1. HUD 显示童心、委屈、时间、天气、基地评分。
2. 建造 UI 视觉整理，家具放置边界和出口保护。
3. 小游戏退出/失败/重开流程。
4. 低童心、雨夜、室内天气、夜间灯光做集中手动测试。
5. Release 构建、运行 10 分钟稳定性测试。

---

## 8. 验收标准

- [ ] `EmotionSystem` 保留旧 `grievance` 行为，并新增 0-1000 的 `childlikeHeart`。
- [ ] 童心值低于 200 时会触发低童心后处理/减速/提示，恢复到阈值以上后效果消失。
- [ ] 宣泄、床铺休息、玩具小游戏、公主谈心至少各有一种方式能影响童心或委屈值。
- [ ] `TimeSystem` 能稳定推进天数和时间段，HUD 或调试信息能显示第几天、时间和时间段。
- [ ] 夜晚基地灯具亮度或光照表现与白天不同。
- [ ] `WeatherSystem` 能按区域上下文过滤室内雨雪粒子，基地内不出现雨滴/雪花穿屋。
- [ ] 天气名称、强度、移动/能见度影响可被调试验证。
- [ ] 玩家能从新手村进入 `home_base`，并能返回原区域。
- [ ] 基地地图有正确墙体、地板、门和物理阻挡。
- [ ] 建造模式可进入/退出，鼠标预览位置正确。
- [ ] 家具能放置、旋转、移动、拆除。
- [ ] 家具不能重叠、不能穿墙、不能堵门。
- [ ] 家具会创建/销毁物理阻挡，玩家不能穿过阻挡家具。
- [ ] 金币能购买家具，库存数量正确变化。
- [ ] 家具布局能保存并在读档后恢复。
- [ ] 至少 7 种家具可用：床、书桌、花盆、星愿灯、玩具架、地毯、童年海报。
- [ ] 家具视觉风格与现有 SDF/Draw2D 风格一致。
- [ ] 基地能计算舒适度、童年收藏分、灯光分，并至少有一项会影响情绪或事件。
- [ ] 至少 1 个玩具可收集、展示、互动。
- [ ] 模型车小游戏可进入、完成、退出，并能结算金币/童心/好感奖励。
- [ ] 每日玩具奖励不能无限重复刷取。
- [ ] 至少 5 个基地任务可完成，包含一个时间/天气条件任务和一个低童心任务。
- [ ] 基地舒适度影响好感、委屈宣泄或童心恢复。
- [ ] `SaveData version = 3` 能保存/读取童心、委屈、时间、天气、家具、库存、玩具和任务状态。
- [ ] 读取 `version <= 2` 的旧存档不会崩溃，并能补齐默认童心和空基地数据。
- [ ] Release 构建通过，运行 10 分钟无崩溃。
- [ ] 不修改 `build/` 生成物作为源码提交内容。

---

## 9. 风险与约束

- `main.cpp` 已经承担大量流程逻辑，阶段7应避免继续把建造/玩具/任务全部堆入 `main.cpp`。新增模块应封装状态，只在主循环做少量调用。
- `CMakeLists.txt` 当前显式列出 `.cpp`，新增文件必须同步加入。
- 当前 UI 没有字体系统，建造菜单早期应优先使用图标、颜色和几何状态，文本可复用 `DialogueUI` 或后续阶段引入位图字体。
- 家具物理体生命周期要与区域切换一致：离开基地时不要泄漏 Box2D body；读档或重建家具时要先清理旧 body。
- 不要直接修改 `TileMap` 做家具占用。家具是动态布置对象，应有独立占用层；只有确实需要改变地形时才通过 `MapTileManager`。
- 玩具小游戏应是独立状态机，避免污染主战斗输入和碰撞逻辑。
- `EmotionSystem` 升级要兼容阶段4-6：旧代码调用 `addGrievance()`、`vent()`、`getSpeedMultiplier()` 时不能失效。
- 童心值和委屈值不要互相替代：童心值是长期主线资源，委屈值是短期状态；两者都可以影响后处理，但恢复和存档字段应分开。
- 金币来源要统一。运行时避免 `coinsCollected`、`Inventory::coins`、`SaveData::player.coins` 三处各自增减导致不同步。
- 天气系统要区分“天气状态”和“天气粒子”：室内可以保留雨天状态与光照修正，但不应生成雨雪粒子。
- 时间推进与存档读档要注意日切事件重复触发：读档恢复时间不应误触发一次每日奖励。
- 9 公主数据可以先按结构预留，不要在阶段7强行完成 9 套完整 AI、日程和剧情，避免阶段目标失控。

---

## 10. 阶段7完成后的阶段8衔接

阶段7完成后，阶段8应重点处理：

- 音频系统与程序化音效。
- 主菜单、设置、背包完整 UI。
- 9 位公主的完整角色数据、立绘/几何外观、日程和章节对话。
- 9 大领域与怀旧主题地图的章节任务。
- 主线剧情任务、童心值归零坏结局、9 公主全救真结局。
- 天气/昼夜与 BGM、环境音、剧情演出的联动。
- 性能剖析与内存/Box2D 生命周期检查。
- 打包与发布流程。

阶段7的核心价值是把“战斗掉落金币”转化为“购买童年物件、布置秘密基地、恢复童心、触发公主事件、再出发冒险”的生活闭环。完成后，《星愿之子》的阶段目标应从单纯可战斗的开放世界原型，推进到能表达“童心 vs 长大”主题的动作 RPG 原型。
