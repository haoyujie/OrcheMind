# Cursor Prompt：OMStudio 渲染升级到 JEV Twisted Torus 电影感效果

> 用法：把下面「═══ 复制以下全部内容 ═══」之间的文字整体粘贴给 Cursor（建议在项目根目录对话，附上 `apps/viewer/viewer-shot.png` 与参考视频帧 `build/ref/frame_001.png`、`frame_003.png`、`frame_005.png`、`frame_008.png` 作为视觉参照）。

═══ 复制以下全部内容 ═══

## 任务
把 OMStudio 3D 查看器的渲染升级为参考视频（JEV ENGINEERING 的 TWISTED TORUS / SMOKE-RING VORTEX）那种电影感效果：霓虹发光粒子流、流光拖尾、全屏 bloom 泛光、扭曲烟圈环面、持续动态旋转。**现有交互与数据编辑功能必须全部保留**。

## 工程背景
- 路径：`G:\myfuture\OrcheMind\OMStudio`，C++17 + Magnum(GL330) + GLFW，MinGW 构建（现有 `build-viewer` 已能构建，沿用同一套 CMake 配置，不要改 triplet/工具链）。
- 渲染管线主文件：`apps/viewer/ViewerApp.cpp`（约 590 行）：曲面网格绘制、实例化节点、线段、TextCache 中文标签、HUD 面板、相机/拾取/聚焦/输入。
- 曲面参数化：`apps/viewer/SurfaceMesh.cpp` / `.h`（已有 `Torus`、`Hyperboloid`、`InnerTorus` 三种）。
- 中文标签：`apps/viewer/TextCache.cpp`（GDI 生成纹理，不要动）。
- 场景/拓扑/空间索引在 `omstudio_core`（`include/omstudio/*`），**本次不要修改核心层**，只改渲染。

## 当前效果（见 viewer-shot.png）
深蓝半透明曲面网格 + 绿色/粉色小球节点 + 白色线段 + 黑色文字标签（"生长/关联 0.xx"）+ 右侧 HUD 面板。静态、平铺、无光效、无动效。

