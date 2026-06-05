# 阶段4：情感与公主系统实现计划

## Context

阶段3已完成物理与基础战斗系统：投射物、敌人AI、生命伤害、掉落拾取、超人抓举。阶段4目标是引入Lua脚本系统，实现情感系统（委屈值/好感度）、对话树、以及第一位公主NPC（小夏）。

**阶段4具体目标（来自技术栈.md）：**
1. Lua脚本系统集成（sol2）
2. 对话树系统（Lua驱动）
3. NPC基础框架，公主小夏第一版（静态日程）
4. 好感度与委屈值实现，后处理暗角
5. 蒙头哭宣泄动画

## 实现方案

### 1. Lua脚本系统集成

在 `src/Engine/Scripting/` 下创建 `LuaVM.h/.cpp`：

```cpp
// LuaVM.h
#pragma once

#include <sol/sol.hpp>
#include <functional>
#include <string>

/**
 * LuaVM - Lua脚本虚拟机
 *
 * 封装sol2状态，提供游戏脚本的加载、执行和C++/Lua互调。
 */
class LuaVM {
public:
    LuaVM();
    ~LuaVM();

    // 初始化（打开常用库，绑定游戏API）
    bool init();

    // 加载并执行Lua文件
    bool loadFile(const char* path);

    // 执行Lua代码字符串
    bool doString(const char* code);

    // 获取sol::state引用（用于高级操作）
    sol::state& state() { return *lua; }

    // 调用Lua函数（返回值可选）
    template<typename... Args>
    auto call(const char* funcName, Args&&... args);

    // 设置全局变量
    template<typename T>
    void setGlobal(const char* name, T&& value);

    // 注册C++函数到Lua
    template<typename Func>
    void setFunction(const char* name, Func&& func);

    // 错误处理
    bool hasError() const { return hasErrorFlag; }
    const std::string& getLastError() const { return lastError; }

private:
    std::unique_ptr<sol::state> lua;
    bool hasErrorFlag;
    std::string lastError;

    // 绑定游戏API到Lua
    void bindGameAPI();

    // 错误处理回调
    void errorHandler(const std::string& msg);
};
```

**LuaVM.cpp 关键实现：**

```cpp
bool LuaVM::init() {
    lua = std::make_unique<sol::state>();
    lua->open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

    // 绑定C++ API到Lua
    bindGameAPI();

    // 设置错误处理
    lua->set_exception_handler([this](lua_State* L, const sol::error& e) {
        hasErrorFlag = true;
        lastError = e.what();
    });

    return true;
}

void LuaVM::bindGameAPI() {
    auto& s = state();

    // 数学工具
    s.set_function("dist", [](float x1, float y1, float x2, float y2) {
        return glm::distance(glm::vec2(x1, y1), glm::vec2(x2, y2));
    });
    s.set_function("normalize", [](float x, float y) {
        glm::vec2 v = glm::normalize(glm::vec2(x, y));
        return sol::make_table(state(), "x", v.x, "y", v.y);
    });
    s.set_function("lerp", [](float a, float b, float t) {
        return a + (b - a) * t;
    });
    s.set_function("random", [](unsigned int seed) {
        return Math::hashRandom(seed);
    });

    // 调试输出
    s.set_function("printLog", [](const char* msg) {
        printf("[Lua] %s\n", msg);
    });
}
```

**Lua脚本路径处理：**

Lua脚本文件需要与着色器同样处理——复制到构建目录。在 CMakeLists.txt 中添加：

```cmake
# Copy Lua scripts to build directory (same as shaders)
file(COPY ${CMAKE_SOURCE_DIR}/assets/scripts
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)
```

或者设置VS调试工作目录：

```cmake
set_target_properties(Starchild2D PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
```

推荐第一种方案（复制到build目录），与着色器保持一致。

**Lua脚本文件结构：**

```
assets/scripts/
├── dialogues/
│   ├── first_meeting.lua      # 与小夏初次见面
│   ├── daily_greetings.lua    # 日常对话
│   └── quest_give.lua         # 任务对话
├── abilities.lua              # 技能配置
├── npc_schedules.lua          # NPC日程表
├── emotion_config.lua         # 情感系统配置
└── ui_layouts.lua             # UI布局描述
```

**技能配置示例（abilities.lua）：**

```lua
-- 技能配置表
abilities = {
    fireball = {
        name = "火球术",
        manaCost = 15,
        cooldown = 0.3,
        projectileSpeed = 400,
        damage = 25,
        lifetime = 2.0,
        particleColor = {1.0, 0.4, 0.0},
        description = "发射一枚火球，对命中的敌人造成伤害"
    },
    -- 预留：冰锥、雷电等
}

-- 技能快捷键绑定
keyBindings = {
    fireball = "J",
    grab = "K",
    interact = "E",
    heal = "H"
}
```

### 2. 对话树系统

在 `src/Game/Social/` 下创建 `DialogueTree.h/.cpp`：

