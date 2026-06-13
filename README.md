# 星愿之子 / Starchild2D

> 我会长大，但不交出童心。
> I will grow up, but I will not hand over my colors.

**《星愿之子》** 是一款 C++17 开发的 2D 俯视角动作 RPG。它把六一傍晚的小卖部、旧游戏币、弹窗小游戏、黑屏电视、课间十分钟和那些没讲完的故事，做成一个可以探索、战斗、存档、失败后再站起来的怀旧童心宇宙。

**Starchild2D** is a C++17 top-down action RPG about a glowing star candy, a secret layer of childhood beneath reality, and the brave decision to grow up without becoming gray inside.

它有魔法、Boss、秘密基地和会嘴硬的公主；也有工程结构、存档边界、任务流、背包系统和一条清楚的垂直切片计划。浪漫归浪漫，构建失败的时候 CMake 可不会因为你童心值高就心软。

It is warm, playful, and a little ridiculous in the right places. It is also built like an actual game project, because nostalgia still needs deterministic save data.

---

## 故事 / Story

2026 年 6 月 1 日傍晚，16 岁的高中生 **星愿** 在小卖部门口捡到一颗发光的星星糖。糖在他手心里冷却成一枚淡蓝色旧游戏币，他把它挂在脖子上，然后现实地面裂开，彩色的光把他带进了 **童心层**。

那里不是梦，也不是回忆文件夹。那里是现实之下的一层世界，原本保存着孩子们的勇气、好奇、想象力和“再玩五分钟”的坚定信念。现在，**灰钟局** 正在把它折叠进现实层，让所有人走向一条标准、安静、没有颜色的成长路线。

星愿遇见了忍饥挨饿但绝不承认自己需要帮助的 **艾莉娅**。她是第九位公主，守护的不是某个单独领域，而是童心层本身。两人从互相嘴硬，到一起闯过弹窗游乐厅、雪花放映厅、纸皮英雄基地、铃声操场、问号实验室，最终登上灰钟塔，面对灰钟局长。

On June 1st, 2026, sixteen-year-old Xingyuan picks up a glowing star candy outside a corner shop. It cools into a translucent blue game token, opens a crack in reality, and drops him into the **Childlike Layer**.

This hidden world is being compressed by the **Gray Bell Bureau**, an institution obsessed with one tidy version of growing up. Xingyuan meets **Alya**, a stubborn princess who insists she does not need help, especially while visibly starving. Together they rescue guardians, restore color to nostalgic domains, and challenge the idea that maturity must be gray.

The heart of the story is simple:

> 长大不是把童年锁起来。长大是带着它继续走。
> Growing up does not mean locking childhood away. It means carrying it forward.

---

## 核心体验 / Core Experience

| 中文 | English |
|---|---|
| **秘密基地 Hub**：休息、保存、整理背包、查看任务、和艾莉娅斗嘴。 | **Secret Base Hub**: rest, save, sort inventory, track quests, and trade stubborn comments with Alya. |
| **章节领域副本**：每章一个童年主题场景，一个独特机制，一个守护伙伴，一个 Boss。 | **Chapter Domains**: each chapter has a nostalgic theme, a signature mechanic, a rescued guardian, and a boss. |
| **童心值动态技能**：童心越亮，技能名字、粒子、伤害、画面饱和度和隐藏物可见性都会变化。 | **Childlike Heart Skill Tiers**: your skills, particles, damage, world color, and hidden discoveries react to your heart level. |
| **怀旧不是复刻**：主线角色和 Boss 原创化，致敬元素放在道具描述、闲聊、贴纸、海报和彩蛋房里。 | **Homage, Not Cloning**: main characters and bosses are original; references live in item text, banter, stickers, posters, and secrets. |
| **反派像压力，不像坏人甲**：灰钟局的敌人常常是规则、格式、弹窗、排名、标准答案。 | **Villains Feel Like Pressure**: rules, formats, popups, rankings, and “standard answers” are just as dangerous as monsters. |

---

## 当前制作目标 / Current Production Target

完整剧情覆盖序章、八个主章节、隐藏章和终章；当前第一轮实现聚焦一个可玩的正式垂直切片：

1. 主菜单。
2. 序章：星星糖穿越，遇见艾莉娅。
3. 秘密基地 Hub：地图桌、存档床、任务、属性、背包、系统菜单。
4. 第一章：弹窗游乐厅。
5. 救出伙伴 **铁翼**。
6. 击败 Boss **六元冠冕**。
7. 带 **像素手柄** 回基地，解锁基地变化。
8. 覆盖上述进度的存档与读档。

The full narrative includes a prologue, eight main domains, an optional hidden chapter, and a finale. The current production milestone is the first playable vertical slice: prologue, secret base, in-game menu, inventory, quest log, childlike-heart skill tiers, Popup Arcade, Tieyi rescue, Six-Yuan Crown boss, and save/load continuity.

这份 README 描述的是新的叙事方向和第一轮生产目标，不代表所有章节都已经实装。换句话说：灰钟塔已经在设计里等你，但它还没有完全搬进可执行文件。

