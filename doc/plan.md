# 阶段1：最小可运行原型实现计划

## Context

项目是一个全新的2D俯视视角动作RPG《星愿之子》（Starchild），当前只有技术栈文档和VS Code调试配置，没有任何代码。阶段1目标是搭建基础开发环境并实现最小可运行原型：一个可以用键盘移动的方块在Box2D物理世界中运动。

**阶段1具体目标（来自技术栈.md）：**
1. 搭建CMake、SDL2窗口、OpenGL上下文
2. 绘制一个彩色三角形（GLSL简单着色器）
3. 实现Box2D世界，创建静态地面和动态方块（玩家）
4. 键盘移动控制

## 实现方案

### 依赖管理

使用 **vcpkg** 管理所有C++依赖。创建 `vcpkg.json` manifest文件，指定以下依赖：
- `sdl2` - 窗口和输入
- `glew` - OpenGL扩展加载（现代替代方案是glad，但GLEW更简单且vcpkg支持良好）
- `box2d` - 2D物理引擎（vcpkg包名小写，链接目标为 `box2d::box2d`）
- `glm` - 数学库
- `sol2` - Lua绑定（阶段1暂不使用，但预先配置）

**注意**: GLEW需要在创建OpenGL上下文后调用 `glewInit()`，且需设置 `glewExperimental = GL_TRUE` 以支持OpenGL 3.3 Core。

### 文件结构

```
e:/DouBao2/Project/KatCode/Project02/
├── CMakeLists.txt           # 新建 - 构建配置
├── vcpkg.json               # 新建 - vcpkg依赖清单
├── src/
│   └── main.cpp             # 新建 - 游戏入口和主循环
├── assets/
│   └── shaders/
│       ├── basic.vert       # 新建 - 基础顶点着色器
│       └── basic.frag       # 新建 - 基础片段着色器（彩色三角形）
└── 技术栈.md                # 已存在
```

### 关键实现细节

#### 1. CMakeLists.txt
- 设置C++17标准
- 通过vcpkg manifest模式自动安装依赖
- 链接SDL2、GLEW、Box2D（`box2d::box2d`）、GLM
- 配置着色器文件复制到构建目录：
  ```cmake
  file(COPY ${CMAKE_SOURCE_DIR}/assets/shaders 
       DESTINATION ${CMAKE_BINARY_DIR}/assets/)
  ```

#### 2. main.cpp 主循环
- SDL2初始化（视频+事件）
- OpenGL 3.3 Core上下文创建（`SDL_GL_SetAttribute` 设置版本和Core profile）
- GLEW初始化（`glewExperimental = GL_TRUE` + `glewInit()`）
- 游戏主循环：事件处理 → 物理步进 → 渲染 → swap buffers
- 固定60FPS时间步长（dt=1/60）

#### 3. 着色器
- **basic.vert**: 接收顶点位置和颜色属性，输出到gl_Position
- **basic.frag**: 接收插值颜色，输出片元颜色

#### 4. Box2D物理
- 创建b2World（重力向下）
- 静态地面（长条矩形，底部）
- 动态玩家方块（受重力和键盘力）
- 碰撞回调框架

#### 5. 渲染
- 使用VAO/VBO绘制三角形（RGB三色顶点）
- 玩家方块用简单矩形绘制
- 清屏颜色设为深色背景

### 验证步骤

1. **编译验证**: `cmake -B build` 和 `cmake --build build` 成功，无错误
2. **运行验证**: 程序启动后显示窗口，能看到彩色三角形
3. **物理验证**: 玩家方块受重力下落，与地面碰撞后停止
4. **控制验证**: WASD/方向键可以对方块施加力，方块在物理世界中移动和碰撞
5. **帧率验证**: 窗口标题显示FPS，稳定在60左右