```cpp
// DialogueTree.h
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <glm/vec3.hpp>

/**
 * 对话选项
 */
struct DialogueChoice {
    std::string text;           // 选项文本
    std::string nextNode;       // 跳转到的节点ID
    int affectionChange;        // 好感度变化
    std::function<void()> onChoose; // 选择后的回调
};

/**
 * 对话节点
 */
struct DialogueNode {
    std::string id;
    std::string speaker;        // 说话者名字
    std::string text;           // 对话内容
    glm::vec3 textColor;        // 文本颜色（可选）
    std::vector<DialogueChoice> choices;
    std::string nextNode;       // 如果没有选项，自动跳转
    float displayTime;          // 自动显示时间（0=等待选择）
};

/**
 * 对话树管理器
 */
class DialogueTree {
public:
    // 从Lua加载对话树
    bool loadFromLua(LuaVM& lua, const char* dialogueId);

    // 开始对话
    void start(const std::string& startNode);

    // 获取当前节点
    const DialogueNode* getCurrentNode() const;

    // 选择选项
    void choose(int choiceIndex);

    // 继续到下一节点
    void next();

    // 是否正在对话中
    bool isActive() const { return !currentNodeId.empty(); }

    // 结束对话
    void end();

    // 对话事件回调
    using DialogueCallback = std::function<void(const DialogueNode&)>;
    void setOnNodeEnter(DialogueCallback cb) { onNodeEnter = std::move(cb); }
    void setOnDialogueEnd(DialogueCallback cb) { onDialogueEnd = std::move(cb); }

private:
    std::unordered_map<std::string, DialogueNode> nodes;
    std::string currentNodeId;
    DialogueCallback onNodeEnter;
    DialogueCallback onDialogueEnd;
};
```

**对话Lua示例（first_meeting.lua）：**

```lua
-- 与小夏初次见面
return {
    start = {
        speaker = "小夏",
        text = "咦？你就是新来的转学生吗？我是小夏，很高兴认识你！✨",
        textColor = {0.9, 0.6, 0.8},
        next = "self_intro"
    },

    self_intro = {
        speaker = "小夏",
        text = "我在这个学校已经一年了，对这里很熟悉的。有什么不懂的可以随时问我哦~",
        next = "player_choice"
    },

    player_choice = {
        speaker = "",
        text = "你要怎么回应？",
        choices = {
            {
                text = "「你好，请多关照！」（友好）",
                next = "friendly_response",
                affectionChange = 10
            },
            {
                text = "「嗯。」（冷淡）",
                next = "cold_response",
                affectionChange = -5
            },
            {
                text = "「你就是传说中的校花？」（调侃）",
                next = "tease_response",
                affectionChange = 5
            }
        }
    },

    friendly_response = {
        speaker = "小夏",
        text = "嘿嘿，你人真好！以后我们就是朋友啦~",
        next = "end"
    },

    cold_response = {
        speaker = "小夏",
        text = "唔...你好像不太爱说话呢。没关系，慢慢来~",
        next = "end"
    },

    tease_response = {
        speaker = "小夏",
        text = "哈？谁、谁说的啦！（脸红）别听别人乱说...",
        next = "end"
    },

    end = {
        speaker = "",
        text = "",
        next = ""  -- 空next表示对话结束
    }
}
```

### 3. NPC与公主系统

在 `src/Game/Social/` 下创建 `NPC.h/.cpp` 和 `Princess.h/.cpp`：

```cpp
// NPC.h
#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

/**
 * NPC日程项
 */
struct ScheduleEntry {
    float startTime;    // 开始时间（0-24小时制）
    float endTime;
    glm::vec2 position; // 目标位置
    std::string action; // 动作类型：idle, walk, sit, etc.
    std::string dialogueTrigger; // 触发对话ID（空=不触发）
};

/**
 * NPC基础类
 */
class NPC {
public:
    NPC(const std::string& name);
    virtual ~NPC();

    // Box2D
    void createBody(b2WorldId world, const glm::vec2& pos);
    b2BodyId getBodyId() const { return bodyId; }

    // 更新
    virtual void update(float dt, float gameTime);

    // 渲染
    virtual void render(const glm::mat4& viewProj);

    // 交互
    virtual bool canInteract(const glm::vec2& playerPos, float range) const;
    virtual void onInteract();

    // 日程
    void setSchedule(const std::vector<ScheduleEntry>& schedule);
    void setCurrentSchedule(const std::string& scheduleId);

    // 属性
    const std::string& getName() const { return name; }
    glm::vec2 getPosition() const;
    glm::vec3 getColor() const { return color; }
    void setColor(const glm::vec3& c) { color = c; }

    // 对话
    void setDialogueId(const std::string& id) { dialogueId = id; }
    const std::string& getDialogueId() const { return dialogueId; }

protected:
    std::string name;
    b2BodyId bodyId;
    glm::vec3 color;
    std::string dialogueId;

    // 日程
    std::vector<ScheduleEntry> currentSchedule;
    int currentScheduleIndex;

    // 状态
    enum class State {
        Idle, Walking, Interacting
    } state;
    glm::vec2 targetPosition;

    // 查找当前时间对应的日程项
    int findScheduleForTime(float gameTime) const;
};
```

