# 阶段2：渲染与绘制系统实现计划

## Context

阶段1已完成最小可运行原型：SDL2窗口、OpenGL上下文、Box2D物理世界、WASD控制的方块。阶段2目标是建立完整的2D渲染与绘制系统，为后续的角色绘制、特效、地图系统打下基础。

**阶段2具体目标（来自技术栈.md）：**
1. 编写2D绘图函数（矩形、圆、线条、渐变）
2. 实现摄像机跟随
3. 引入GLSL角色SDF绘制：绘制玩家几何体，动画参数通过uniform传入（测试挥手）
4. 瓦片地图系统（简单数组，绘制草地/路）

## 实现方案

### 1. 2D绘图函数库

在 `src/Engine/Renderer/` 下创建 `Draw2D.h` 和 `Draw2D.cpp`，提供即时模式2D绘图API：

```cpp
// Draw2D.h
namespace Draw2D {
    void init();
    void shutdown();
    void beginFrame(const glm::mat4& viewProj);
    void endFrame();

    // 基础图形
    void drawRect(float x, float y, float w, float h, const glm::vec3& color);
    void drawRectFilled(float x, float y, float w, float h, const glm::vec3& color);
    void drawCircle(float cx, float cy, float r, const glm::vec3& color, int segments = 32);
    void drawLine(float x1, float y1, float x2, float y2, const glm::vec3& color, float thickness = 1.0f);

    // 渐变
    void drawRectGradient(float x, float y, float w, float h,
                          const glm::vec3& topLeft, const glm::vec3& topRight,
                          const glm::vec3& bottomLeft, const glm::vec3& bottomRight);
    void drawCircleGradient(float cx, float cy, float r,
                            const glm::vec3& innerColor, const glm::vec3& outerColor);
}
```

**实现要点：**
- 使用单个VAO + VBO的批处理策略，所有2D图形共享一个着色器程序
- 采用"一次性提交"策略：每帧 `beginFrame()` 清空顶点缓存，各绘制调用将顶点推入同一个 vector，`endFrame()` 时一次性 `glBufferData` + `glDrawArrays`
- 矩形：2个三角形（6顶点），圆：三角形扇（N顶点），线条：拉伸矩形
- 渐变：顶点颜色插值（矩形4顶点各赋不同颜色）
- 着色器使用正交投影矩阵，世界坐标直接映射到NDC

**着色器 `assets/shaders/draw2d.vert/.frag`：**
```glsl
// draw2d.vert
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
uniform mat4 uViewProj;
out vec3 vColor;
void main() {
    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}

// draw2d.frag
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
```

### 2. 摄像机系统

在 `src/Engine/Camera/Camera2D.h` 中实现：

```cpp
struct Camera2D {
    glm::vec2 position{0.0f, 0.0f};
    float zoom = 1.0f;        // >1 放大, <1 缩小
    float rotation = 0.0f;    // 弧度

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float screenWidth, float screenHeight) const;
    glm::mat4 getViewProjMatrix(float screenWidth, float screenHeight) const;

    // 屏幕坐标转世界坐标
    glm::vec2 screenToWorld(float sx, float sy, float screenWidth, float screenHeight) const;
    // 世界坐标转屏幕坐标
    glm::vec2 worldToScreen(float wx, float wy, float screenWidth, float screenHeight) const;
};
```

**跟随逻辑：**
- 摄像机目标位置 = 玩家位置 + 偏移量
- 使用指数衰减平滑跟随：`t = 1 - exp(-followSpeed * dt)`，然后 `cam.position = lerp(cam.position, target, t)`
  - 相比 `lerp(cam.position, target, speed * dt)`，指数衰减公式保证不同帧率下跟随行为一致
  - `followSpeed` 控制跟随快慢（如 5.0 表示较快跟随）
- 支持边界限制（可选，防止看到地图外区域）

### 3. GLSL SDF 角色绘制

创建 `assets/shaders/character.frag`，使用有符号距离函数(SDF)组合绘制角色几何体：