## 目标效果（见参考帧）
1. **扭曲环面**：环面呈"烟圈涡旋"质感——三瓣（3 lobes）、截面沿环向扭转（twist ≈ 1/2 圈）、管面带半透明流光。
2. **发光粒子流（streaks）**：数千个霓虹发光粒子沿环面轨道持续流动，每个粒子带彩色流光拖尾（记录最近若干帧历史位置画 additive 线段），整体像烟圈上的能量流。
3. **辐条（spokes）**：从环面径向向外辐射的细发光线（类似"圆环上长头发"）。
4. **霓虹配色**：青(#29E6FF)、品红(#FF3EA5)、紫(#B06BFF)、琥珀(#FFD25A)为主色，深色背景（近黑 #05070B）高对比。
5. **全屏 bloom**：高亮部分泛光渗开（阈值提取 → 降采样高斯模糊 → 叠加回场景）。
6. **持续动态**：环面整体自转（模拟 RPM 转速，约 10~15 圈/分钟）+ 粒子沿轨道流动 + 流光脉冲闪烁。
7. **边缘发光**：曲面/节点用菲涅尔边缘光（emissive），节点小球带外发光晕。

## 实现要求（按顺序做，每步可编译）
### Step 1 — Bloom 后处理链（先搭骨架）
- 用 `GL::Framebuffer` + `GL::Texture2D`（`GL::TextureFormat::RGBA16F`）做离屏渲染：场景先画到 FBO，再做 bloom：亮度阈值（>0.8）→ 4 级降采样 ping-pong 高斯模糊（半分辨率 1/2 → 1/4 → 1/8 → 1/16）→ 最终把原图 + bloom 图叠加到默认 framebuffer。
- 新建 `Bloom.cpp/.h` 封装（构造、resize、render(sceneTex, outTarget)），`ViewerApp` 里只挂两三个调用。
- 视口变化时 FBO/纹理尺寸跟随 framebufferSize。

### Step 2 — 发光材质与背景
- 曲面 shader：加 `emissive` 系数 + 菲涅尔边缘光 `glow = pow(1.0 - max(dot(N,V),0.0), 3.0) * glowColor`，颜色走霓虹调色板。
- 背景改近黑 + 微星点（数百个静态点精灵，极小亮度，避免抢戏）。

### Step 3 — 扭曲环面几何（新增第 4 种曲面，不改旧的三种）
- 在 `SurfaceMesh` 加 `SurfaceKind::TwistedTorus`，参数化：
  ```cpp
  // θ ∈ [0,2π) 环向, φ ∈ [0,2π) 管向, R0 主半径, r 管半径
  // lobes = 3, twist = 0.5（圈）
  float ph = phi + twist * theta;              // 截面沿环向扭转
  float R  = R0 + r * std::cos(lobes * theta); // 三瓣烟圈
  P = { (R + r*cos(ph)) * cos(theta),  r*sin(ph),  (R + r*cos(ph)) * sin(theta) };
  ```
  法线用参数导数叉积。HUD「曲面」按钮排 4 个：环面/双曲面/内环面/涡旋环。
- 注意与场景坐标系的衔接：本拓扑渲染仍以 Scene 的投影坐标为准，涡旋环作为"展示态基底"，节点/标签位置保持不变。

### Step 4 — 粒子流 + 流光拖尾（spokes + streaks）
- 粒子系统：N≈3000~6000，每个粒子有轨道参数 `(θ0, φ0, speed, phase, colorIdx)`，世界位置 = 环面表面点经 `RotY(ωt)` 旋转；`θ(t) = θ0 + speed * t`。
- 渲染为点精灵（`GL::MeshPrimitive::Points` + 实例化/动态 VBO，一帧一 DrawCall），shader 里画成径向渐变发光圆点（`smoothstep` 圆 + additive 混合）。
- 拖尾：每粒子保留最近 K=12 个历史世界位置，拼成线段缓冲（`Lines`，additive 混合，颜色=粒子色、亮度随新旧衰减——可在 shader 里按距离或单独 alpha 属性衰减）。
- 辐条：从环面表面沿径向向外拉出 1500~2500 根细线（起点=环面点，终点=起点向外 0.6~1.4 倍），低亮度、加粗线宽不可靠时用两段线或缩放 quad，additive。
- 全部粒子/拖尾/辐条数据预生成一次（θ0/φ0/phase/colorIdx），每帧只更新 `t` 与 `RotY(ωt)` 矩阵，避免每帧重建整个缓冲的 CPU 开销；若数据量大再考虑 GPU 侧计算（shader 里由 phase 推位置，不要求，先用 CPU 版本）。

### Step 5 — 展示 / 数据 双模式
- 按键 **T** 切换：
  - 展示模式（默认新增强化后的效果）：粒子流+拖尾+辐条+bloom+霓虹基底，节点可降为发光点、标签隐藏或淡出。
  - 数据模式（原有效果）：现有节点+白线+中文标签+属性面板，关闭 bloom 与粒子流（或仅保留弱 bloom）。
- 两种模式都保留：拾取（左键）、聚焦（F）、三种/四种曲面切换、位置锁定拖动、Esc、相机操作。

### Step 6 — 性能
- 节点实例化仍是一 DrawCall；粒子/拖尾/辐条合计 ≤3 个 DrawCall（Points + Lines + Lines）。
- Bloom 全流程 ≤ 8 个 draw（4 级模糊 × 2 ping-pong），RGBA16F 半分辨率即可。
- 保持 RTX4060 上 5 万实体 60fps 的目标；任何新 pass 先测再优化。

## 验收标准
启动 `omstudio_viewer` 后：
1. 默认展示模式下，环面呈霓虹三瓣烟圈涡旋，粒子流光持续流动、有拖尾，全屏 bloom 泛光明显，观感接近参考帧；
2. 按 T 切回数据模式，节点/标签/属性面板与升级前一致；
3. 左键点选节点仍能拾取并聚焦、右侧 HUD 仍显示属性与关联；
4. 环面/双曲面/内环面/涡旋环四种曲面可切换；
5. 5 万实体流畅（无卡顿、无显存暴涨）。

## 不要做
- 不要动 `omstudio_core`（Scene/Octree/Topology/OmStudio.cpp）；
- 不要引入新第三方库（不用 bgfx/Dear ImGui/后处理库）；
- 不要改中文标签方案（TextCache）；
- 不要牺牲现有拾取/聚焦/属性面板功能换取视觉效果；
- 不要做地图/真实场景渲染，保持抽象数据可视化的风格。

═══ 复制到此结束 ═══