**NPC日程时间处理（处理跨夜和循环）：**

```cpp
// NPC.cpp
int NPC::findScheduleForTime(float gameTime) const {
    // 归一化到 [0, 24)
    while (gameTime < 0.0f) gameTime += 24.0f;
    while (gameTime >= 24.0f) gameTime -= 24.0f;

    for (int i = 0; i < static_cast<int>(currentSchedule.size()); ++i) {
        const auto& entry = currentSchedule[i];
        float start = entry.startTime;
        float end = entry.endTime;

        if (start < end) {
            // 普通区间
            if (gameTime >= start && gameTime < end) return i;
        } else if (start > end) {
            // 跨夜区间（如 22:00-6:00）
            if (gameTime >= start || gameTime < end) return i;
        }
    }
    return 0; // fallback
}

void NPC::update(float dt, float gameTime) {
    if (currentSchedule.empty()) return;

    int targetIndex = findScheduleForTime(gameTime);
    if (targetIndex != currentScheduleIndex) {
        currentScheduleIndex = targetIndex;
        const auto& entry = currentSchedule[currentScheduleIndex];
        targetPosition = entry.position;

        if (entry.action == "idle") {
            state = State::Idle;
        } else if (entry.action == "walk") {
            state = State::Walking;
        }
    }

    // 移动到目标位置
    if (state == State::Walking) {
        b2Vec2 pos = b2Body_GetPosition(bodyId);
        glm::vec2 currentPos(pos.x, pos.y);
        glm::vec2 dir = targetPosition - currentPos;
        float dist = glm::length(dir);

        if (dist > 0.1f) {
            dir = glm::normalize(dir);
            b2Vec2 force = { dir.x * 2.0f, dir.y * 2.0f };
            b2Body_ApplyForceToCenter(bodyId, force, true);
        } else {
            state = State::Idle;
        }
    }

    // 限制速度
    b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
    float maxSpeed = 3.0f;
    float speed = glm::length(glm::vec2(vel.x, vel.y));
    if (speed > maxSpeed) {
        vel.x = vel.x * maxSpeed / speed;
        vel.y = vel.y * maxSpeed / speed;
        b2Body_SetLinearVelocity(bodyId, vel);
    }
}
```

```cpp
// Princess.h
#pragma once

#include "NPC.h"

/**
 * 好感度等级
 */
enum class AffectionLevel : int {
    Stranger,     // 0-100:   陌生人
    Acquaintance, // 100-300: 熟人
    Friend,       // 300-500: 朋友
    CloseFriend,  // 500-700: 亲密朋友
    Beloved       // 700-1000: 心上人
};

inline const char* affectionLevelName(AffectionLevel level) {
    switch (level) {
        case AffectionLevel::Stranger:     return "陌生人";
        case AffectionLevel::Acquaintance: return "熟人";
        case AffectionLevel::Friend:       return "朋友";
        case AffectionLevel::CloseFriend:  return "亲密朋友";
        case AffectionLevel::Beloved:      return "心上人";
        default: return "?";
    }
}

/**
 * 公主 - 可攻略NPC
 */
class Princess : public NPC {
public:
    Princess(const std::string& name);

    // 好感度
    float getAffection() const { return affection; }
    void addAffection(float amount) {
        affection = glm::clamp(affection + amount, 0.0f, 1000.0f);
    }
    AffectionLevel getAffectionLevel() const {
        if (affection < 100.0f) return AffectionLevel::Stranger;
        if (affection < 300.0f) return AffectionLevel::Acquaintance;
        if (affection < 500.0f) return AffectionLevel::Friend;
        if (affection < 700.0f) return AffectionLevel::CloseFriend;
        return AffectionLevel::Beloved;
    }
    const char* getAffectionLevelName() const {
        return affectionLevelName(getAffectionLevel());
    }

    // 跟随行为
    void setFollowing(bool follow);
    bool isFollowing() const { return following; }

    // 战斗增益（根据好感度）
    float getCombatBonus() const;

    // 特殊事件
    void triggerSpecialEvent(const std::string& eventId);

    // 渲染（公主有特殊外观）
    void render(const glm::mat4& viewProj) override;

    // 羁绊技就绪
    bool isUltimateReady() const { return ultimateCharge >= 100.0f; }
    void addUltimateCharge(float amount) { ultimateCharge = std::min(100.0f, ultimateCharge + amount); }

private:
    float affection;        // 0-1000
    bool following;         // 是否跟随玩家
    float ultimateCharge;   // 羁绊技充能 0-100

    // 根据好感度的行为差异
    void updateBehavior(float dt);
};
```

**公主小夏配置：**