This README describes the new narrative direction and first production target. Not every chapter is playable yet. The Gray Bell Tower is waiting in the design, not fully in the executable.

---

## 章节路线 / Chapter Roadmap

| 顺序 | 中文章节 | English Chapter | 核心机制 |
|---:|---|---|---|
| 0 | 序章：星星糖穿越 | Prologue: Star Candy Crossing | 移动、互动、童心值教学 |
| Hub | 秘密基地 | Secret Base | 任务、背包、存档、恢复、基地展示物 |
| 1 | 弹窗游乐厅 | Popup Arcade | 弹窗遮挡、诱导按钮、试玩币机关 |
| 2 | 雪花放映厅 | Snow-Screen Theater | 色彩通道、黑屏记忆、光束显形 |
| 3 | 纸皮英雄基地 | Cardboard Hero Base | 真假危险、救援、勇气槽 |
| 4 | 反面童话森林 | Rewritten Fairy-Tale Forest | 故事页、规则改写、命运书签 |
| 5 | 铃声操场 | Bell-Ring Playground | 上课 / 课间切换、试卷墙、冲刺 |
| 6 | 跑调童谣镇 | Off-Key Nursery Town | 节拍道路、三轨舞台、噪音扰乱 |
| 7 | 断格漫画城 | Broken-Panel Comic City | 分镜格、进度回滚、未完待续屏障 |
| 8 | 问号实验室 | Question-Mark Laboratory | 火冰电风实验、创造性错误、标准答案屏障 |
| ? | 回声音乐厅 | Echo Music Hall | 十二乐器复调、隐藏结局条件 |
| End | 灰钟塔 | Gray Bell Tower | 前章机制回归、标准化封印、最终选择 |

---

## 主要角色 / Cast

| 角色 | Role | 说明 |
|---|---|---|
| 星愿 / Xingyuan | 主角 / Protagonist | 16 岁高中生，星星糖选中的人。外冷内热，关键时刻会帅一下，之后可能装作没有发生。 |
| 艾莉娅 / Alya | 第九公主 / Ninth Princess | 守护童心层本身。嘴硬、温柔、饿了也要先讲道理。 |
| 铁翼 / Tieyi | 第一章伙伴 / Chapter 1 Guardian | 红蓝机器人少年，火箭核心恢复后提供破盾支援。 |
| 灰钟局长 / Director of the Gray Bell Bureau | 终章反派 / Final Antagonist | 不是单纯作恶，而是试图用“标准成长”保护所有人，结果差点把颜色保护没了。 |
| 六元冠冕 / Six-Yuan Crown | 第一章 Boss / Chapter 1 Boss | 漂浮广告皇冠。它的职业理想是让你点击“立即变强”，你的职业理想是让它闭嘴。 |

更多伙伴会在后续章节中加入：星牌师洛宸、纸甲工程师阿洛、键盘战士赤弦、星拳守护者承野、蓝袋工匠多洛、铃羊发明家乐铃、云棍少年悟星，以及隐藏章的回音。

More guardians join later chapters, including star-card tacticians, cardboard engineers, rhythm helpers, inventors, and one very serious argument against the phrase “there is only one correct answer.”

---

## 系统亮点 / Systems

- **童心值 / Childlike Heart**: 0-1000 的长期资源，影响后处理、移动、对话、技能表现、隐藏物和结局判断。
- **委屈值 / Grievance**: 失败和压力的积累，不是为了惩罚玩家，而是让基地恢复和情绪回报有意义。
- **任务日志 / Quest Log**: 支持主线、领域支线和基地任务，目标可以是对话、收集、击败、互动、进入区域和通关 Boss。
- **通用背包 / Inventory**: 消耗品、材料、剧情物、玩具 / 家具、领域信物、隐藏收集物。
- **章节进度 / Story Progress**: 记录章节状态、伙伴解锁、Boss 奖励、隐藏章 flag 和多结局条件。
- **Boss 机制 / Boss Design**: 每个 Boss 至少有一个本章机制、一个压力诱导点、一个清晰反制点和一个高童心奖励条件。
- **程序化发光像素表现 / Procedural Glow-Pixel Look**: 第一轮先用稳定尺寸的程序化像素角色和特效，后续可替换正式 sprite 素材管线。

---

## 技术栈 / Tech Stack

| 领域 | Stack |
|---|---|
| 语言 / Language | C++17 |
| 构建 / Build | CMake + vcpkg |
| 图形 / Graphics | OpenGL 3.3 Core |
| 物理 / Physics | Box2D v3, top-down zero-gravity setup |
| 脚本 / Scripting | Lua + sol2 |
| 存档 / Save Data | Structured game snapshot and migration path |
| 内容方向 / Content Direction | Secret Base Hub + Chapter Domains |

---

## 快速开始 / Quick Start

前提：安装 vcpkg，并设置 `VCPKG_ROOT`。

Prerequisite: install vcpkg and set `VCPKG_ROOT`.