```glsl
// character.frag - SDF角色绘制
#version 330 core
in vec2 vWorldPos;  // 从顶点着色器传入的世界坐标
uniform vec2 uPosition;     // 角色世界位置
uniform float uTime;        // 动画时间
uniform vec3 uBodyColor;    // 身体颜色
uniform int uExpression;    // 表情类型：0=普通, 1=开心, 2=委屈
out vec4 FragColor;

// SDF基础函数
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

float sdRoundedBox(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return length(max(q, 0.0)) - r + min(max(q.x, q.y), 0.0);
}

float sdCapsule(vec2 p, float len, float rad) {
    p.y = abs(p.y) - len;
    return length(max(vec2(p.x, p.y), 0.0)) + min(max(p.x, p.y), 0.0) - rad;
}

void main() {
    vec2 uv = vWorldPos - uPosition;

    // 身体 = 圆角矩形
    float body = sdRoundedBox(uv, vec2(0.4, 0.5), 0.1);

    // 眼睛（两个小圆）
    float eyeOffset = 0.15;
    float blink = smoothstep(0.95, 1.0, sin(uTime * 2.0)); // 眨眼
    float eyeScale = mix(1.0, 0.1, blink);
    float leftEye = sdCircle(uv - vec2(-eyeOffset, 0.15), 0.06 * eyeScale);
    float rightEye = sdCircle(uv - vec2(eyeOffset, 0.15), 0.06 * eyeScale);

    // 嘴巴（根据表情变化）
    float mouth = 0.0;
    if (uExpression == 0) {
        // 普通：弧线
        mouth = -uv.y - 0.1 - 0.05 * uv.x * uv.x;
    } else if (uExpression == 1) {
        // 开心：上弯弧
        mouth = -uv.y - 0.05 + 0.1 * uv.x * uv.x;
    } else {
        // 委屈：下弯弧
        mouth = -uv.y - 0.15 - 0.1 * uv.x * uv.x;
    }

    // 组合SDF（取最小值 = 并集）
    float character = min(body, min(min(leftEye, rightEye), mouth));

    // 抗锯齿边缘
    float alpha = 1.0 - smoothstep(0.0, 0.02, character);

    // 身体颜色
    vec3 color = uBodyColor;

    // 眼睛白色
    if (leftEye < 0.0 || rightEye < 0.0) {
        color = vec3(1.0);
    }

    FragColor = vec4(color, alpha);
}
```

**顶点着色器 `character.vert`：**
```glsl
#version 330 core
layout(location = 0) in vec2 aPos;  // 角色包围盒四边形顶点（世界坐标）
uniform mat4 uViewProj;
out vec2 vWorldPos;
void main() {
    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
    vWorldPos = aPos;  // 传递世界坐标，供片段着色器计算局部坐标
}
```

**C++ 端顶点数据生成（每帧更新）：**

```cpp
// 角色包围盒：宽 2*s，高 2*sh
float s = 1.0f;   // 半宽
float sh = 1.5f;  // 半高
GLfloat verts[] = {
    px - s, py - sh,  // 左下
    px + s, py - sh,  // 右下
    px + s, py + sh,  // 右上
    px - s, py + sh,  // 左上
};
// glBufferSubData 更新 VBO，然后 glDrawArrays(GL_TRIANGLE_FAN, 0, 4)
```

**渲染方式：**
- 绘制一个覆盖角色包围盒的四边形（2个三角形）
- 片段着色器中用SDF判断像素是否在角色内部
- 动画参数（眨眼、挥手、表情）通过uniform传入

### 4. 瓦片地图系统

创建 `src/Game/World/TileMap.h/.cpp`：

```cpp
enum class TileType : uint8_t {
    Grass = 0,
    Dirt = 1,
    Stone = 2,
    Water = 3,
    Wall = 4,
    Count
};

struct TileMap {
    int width, height;
    float tileSize;
    std::vector<TileType> tiles;  // 行优先: tiles[y * width + x]

    void load(const char* filename);  // 从简单文本文件加载
    void generate(int seed);           // 程序化生成（确定性伪随机）

    TileType getTile(int x, int y) const;
    bool isSolid(int x, int y) const;  // Wall/Water为不可通行

    // 世界坐标转瓦片坐标
    glm::ivec2 worldToTile(float wx, float wy) const;
    // 瓦片坐标转世界坐标
    glm::vec2 tileToWorld(int tx, int ty) const;
};
```