```cpp
// 创建小夏
auto xiaoxia = std::make_unique<Princess>("小夏");
xiaoxia->setColor(glm::vec3(0.9f, 0.6f, 0.8f)); // 粉色
xiaoxia->setDialogueId("first_meeting");
xiaoxia->createBody(world, glm::vec2(5.0f, 3.0f));

// 日程：白天在学校，傍晚回家
xiaoxia->setSchedule({
    {6.0f, 8.0f,  glm::vec2(10, 5),  "idle",  ""},          // 早上在学校
    {8.0f, 12.0f, glm::vec2(12, 8),  "walk",  "daily_greetings"}, // 上午走动
    {12.0f, 14.0f, glm::vec2(10, 5), "idle",  ""},           // 中午在学校
    {14.0f, 18.0f, glm::vec2(15, 10), "walk", ""},           // 下午走动
    {18.0f, 22.0f, glm::vec2(3, 2),   "idle",  ""},           // 晚上在家
    {22.0f, 24.0f, glm::vec2(3, 2),   "idle",  ""},           // 深夜在家
    {0.0f, 6.0f,  glm::vec2(3, 2),    "idle",  ""},           // 凌晨在家
});
```

### 4. 情感系统（委屈值与好感度）

在 `src/Game/Emotion/` 下创建 `EmotionSystem.h/.cpp`：

```cpp
// EmotionSystem.h
#pragma once

#include <glm/vec2.hpp>
#include <functional>

/**
 * 情感状态
 */
struct EmotionState {
    float grievance;      // 委屈值 0-100
    float joy;            // 快乐值 0-100（可选）
    float stress;         // 压力值 0-100（可选）

    EmotionState() : grievance(0), joy(50), stress(0) {}

    // 委屈值超过阈值
    bool isDepressed() const { return grievance >= 70.0f; }
    bool isExtreme() const { return grievance >= 90.0f; }
};

/**
 * 情感系统
 */
class EmotionSystem {
public:
    EmotionSystem();

    // 更新委屈值
    void addGrievance(float amount);
    void reduceGrievance(float amount);
    void setGrievance(float value);

    // 宣泄（蒙头哭）
    void vent();  // 清零委屈值

    // 设置是否在家（影响自然恢复）
    void setAtHome(bool atHome) { this->atHome = atHome; }
    bool isAtHome() const { return atHome; }

    // 获取当前状态
    const EmotionState& getState() const { return state; }

    // 获取后处理强度（暗角程度，clamp到[0,1]）
    float getPostProcessIntensity() const {
        return glm::clamp((state.grievance - 70.0f) / 30.0f, 0.0f, 1.0f);
    }

    // 移动速度修正（委屈时减速）
    float getSpeedMultiplier() const {
        return state.isDepressed() ? 0.7f : 1.0f;
    }

    // 回调
    using EmotionCallback = std::function<void(const EmotionState&)>;
    void setOnEmotionChange(EmotionCallback cb) { onEmotionChange = std::move(cb); }

    // 更新（dt为秒）
    void update(float dt);

private:
    EmotionState state;
    float ventCooldown;         // 宣泄后冷却时间
    bool atHome;                // 是否在家（影响自然恢复）
    float naturalRecoveryTimer; // 自然恢复计时器

    EmotionCallback onEmotionChange;
};
```

**EmotionSystem.cpp 实现：**

```cpp
EmotionSystem::EmotionSystem()
    : ventCooldown(0.0f)
    , atHome(false)
    , naturalRecoveryTimer(0.0f)
{}

void EmotionSystem::addGrievance(float amount) {
    state.grievance = std::min(100.0f, state.grievance + amount);
    if (onEmotionChange) onEmotionChange(state);
}

void EmotionSystem::reduceGrievance(float amount) {
    state.grievance = std::max(0.0f, state.grievance - amount);
    if (onEmotionChange) onEmotionChange(state);
}

void EmotionSystem::vent() {
    if (ventCooldown > 0.0f) return; // 冷却中不可宣泄
    state.grievance = 0.0f;
    ventCooldown = 5.0f; // 宣泄后5秒冷却
    if (onEmotionChange) onEmotionChange(state);
}

```cpp
void EmotionSystem::update(float dt) {
    // 宣泄冷却
    if (ventCooldown > 0.0f) ventCooldown -= dt;

    // 自然恢复：在家时每60秒 -1 委屈值
    if (atHome && state.grievance > 0.0f) {
        naturalRecoveryTimer += dt;
        if (naturalRecoveryTimer >= 60.0f) {
            naturalRecoveryTimer -= 60.0f;
            state.grievance = std::max(0.0f, state.grievance - 1.0f);
            if (onEmotionChange) onEmotionChange(state);
        }
    }
}
```

**家的区域判断：**

```cpp
// 在main.cpp或GameState中定义家的位置
const glm::vec2 HOME_POSITION = glm::vec2(3.0f, 2.0f);
const float HOME_RADIUS = 3.0f;

bool isAtHome(const glm::vec2& playerPos) {
    return glm::distance(playerPos, HOME_POSITION) <= HOME_RADIUS;
}