```bash
cmake -B build
cmake --build build --config Release
```

从 `build/` 目录运行游戏，否则相对资源路径会找不到 `assets/`。这是电脑在提醒你：小卖部可以乱逛，工作目录不能乱跑。

Run from `build/` so relative asset paths resolve correctly.

```bash
cd build
./Release/Starchild2D.exe
```

Debug 构建 / Debug build:

```bash
cmake --build build --config Debug
```

Visual Studio 工程 / Visual Studio solution:

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
```

---

## 操作 / Controls

| 输入 | 功能 | Notes |
|---|---|---|
| `WASD` | 移动 / Move | 童年没有自动寻路，秘密基地也没有。 |
| `J` | 火系投射 / Fire skill | 从打火机到星愿焰火，取决于童心值。 |
| `L` | 冰系投射 / Ice skill | 降温可以给敌人，也可以给补课总监的 KPI。 |
| `Q` | 连锁闪电 / Chain lightning | 适合处理“我就站这儿碍你事了吗”的敌群。 |
| `F` | 防御 / Shield | 暂不保证能挡住家长会。 |
| `Space` | 位移 / Flight or dash | 童心够高时，世界会同意你飞一会儿。 |
| `K` | 抓取 / 投掷 / Grab or throw | 怪力是童年幻想的一种合理表达。 |
| `E` | 互动 / Interact | 对话、调查、触发机关。 |
| `Tab` / `Esc` | 游戏内菜单 / In-game menu | 任务、属性、背包、伙伴、系统。 |
| `F5` | 快速存档 / Quick save | 防止“刚才那段能不能重来”的经典时刻。 |
| `F9` | 快速读档 / Quick load | 给童心一次从检查点站起来的机会。 |
| 鼠标滚轮 / Mouse wheel | 缩放视角 / Zoom | 近看是冒险，远看是地图编辑器的尊严。 |

---

## 项目结构 / Project Structure

```text
src/
├── main.cpp          # Application setup and game loop
├── Engine/           # Renderer, physics, camera, scripting
├── Game/             # World, abilities, AI, quests, inventory, saves, UI
└── Utils/            # Shared helpers

assets/
├── shaders/          # GLSL shaders
└── scripts/          # Lua gameplay and dialogue scripts

doc/
└── 怀旧主题-*.md      # Narrative, cast, maps, bosses, music, promo ideas

docs/superpowers/
├── specs/            # Design specs
└── plans/            # Implementation plans
```

`build/` 是生成目录，请把它当作自动售货机吐出来的小票：有用，但不手改。

`build/` is generated output. Useful, but not hand-authored.

---

## 设计文档 / Design References

- [`doc/怀旧主题-完整剧情.md`](doc/怀旧主题-完整剧情.md) — 新主线剧情、章节、结局和叙事语气。
- [`docs/superpowers/specs/2026-06-10-starchild-nostalgia-rework-design.md`](docs/superpowers/specs/2026-06-10-starchild-nostalgia-rework-design.md) — 怀旧主题重构设计。
- [`docs/superpowers/plans/2026-06-10-starchild-nostalgia-rework.md`](docs/superpowers/plans/2026-06-10-starchild-nostalgia-rework.md) — 第一轮垂直切片实施计划。
- [`doc/技术栈.md`](doc/技术栈.md) — 技术路线与阶段规划。

---

## 开发原则 / Development Principles

- 基于现有架构扩展，不推倒重来。
- 先证明“序章 + 秘密基地 + 第一章”完整循环，再扩展后续章节。
- 玩法相关地图改动通过 `MapTileManager` 同步物理和持久化。
- 新增 `.cpp` 文件需要显式加入 `CMakeLists.txt`。
- 运行时从 `build/` 启动，确认 CMake 已复制 `assets/`。
- 渲染、UI、剧情和战斗可以很有童心；存档、任务和进度状态必须像班主任点名一样可靠。

- Extend the existing architecture instead of replacing it.
- Prove the prologue, secret base, and first chapter loop before scaling the rest.
- Use `MapTileManager` for gameplay-relevant tile edits so physics and persistence stay synchronized.
- Add new `.cpp` files explicitly to `CMakeLists.txt`.
- Run from `build/` and confirm assets are copied.
- Keep the story playful; keep save data boring in the best possible way.

---

## 童心声明 / Childlike-Heart Statement

我们不是要做一款拒绝长大的游戏。

我们想做的是一款承认“长大很难”的游戏：你会遇到标准答案、黑屏、排名、弹窗、断更和很多听起来很有道理的灰色规则。但你也会遇到艾莉娅、秘密基地、像素手柄、恢复糖、会发光的旧游戏币，以及那些愿意陪你把颜色找回来的人。

所以，六一快乐。不管今天是不是 6 月 1 日。

This is not a game about refusing to grow up.

It is a game about growing up with witnesses: a stubborn princess, a secret base, a pixel controller, a glowing token, and enough recovered color to keep walking.

Happy Children’s Day, even when the calendar disagrees.