**地图数据格式（简单文本）：**
```
10 8
0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0
0 0 4 4 4 0 0 0 0 0
0 0 4 0 4 0 0 0 0 0
0 0 4 0 4 0 0 0 0 0
0 0 0 0 0 0 3 3 0 0
0 0 0 0 0 0 3 3 0 0
0 0 0 0 0 0 0 0 0 0
```

**渲染策略：**
- 每帧只绘制摄像机视锥内的瓦片（视锥剔除）
- 小地图（≤50×50）：每帧动态提交顶点数据，复用 draw2d 着色器
- 大地图：预烘焙为静态 VBO（加载时生成一次），仅更新视锥剔除后的绘制范围
- 进阶优化：使用实例化绘制（`glDrawArraysInstanced`）批量提交同类型瓦片

**物理碰撞处理：**
- 对于不可通行的瓦片（Wall、Water），在地图加载时为每个固体瓦片创建 Box2D 静态刚体
- 使用 `b2MakeBox(tileSize * 0.45f, tileSize * 0.45f)` 作为碰撞形状（略小于瓦片尺寸，避免缝隙卡住）
- 玩家和动态对象通过 Box2D 物理引擎与这些静态刚体碰撞，实现自动阻挡
- 这种纯物理驱动方式与阶段1保持一致，无需额外的逻辑碰撞检测

### 5. 文件结构更新

```
e:/DouBao2/Project/KatCode/Project02/
├── CMakeLists.txt           # 更新：添加新源文件
├── vcpkg.json
├── src/
│   ├── main.cpp             # 更新：集成新系统
│   ├── Engine/
│   │   ├── Renderer/
│   │   │   ├── Draw2D.h
│   │   │   ├── Draw2D.cpp
│   │   │   └── Renderer.h   # 统一管理着色器、VAO等
│   │   └── Camera/
│   │       └── Camera2D.h
│   └── Game/
│       └── World/
│           ├── TileMap.h
│           └── TileMap.cpp
├── assets/
│   └── shaders/
│       ├── basic.vert/.frag     # 保留（阶段1三角形）
│       ├── draw2d.vert/.frag    # 2D绘图着色器
│       ├── character.vert/.frag # 角色SDF着色器
│       └── tile.vert/.frag      # 瓦片着色器（可选，可用draw2d复用）
└── doc/
    ├── plan.md                  # 阶段1计划
    └── plan2.md                 # 本文件
```

### 6. CMakeLists.txt 更新

```cmake
# 添加新源文件
add_executable(Starchild2D
    src/main.cpp
    src/Engine/Renderer/Draw2D.cpp
    src/Game/World/TileMap.cpp
)

# 添加新着色器复制
file(COPY ${CMAKE_SOURCE_DIR}/assets/shaders
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)
```

### 7. 验证步骤

1. **绘图函数验证**：在窗口中绘制测试图形（红色矩形、蓝色圆、绿色线条、渐变矩形），确认正确显示
2. **摄像机验证**：按方向键移动摄像机，场景内容跟随移动；缩放功能正常
3. **SDF角色验证**：屏幕上显示一个几何风格的角色（圆角矩形身体+眼睛+嘴巴），眨眼动画正常运行，切换表情uniform能看到不同嘴型
4. **瓦片地图验证**：加载测试地图文件，正确绘制草地、墙壁、水域；摄像机移动时视锥剔除正确
5. **集成验证**：将阶段1的玩家方块替换为SDF角色，角色站在瓦片地图上，摄像机跟随角色移动，WASD控制角色在地图上行走

### 8. 验收标准

- [ ] 所有2D绘图函数（矩形、圆、线条、渐变）正常工作
- [ ] 摄像机平滑跟随玩家，支持缩放
- [ ] SDF角色渲染正确，动画参数可调节
- [ ] 瓦片地图正确加载和渲染，支持至少100x100规模地图
- [ ] 帧率稳定60FPS（1080p窗口）
- [ ] 无内存泄漏（使用RAII管理GL资源）
- [ ] 代码编译无警告（`/W4`）