// 在主循环中更新
emotionSystem.setAtHome(isAtHome(playerPos));
```

**委屈值触发规则：**

| 事件 | 委屈值变化 |
|------|-----------|
| 受伤 | +10 |
| 死亡 | +20 |
| 对话被拒绝 | +5 |
| 被敌人击中 | +5 |
| 在家宣泄（蒙头哭） | -100（清零） |
| 与公主互动（好感度高时） | -10 |
| 自然恢复（在家时） | -1/分钟 |

**后处理效果：**

委屈值影响：
- 70+：屏幕四角暗角（vignette），强度 = (grievance - 70) / 30
- 移动速度 × 0.7
- 颜色饱和度降低

### 5. 蒙头哭宣泄动画

在 `src/Game/Emotion/` 下创建 `VentAnimation.h/.cpp`：

```cpp
// VentAnimation.h
#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

/**
 * 蒙头哭动画状态
 */
class VentAnimation {
public:
    // 开始动画
    void start(const glm::vec2& position);

    // 更新
    void update(float dt);

    // 是否正在进行中
    bool isActive() const { return active; }

    // 获取动画进度（0-1）
    float getProgress() const { return progress; }

    // 获取颤抖幅度
    float getShakeAmount() const;

    // 获取粒子效果位置
    glm::vec2 getEffectPosition() const { return position; }

    // 强制结束
    void stop();

private:
    bool active;
    float timer;
    float duration;  // 3秒
    glm::vec2 position;
};
```

**渲染表现：**
- 角色蜷缩（SDF参数调整：身体缩小，手臂抱头）
- 身体颤抖（正弦波抖动）
- 泪珠粒子（小蓝色粒子从角色位置向下飘落）
- 屏幕边缘暗角逐渐消退

### 6. 后处理渲染管线

后处理需要将场景先渲染到FBO纹理，再采样该纹理施加效果。

**PostProcess.h/.cpp：**

```cpp
// PostProcess.h
#pragma once

#include <GL/glew.h>
#include <glm/vec2.hpp>

class PostProcess {
public:
    PostProcess();
    ~PostProcess();

    // 初始化FBO和全屏四边形
    bool init(int width, int height);

    // 窗口大小改变时重置FBO
    void resize(int width, int height);

    // 开始渲染到FBO
    void beginRender();

    // 结束FBO渲染，返回屏幕空间四边形用于后处理绘制
    void endRender();

    // 使用着色器绘制后处理结果到屏幕
    void draw(GLuint shaderProgram);

    // 设置uniform
    void setVignetteIntensity(float intensity);

    // 设置uniform变量（通用）
    void setUniform(const char* name, float value);
    void setUniform(const char* name, const glm::vec2& value);
    void setUniform(const char* name, const glm::vec3& value);

    // 获取屏幕纹理ID（供其他系统使用）
    GLuint getScreenTexture() const { return colorTexture; }

    // 获取全屏四边形VAO
    GLuint getQuadVAO() const { return quadVAO; }

private:
    GLuint fbo;
    GLuint colorTexture;
    GLuint quadVAO, quadVBO;
    int width, height;
};
```

**实现要点：**

```cpp
bool PostProcess::init(int w, int h) {
    width = w; height = h;

    // 1. 创建颜色纹理（渲染目标）
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 2. 创建FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "FBO incomplete\n");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. 创建全屏四边形（NDC空间 -1~1）
    float quadVerts[] = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return true;
}

void PostProcess::resize(int w, int h) {
    if (w == width && h == height) return;
    width = w;
    height = h;

    // 删除旧纹理和FBO
    glDeleteTextures(1, &colorTexture);
    glDeleteFramebuffers(1, &fbo);

    // 重新创建
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcess::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void PostProcess::endRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

void PostProcess::draw(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}
```

**后处理着色器（postprocess.vert/.frag）：**

```glsl
// postprocess.vert
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;
out vec2 vTexCoords;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vTexCoords = aTexCoords;
}

// postprocess.frag - 委屈暗角后处理
#version 330 core
in vec2 vTexCoords;
uniform sampler2D uScreenTexture;
uniform float uVignetteIntensity;  // 0-1，由委屈值决定（clamp）
out vec4 FragColor;

void main() {
    vec4 color = texture(uScreenTexture, vTexCoords);

    if (uVignetteIntensity > 0.0) {
        vec2 center = vec2(0.5, 0.5);
        float dist = distance(vTexCoords, center);
        // 暗角：边缘变暗，强度由委屈值控制
        float vignette = smoothstep(0.8, 0.3, dist * (1.0 + uVignetteIntensity * 0.5));
        color.rgb *= vignette;

        // 委屈时降低饱和度
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        float sat = 1.0 - uVignetteIntensity * 0.3;
        color.rgb = mix(vec3(gray), color.rgb, sat);
    }

    FragColor = color;
}
```

**主循环中的渲染顺序调整：**

```cpp
// 每帧渲染流程：
// 1. 绑定FBO，渲染场景到纹理
postProcess.beginRender();
glClear(GL_COLOR_BUFFER_BIT);

// 正常渲染场景（TileMap、角色、敌人、投射物、掉落、粒子）
renderTileMap(gs, tileColors, viewProj);
// ... 其他场景渲染 ...

postProcess.endRender();

// 2. 渲染后处理效果到屏幕
postProcess.setUniform("uVignetteIntensity",
    glm::clamp((emotionSystem.getState().grievance - 70.0f) / 30.0f, 0.0f, 1.0f));
postProcess.draw(postProcessShader);

// 3. 渲染UI（直接渲染到屏幕，不经过FBO）
// 使用正交投影直接绘制UI元素
renderUI(gs);
renderDialogueUI(gs);  // 对话框UI
```

### 6.1 对话UI渲染

对话UI使用Draw2D立即模式绘制，不经过FBO后处理。

```cpp
// DialogueUI.h
#pragma once

#include "DialogueTree.h"
#include <glm/vec3.hpp>

struct DialogueUIState {
    bool visible;
    std::string speakerName;
    std::string dialogueText;
    std::vector<std::string> choiceTexts;
    int selectedChoice;
    float typewriterTimer;
    int typewriterIndex;  // 逐字显示的当前字符索引
};

class DialogueUI {
public:
    void begin(const DialogueNode& node);
    void update(float dt);
    void render(const glm::mat4& orthoProj, int screenWidth, int screenHeight);

    // 输入处理
    void selectChoice(int index);
    void navigateUp();
    void navigateDown();
    void confirm();

    bool isVisible() const { return state.visible; }
    void hide() { state.visible = false; }

    int getSelectedChoice() const { return state.selectedChoice; }

private:
    DialogueUIState state;
    float boxWidth, boxHeight;
    float choiceBoxY;
};
```

**渲染实现：**

```cpp
void DialogueUI::render(const glm::mat4& orthoProj, int sw, int sh) {
    if (!state.visible) return;

    Draw2D::beginFrame(orthoProj);

    // 对话框背景（半透明深色矩形）
    float padding = 20.0f;
    float textHeight = 24.0f;
    int lines = (static_cast<int>(state.dialogueText.size()) / 30) + 1;
    boxHeight = padding * 2 + textHeight * std::max(lines, 2);
    boxWidth = sw - padding * 4;
    float boxX = padding * 2;
    float boxY = sh - boxHeight - padding;

    // 背景
    Draw2D::drawRectFilled(boxX, boxY, boxWidth, boxHeight, glm::vec3(0.1f, 0.1f, 0.15f));
    Draw2D::drawRect(boxX, boxY, boxWidth, boxHeight, glm::vec3(0.4f, 0.3f, 0.5f), 0.02f);

    // 说话者名字
    if (!state.speakerName.empty()) {
        // 名字标签背景
        float nameW = state.speakerName.size() * 14.0f + 20.0f;
        Draw2D::drawRectFilled(boxX + 10, boxY + boxHeight - 30, nameW, 25.0f,
                               glm::vec3(0.4f, 0.3f, 0.5f));
        // 这里应使用文字渲染（阶段4先用简单矩形占位，阶段8集成字体）
    }

    // 对话文本（逐字显示）
    std::string visibleText = state.dialogueText.substr(0, state.typewriterIndex);

    // 选项列表
    if (!state.choiceTexts.empty()) {
        choiceBoxY = boxY + padding;
        for (size_t i = 0; i < state.choiceTexts.size(); ++i) {
            float optY = choiceBoxY - i * 35.0f;
            bool selected = (i == static_cast<size_t>(state.selectedChoice));
            glm::vec3 optColor = selected ? glm::vec3(0.9f, 0.7f, 0.4f) : glm::vec3(0.7f);
            Draw2D::drawRectFilled(boxX + padding, optY, boxWidth - padding * 2, 28.0f,
                                   selected ? glm::vec3(0.2f, 0.15f, 0.25f) : glm::vec3(0.12f));
            Draw2D::drawRect(boxX + padding, optY, boxWidth - padding * 2, 28.0f,
                             optColor, 0.015f);
        }
    }

    Draw2D::endFrame();
}
```

**输入处理：**

```cpp
// 在main.cpp的SDL_KEYDOWN中
if (dialogueUI.isVisible()) {
    if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP) {
        dialogueUI.navigateUp();
    } else if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN) {
        dialogueUI.navigateDown();
    } else if (scancode == SDL_SCANCODE_J || scancode == SDL_SCANCODE_SPACE) {
        dialogueUI.confirm();
        if (dialogueUI.getSelectedChoice() >= 0) {
            dialogueTree.choose(dialogueUI.getSelectedChoice());
        } else {
            dialogueTree.next();
        }
    }
}
```

### 7. 文件结构与CMake更新

```
src/
├── main.cpp                      # 更新：集成情感、对话、NPC系统
├── Engine/
│   ├── Scripting/
│   │   └── LuaVM.h/.cpp          # 新增：Lua虚拟机
│   ├── Renderer/
│   │   ├── PostProcess.h/.cpp    # 新增：后处理FBO管线
│   │   └── DialogueUI.h/.cpp     # 新增：对话UI渲染
│   └── ...
├── Game/
│   ├── Social/
│   │   ├── NPC.h/.cpp            # 新增：NPC基础
│   │   ├── Princess.h/.cpp       # 新增：公主系统
│   │   └── DialogueTree.h/.cpp   # 新增：对话树
│   ├── Emotion/
│   │   ├── EmotionSystem.h/.cpp  # 新增：情感系统
│   │   └── VentAnimation.h/.cpp  # 新增：宣泄动画
│   └── ...
└── Utils/
    └── Math.h                    # 已有，补充工具函数

assets/
├── scripts/                      # 新增：Lua脚本（CMake复制到build/）
│   ├── dialogues/
│   │   ├── first_meeting.lua
│   │   └── daily_greetings.lua
│   ├── abilities.lua
│   ├── npc_schedules.lua
│   └── emotion_config.lua
└── shaders/
    └── postprocess.vert/.frag    # 新增：后处理
```

**CMakeLists.txt 更新：**

```cmake
# 添加新源文件
target_sources(Starchild2D PRIVATE
    src/Engine/Scripting/LuaVM.cpp
    src/Engine/Renderer/PostProcess.cpp
    src/Engine/Renderer/DialogueUI.cpp
    src/Game/Social/NPC.cpp
    src/Game/Social/Princess.cpp
    src/Game/Social/DialogueTree.cpp
    src/Game/Emotion/EmotionSystem.cpp
    src/Game/Emotion/VentAnimation.cpp
)

# 复制Lua脚本到构建目录（与着色器同样处理）
file(COPY ${CMAKE_SOURCE_DIR}/assets/scripts
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)

# 设置VS调试工作目录为项目根目录（可选，双重保险）
set_target_properties(Starchild2D PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
```

### 8. 主循环集成

在 `main.cpp` 中更新游戏循环：

```cpp
// 新增成员
LuaVM luaVM;
EmotionSystem emotionSystem;
VentAnimation ventAnimation;
PostProcess postProcess;
DialogueUI dialogueUI;
std::unique_ptr<Princess> princess;
DialogueTree dialogueTree;
bool isVenting = false;
float ventCooldown = 0.0f;

// 初始化（在initBox2D之后）
luaVM.init();
luaVM.bindGameAPI();
luaVM.loadFile("assets/scripts/abilities.lua");
luaVM.loadFile("assets/scripts/emotion_config.lua");

// 后处理初始化
postProcess.init(800, 600);
GLuint postProcessShader = createProgram(
    "assets/shaders/postprocess.vert",
    "assets/shaders/postprocess.frag");

// 创建公主小夏
princess = std::make_unique<Princess>("小夏");
princess->setColor(glm::vec3(0.9f, 0.6f, 0.8f));
princess->createBody(gs.worldId, glm::vec2(5.0f, 3.0f));
princess->setDialogueId("first_meeting");

// 家的区域定义
const glm::vec2 HOME_POS = glm::vec2(3.0f, 2.0f);
const float HOME_RADIUS = 3.0f;
auto isAtHome = [&](const glm::vec2& pos) {
    return glm::distance(pos, HOME_POS) <= HOME_RADIUS;
};

// 输入处理新增
if (scancode == SDL_SCANCODE_E && !gs.isDead) {
    if (dialogueUI.isVisible()) {
        // 对话中按E继续
        dialogueTree.next();
    } else if (princess && princess->canInteract(playerPos, 2.0f)) {
        // 靠近公主，开始对话
        dialogueTree.start("first_meeting");
    } else if (isAtHome(playerPos) && emotionSystem.getState().grievance > 30.0f) {
        // 在家且委屈值足够，宣泄
        isVenting = true;
        ventAnimation.start(playerPos);
        emotionSystem.vent();
    }
}

// 对话选择输入
if (dialogueUI.isVisible()) {
    if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP)
        dialogueUI.navigateUp();
    else if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN)
        dialogueUI.navigateDown();
    else if (scancode == SDL_SCANCODE_J || scancode == SDL_SCANCODE_SPACE) {
        dialogueUI.confirm();
        if (dialogueUI.getSelectedChoice() >= 0) {
            dialogueTree.choose(dialogueUI.getSelectedChoice());
        } else {
            dialogueTree.next();
        }
    }
}

// 更新阶段
emotionSystem.setAtHome(isAtHome(playerPos));
emotionSystem.update(dt);
if (isVenting) {
    ventAnimation.update(dt);
    if (!ventAnimation.isActive()) isVenting = false;
}
if (princess) princess->update(dt, gameTime);
dialogueTree.update(dt);

// 受伤时增加委屈值
playerHealth.setHurtCallback([&](const DamageInfo&) {
    emotionSystem.addGrievance(10.0f);
});
```

**玩家移动速度受委屈值影响：**

在阶段3已有的玩家移动代码中（`b2Body_ApplyForceToCenter` 之前），将力乘以速度修正系数：

```cpp
// 在物理步进前，计算玩家受力
b2Vec2 force = {0, 0};
if (gs.keys[SDL_SCANCODE_W] || gs.keys[SDL_SCANCODE_UP])    force.y += gs.playerForce;
if (gs.keys[SDL_SCANCODE_S] || gs.keys[SDL_SCANCODE_DOWN])  force.y -= gs.playerForce;
if (gs.keys[SDL_SCANCODE_A] || gs.keys[SDL_SCANCODE_LEFT])  force.x -= gs.playerForce;
if (gs.keys[SDL_SCANCODE_D] || gs.keys[SDL_SCANCODE_RIGHT]) force.x += gs.playerForce;

// 应用委屈值对移动速度的修正（委屈时减速）
float speedMult = emotionSystem.getSpeedMultiplier();
force.x *= speedMult;
force.y *= speedMult;

b2Body_ApplyForceToCenter(gs.playerBodyId, force, true);
```

**渲染流程调整（后处理FBO + UI直绘）：**

```cpp
// 1. 渲染场景到FBO
postProcess.beginRender();
glClear(GL_COLOR_BUFFER_BIT);

renderTileMap(gs, tileColors, viewProj);
// 渲染掉落、敌人、投射物、角色、粒子...

postProcess.endRender();

// 2. 后处理效果（暗角等）绘制到屏幕
postProcess.setUniform("uVignetteIntensity", emotionSystem.getPostProcessIntensity());
postProcess.draw(postProcessShader);

// 3. 直接渲染UI到屏幕（不经过FBO）
glm::mat4 uiProj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

// 玩家血条
renderUI(gs);

// 对话UI
if (dialogueTree.isActive()) {
    dialogueUI.render(uiProj, 800, 600);
}

// 4. 蒙头哭粒子效果（叠加在后处理之上）
if (isVenting) {
    Draw2D::beginFrame(viewProj);
    // 泪珠粒子...
    Draw2D::endFrame();
}
```

### 9. 验证步骤

1. **Lua系统验证**：
   - 成功加载 `abilities.lua`，技能参数可读取
   - `doString("return 1+1")` 返回 2
   - 无内存泄漏，Lua状态正确销毁
   - 脚本文件从 `build/assets/scripts/` 正确加载

2. **对话树验证**：
   - 靠近公主按E触发对话
   - 对话框正确显示（背景、说话者、文本、选项）
   - 选项可用W/S键选择，J/空格确认
   - 选择后好感度正确变化
   - 对话结束后对话框消失

3. **对话UI验证**：
   - 对话框半透明背景正常渲染
   - 选项高亮正确
   - UI不经过FBO后处理（暗角不影响对话框可见性）

4. **公主系统验证**：
   - 小夏按日程在地图各处出现
   - 不同时段在不同位置（验证跨夜日程处理）
   - 好感度系统正常工作，5个等级正确切换
   - 好感度影响对话内容

5. **情感系统验证**：
   - 受伤时委屈值上升（+10）
   - 委屈值70+时屏幕出现暗角，强度随委屈值渐变
   - 移动速度在委屈时降低到0.7倍
   - 在家按E宣泄，委屈值清零，暗角消退
   - 在家时委屈值自然衰减（-1/分钟）

6. **后处理管线验证**：
   - FBO渲染正常，无花屏
   - 暗角效果正确叠加到场景上
   - 帧率无明显下降（FBO渲染开销可接受）

7. **蒙头哭验证**：
   - 宣泄时角色蜷缩颤抖
   - 泪珠粒子效果
   - 3秒后恢复正常
   - 委屈值清零

8. **集成验证**：
   - 完整的互动循环：战斗受伤→委屈值上升→找公主对话/回家宣泄→恢复
   - 帧率稳定60FPS
   - 无Lua脚本崩溃
   - 所有资源正确释放

### 10. 验收标准

- [ ] Lua脚本系统正常工作，可加载和执行脚本
- [ ] Lua脚本文件正确复制到build目录
- [ ] 对话树系统完整，支持分支选择和好感度变化
- [ ] 对话UI正确渲染（背景框、选项高亮、键盘选择）
- [ ] 公主小夏可按日程在地图中移动（含跨夜处理）
- [ ] 好感度系统有5个等级，阈值正确，影响对话和行为
- [ ] 委屈值系统完整，受伤增加，宣泄清零，在家自然衰减
- [ ] 后处理FBO管线正常工作，暗角效果正确叠加
- [ ] 委屈值70+触发暗角后处理（强度clamp到[0,1]）和减速
- [ ] 蒙头哭动画（蜷缩、颤抖、泪珠粒子）
- [ ] 所有Lua脚本无语法错误
- [ ] 帧率稳定60FPS
- [ ] 无内存泄漏，RAII管理Lua状态
- [ ] 代码编译无警告（`/W4`）

### 11. 扩展预留

本阶段设计预留了以下扩展点：

1. **更多公主**：`Princess` 类可实例化多个对象，每个有独立的日程和好感度
2. **更多对话**：对话树支持从Lua动态加载任意对话文件
3. **约会事件**：预留 `triggerSpecialEvent()` 用于阶段5的约会系统
4. **天气/时间影响**：日程系统基于 `gameTime`，可扩展天气影响
5. **羁绊技**：`ultimateCharge` 已预留，阶段5实现合体技

这些扩展将在阶段5-6中逐步实现。
