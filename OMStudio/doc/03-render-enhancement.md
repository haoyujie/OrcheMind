Prompt 已写入 `docs/cursor-prompt-render-upgrade.md`。下面是可直接复制的全文，以及为什么当前效果"不对味"的一句话解释。

## 差距一句话（为什么现在不像视频4）

你现在的渲染是**"数据图"**（平铺的网格+节点+标签，静态、无光效）；视频4 是**"数据流电影"**（发光粒子+流光拖尾+bloom+动态旋转）。拓扑层你已经有三种曲面了，缺的是四个渲染层：**发光材质、粒子/拖尾系统、bloom 后处理、时间动画**——所以加渲染加的是"图层"，不是改拓扑。

## 升级管线（Cursor 会按这个顺序动刀）

```
<html style="margin:0;padding:0;">
<title>OMStudio 渲染升级管线</title>
<div style="width:100%;box-sizing:border-box;background:#0b0e14;font-family:'Segoe UI','Microsoft YaHei',system-ui,sans-serif;padding:18px;color:#e8ecf3;">
<div style="max-width:880px;margin:0 auto;">
<div style="font-size:19px;font-weight:700;margin-bottom:4px;">OMStudio 渲染升级管线：数据图 → 数据流电影</div>
<div style="font-size:13px;color:#9aa7b8;margin-bottom:14px;">现有层不动，新增 5 层（Step 1~5），T 键双模式切换</div>
<svg viewBox="0 0 840 560" width="100%" xmlns="[http://www.w3.org/2000/svg](http://www.w3.org/2000/svg)">
<defs>
<marker id="a" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto" markerUnits="strokeWidth"><path d="M0,0 L7,3 L0,6 Z" fill="#4d9fff"/></marker>
</defs>
<!-- 输入层 -->
<rect x="120" y="10" width="600" height="44" rx="8" fill="#141a26" stroke="#3a4a63"/>
<text x="140" y="38" fill="#9fb4d6" font-size="13" font-weight="700">输入（不动）：Scene 投影坐标 · omstudio_core · TextCache 中文标签</text>
<line x1="420" y1="54" x2="420" y2="72" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 现有层 -->
<rect x="140" y="74" width="560" height="40" rx="8" fill="#182230" stroke="#3a4a63"/>
<text x="160" y="100" fill="#c9d6ea" font-size="13" font-weight="700">现有层（保留）：曲面网格(环面/双曲面/内环面) · 实例化节点 · 线段 · HUD/拾取/聚焦/相机</text>
<line x1="420" y1="114" x2="420" y2="132" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 新增层标题 -->
<rect x="260" y="134" width="320" height="34" rx="8" fill="#1c2f4a" stroke="#4d9fff" stroke-width="1.5"/>
<text x="420" y="156" fill="#8fd0ff" font-size="13" font-weight="700" text-anchor="middle">新增 5 层（渲染风格升级）</text>
<line x1="420" y1="168" x2="420" y2="186" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 5 个新增层 -->
<rect x="140" y="188" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="213" fill="#9fe8c4" font-size="13" font-weight="700">Step 1 · Bloom 后处理链</text>
<text x="470" y="213" fill="#7fb89a" font-size="12">RGBA16F离屏 → 阈值>0.8 → 4级降采样高斯模糊 → 叠加回屏</text>
<line x1="420" y1="228" x2="420" y2="242" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="244" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="269" fill="#9fe8c4" font-size="13" font-weight="700">Step 2 · 发光材质 + 星点背景</text>
<text x="470" y="269" fill="#7fb89a" font-size="12">emissive + 菲涅尔边缘光 glow=pow(1-N·V,3)</text>
<line x1="420" y1="284" x2="420" y2="298" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="300" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="325" fill="#9fe8c4" font-size="13" font-weight="700">Step 3 · 涡旋环 TwistedTorus（第4种曲面）</text>
<text x="470" y="325" fill="#7fb89a" font-size="12">3 lobes · twist=0.5 · 烟圈涡旋参数化</text>
<line x1="420" y1="340" x2="420" y2="354" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="356" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="381" fill="#9fe8c4" font-size="13" font-weight="700">Step 4 · 粒子流 + 流光拖尾 + 辐条</text>
<text x="470" y="381" fill="#7fb89a" font-size="12">3000+粒子沿轨道流动 · K=12历史拖尾 · 1500+径向辐条</text>
<line x1="420" y1="396" x2="420" y2="410" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="412" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="437" fill="#9fe8c4" font-size="13" font-weight="700">Step 5 · T 键双模式</text>
<text x="470" y="437" fill="#7fb89a" font-size="12">展示模式(流光+bloom) ⇄ 数据模式(节点+标签+属性面板)</text>
<line x1="420" y1="452" x2="420" y2="470" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 输出 -->
<rect x="200" y="472" width="440" height="44" rx="8" fill="#3a1f2e" stroke="#ff6aa8"/>
<text x="420" y="499" fill="#ffb3d2" font-size="13" font-weight="700" text-anchor="middle">输出：霓虹烟圈涡旋 + 粒子流光 + bloom 泛光 + 持续动态</text>
</svg>
<div style="font-size:12px;color:#6b7788;margin-top:8px;">要求：≤11 个 draw（节点1 + 粒子1 + 拖尾1 + 辐条1 + bloom 8）· 不动核心层 · 不引新库 · RTX4060 上 5 万实体 60fps</div>
</div>
</div>
</html>
```

## 直接复制给 Cursor 的 Prompt

```
## 任务
把 OMStudio 3D 查看器的渲染升级为参考视频（JEV ENGINEERING 的 TWISTED TORUS / SMOKE-RING VORTEX）那种电影感效果：霓虹发光粒子流、流光拖尾、全屏 bloom 泛光、扭曲烟圈环面、持续动态旋转。现有交互与数据编辑功能必须全部保留。

## 工程背景
- 路径：G:\myfuture\OrcheMind\OMStudio，C++17 + Magnum(GL330) + GLFW，MinGW 构建（沿用现有 build-viewer 的 CMake 配置，不要改工具链）。
- 渲染主文件：apps/viewer/ViewerApp.cpp（约590行）：曲面网格、实例化节点、线段、TextCache 中文标签、HUD、相机/拾取/聚焦/输入。
- 曲面参数化：apps/viewer/SurfaceMesh.cpp/.h（已有 Torus、Hyperboloid、InnerTorus 三种）。
- 中文标签：apps/viewer/TextCache.cpp（GDI 生成纹理，不要动）。
- omstudio_core（include/omstudio/*）本次不要修改，只改渲染。

## 当前效果（见 viewer-shot.png）
深蓝半透明曲面 + 绿/粉小球 + 白线 + 黑字标签 + 右侧 HUD。静态、平铺、无光效。

## 目标效果（见 build/ref/frame_001/003/005/008.png）
1. 扭曲环面：三瓣（3 lobes）、截面扭转（twist≈1/2 圈）、烟圈涡旋质感；
2. 发光粒子流（streaks）：数千霓虹发光粒子沿环面轨道持续流动，带彩色流光拖尾（记录最近若干帧历史位置画 additive 线段）；
3. 辐条（spokes）：从环面径向向外辐射的细发光线（"圆环上长头发"）；
4. 霓虹配色：青#29E6FF / 品红#FF3EA5 / 紫#B06BFF / 琥珀#FFD25A，深色背景近黑#05070B；
5. 全屏 bloom：亮度阈值 → 降采样高斯模糊 → 叠加回场景；
6. 持续动态：环面自转（约 10~15 圈/分）+ 粒子流动 + 流光脉冲；
7. 边缘发光：菲涅尔边缘光 + 节点外发光晕。

## 实现要求（按顺序，每步可编译）
Step 1 — Bloom 后处理链：GL::Framebuffer + GL::Texture2D(RGBA16F) 离屏渲染，阈值>0.8 → 4级降采样(1/2→1/16) ping-pong 高斯模糊 → 原图+bloom 叠加到默认 framebuffer。新建 Bloom.cpp/.h 封装，ViewerApp 只挂两三个调用。视口变化跟随 framebufferSize。
Step 2 — 发光材质：曲面 shader 加 emissive + 菲涅尔边缘光 glow=pow(1-max(dot(N,V),0),3)*glowColor，霓虹调色板。背景改近黑+微星点（数百点精灵，低亮度）。
Step 3 — 扭曲环面：SurfaceMesh 加 SurfaceKind::TwistedTorus，参数化：
    float ph = phi + twist*theta;                 // twist=0.5
    float R  = R0 + r*cos(lobes*theta);           // lobes=3
    P = {(R+r*cos(ph))*cos(theta), r*sin(ph), (R+r*cos(ph))*sin(theta)};
  法线用参数导数叉积。HUD 曲面按钮排4个：环面/双曲面/内环面/涡旋环。
Step 4 — 粒子流+拖尾+辐条：N≈3000~6000 粒子，轨道参数(θ0,φ0,speed,phase,colorIdx)，世界位置=环面表面点经 RotY(ωt)；渲染点精灵（Points+动态VBO，一帧一DrawCall，shader 径向渐变发光圆+additive）；拖尾=每粒子最近 K=12 历史位置拼 Lines（additive，亮度随新旧衰减）；辐条=从环面径向向外 1500~2500 根细线。粒子数据预生成一次，每帧只更新 t 与 RotY(ωt)。
Step 5 — T 键双模式：展示模式（默认，粒子流+拖尾+辐条+bloom+霓虹基底，节点降为发光点、标签淡出）；数据模式（原有效果，节点+白线+中文标签+属性面板，关 bloom 或留弱 bloom）。两模式都保留拾取/聚焦/曲面切换/锁定拖动/Esc/相机。
Step 6 — 性能：节点1 DrawCall；粒子/拖尾/辐条合计≤3；bloom≤8 draw；RTX4060 上 5 万实体 60fps。

## 验收标准
1. 默认展示模式：霓虹三瓣烟圈涡旋、粒子流光持续流动带拖尾、全屏 bloom，观感接近参考帧；
2. 按 T 切回数据模式：节点/标签/属性面板与升级前一致；
3. 左键拾取聚焦、HUD 属性关联正常；
4. 四种曲面可切换；
5. 5 万实体流畅，显存不暴涨。

## 不要做
- 不动 omstudio_core（Scene/Octree/Topology/OmStudio.cpp）；
- 不引新第三方库（不用 bgfx/Dear ImGui/后处理库）；
- 不改 TextCache 中文标签方案；
- 不牺牲拾取/聚焦/属性面板换取视觉效果；
- 不做地图/真实场景，保持抽象数据可视化风格。
```

要点提醒：把 `apps/viewer/viewer-shot.png` 和 `build/ref/frame_001/003/005/008.png` 一并拖进 Cursor 对话作视觉参照，Cursor 会按"先 bloom 骨架 → 发光材质 → 涡旋几何 → 粒子流 → 双模式"的顺序实现，每步可编译，不会一上来就崩。

Prompt 已写入 `docs/cursor-prompt-render-upgrade.md`。下面是可直接复制的全文，以及为什么当前效果"不对味"的一句话解释。

## 差距一句话（为什么现在不像视频4）

你现在的渲染是**"数据图"**（平铺的网格+节点+标签，静态、无光效）；视频4 是**"数据流电影"**（发光粒子+流光拖尾+bloom+动态旋转）。拓扑层你已经有三种曲面了，缺的是四个渲染层：**发光材质、粒子/拖尾系统、bloom 后处理、时间动画**——所以加渲染加的是"图层"，不是改拓扑。

## 升级管线（Cursor 会按这个顺序动刀）

```
<html style="margin:0;padding:0;">
<title>OMStudio 渲染升级管线</title>
<div style="width:100%;box-sizing:border-box;background:#0b0e14;font-family:'Segoe UI','Microsoft YaHei',system-ui,sans-serif;padding:18px;color:#e8ecf3;">
<div style="max-width:880px;margin:0 auto;">
<div style="font-size:19px;font-weight:700;margin-bottom:4px;">OMStudio 渲染升级管线：数据图 → 数据流电影</div>
<div style="font-size:13px;color:#9aa7b8;margin-bottom:14px;">现有层不动，新增 5 层（Step 1~5），T 键双模式切换</div>
<svg viewBox="0 0 840 560" width="100%" xmlns="[http://www.w3.org/2000/svg](http://www.w3.org/2000/svg)">
<defs>
<marker id="a" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto" markerUnits="strokeWidth"><path d="M0,0 L7,3 L0,6 Z" fill="#4d9fff"/></marker>
</defs>
<!-- 输入层 -->
<rect x="120" y="10" width="600" height="44" rx="8" fill="#141a26" stroke="#3a4a63"/>
<text x="140" y="38" fill="#9fb4d6" font-size="13" font-weight="700">输入（不动）：Scene 投影坐标 · omstudio_core · TextCache 中文标签</text>
<line x1="420" y1="54" x2="420" y2="72" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 现有层 -->
<rect x="140" y="74" width="560" height="40" rx="8" fill="#182230" stroke="#3a4a63"/>
<text x="160" y="100" fill="#c9d6ea" font-size="13" font-weight="700">现有层（保留）：曲面网格(环面/双曲面/内环面) · 实例化节点 · 线段 · HUD/拾取/聚焦/相机</text>
<line x1="420" y1="114" x2="420" y2="132" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 新增层标题 -->
<rect x="260" y="134" width="320" height="34" rx="8" fill="#1c2f4a" stroke="#4d9fff" stroke-width="1.5"/>
<text x="420" y="156" fill="#8fd0ff" font-size="13" font-weight="700" text-anchor="middle">新增 5 层（渲染风格升级）</text>
<line x1="420" y1="168" x2="420" y2="186" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 5 个新增层 -->
<rect x="140" y="188" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="213" fill="#9fe8c4" font-size="13" font-weight="700">Step 1 · Bloom 后处理链</text>
<text x="470" y="213" fill="#7fb89a" font-size="12">RGBA16F离屏 → 阈值>0.8 → 4级降采样高斯模糊 → 叠加回屏</text>
<line x1="420" y1="228" x2="420" y2="242" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="244" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="269" fill="#9fe8c4" font-size="13" font-weight="700">Step 2 · 发光材质 + 星点背景</text>
<text x="470" y="269" fill="#7fb89a" font-size="12">emissive + 菲涅尔边缘光 glow=pow(1-N·V,3)</text>
<line x1="420" y1="284" x2="420" y2="298" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="300" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="325" fill="#9fe8c4" font-size="13" font-weight="700">Step 3 · 涡旋环 TwistedTorus（第4种曲面）</text>
<text x="470" y="325" fill="#7fb89a" font-size="12">3 lobes · twist=0.5 · 烟圈涡旋参数化</text>
<line x1="420" y1="340" x2="420" y2="354" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="356" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="381" fill="#9fe8c4" font-size="13" font-weight="700">Step 4 · 粒子流 + 流光拖尾 + 辐条</text>
<text x="470" y="381" fill="#7fb89a" font-size="12">3000+粒子沿轨道流动 · K=12历史拖尾 · 1500+径向辐条</text>
<line x1="420" y1="396" x2="420" y2="410" stroke="#4d9fff" stroke-width="1.6" marker-end="url(#a)"/>
<rect x="140" y="412" width="560" height="40" rx="8" fill="#203a2e" stroke="#4fbf8a"/>
<text x="160" y="437" fill="#9fe8c4" font-size="13" font-weight="700">Step 5 · T 键双模式</text>
<text x="470" y="437" fill="#7fb89a" font-size="12">展示模式(流光+bloom) ⇄ 数据模式(节点+标签+属性面板)</text>
<line x1="420" y1="452" x2="420" y2="470" stroke="#4d9fff" stroke-width="2" marker-end="url(#a)"/>
<!-- 输出 -->
<rect x="200" y="472" width="440" height="44" rx="8" fill="#3a1f2e" stroke="#ff6aa8"/>
<text x="420" y="499" fill="#ffb3d2" font-size="13" font-weight="700" text-anchor="middle">输出：霓虹烟圈涡旋 + 粒子流光 + bloom 泛光 + 持续动态</text>
</svg>
<div style="font-size:12px;color:#6b7788;margin-top:8px;">要求：≤11 个 draw（节点1 + 粒子1 + 拖尾1 + 辐条1 + bloom 8）· 不动核心层 · 不引新库 · RTX4060 上 5 万实体 60fps</div>
</div>
</div>
</html>
```

## 直接复制给 Cursor 的 Prompt

```
## 任务
把 OMStudio 3D 查看器的渲染升级为参考视频（JEV ENGINEERING 的 TWISTED TORUS / SMOKE-RING VORTEX）那种电影感效果：霓虹发光粒子流、流光拖尾、全屏 bloom 泛光、扭曲烟圈环面、持续动态旋转。现有交互与数据编辑功能必须全部保留。

## 工程背景
- 路径：G:\myfuture\OrcheMind\OMStudio，C++17 + Magnum(GL330) + GLFW，MinGW 构建（沿用现有 build-viewer 的 CMake 配置，不要改工具链）。
- 渲染主文件：apps/viewer/ViewerApp.cpp（约590行）：曲面网格、实例化节点、线段、TextCache 中文标签、HUD、相机/拾取/聚焦/输入。
- 曲面参数化：apps/viewer/SurfaceMesh.cpp/.h（已有 Torus、Hyperboloid、InnerTorus 三种）。
- 中文标签：apps/viewer/TextCache.cpp（GDI 生成纹理，不要动）。
- omstudio_core（include/omstudio/*）本次不要修改，只改渲染。

## 当前效果（见 viewer-shot.png）
深蓝半透明曲面 + 绿/粉小球 + 白线 + 黑字标签 + 右侧 HUD。静态、平铺、无光效。

## 目标效果（见 build/ref/frame_001/003/005/008.png）
1. 扭曲环面：三瓣（3 lobes）、截面扭转（twist≈1/2 圈）、烟圈涡旋质感；
2. 发光粒子流（streaks）：数千霓虹发光粒子沿环面轨道持续流动，带彩色流光拖尾（记录最近若干帧历史位置画 additive 线段）；
3. 辐条（spokes）：从环面径向向外辐射的细发光线（"圆环上长头发"）；
4. 霓虹配色：青#29E6FF / 品红#FF3EA5 / 紫#B06BFF / 琥珀#FFD25A，深色背景近黑#05070B；
5. 全屏 bloom：亮度阈值 → 降采样高斯模糊 → 叠加回场景；
6. 持续动态：环面自转（约 10~15 圈/分）+ 粒子流动 + 流光脉冲；
7. 边缘发光：菲涅尔边缘光 + 节点外发光晕。

## 实现要求（按顺序，每步可编译）
Step 1 — Bloom 后处理链：GL::Framebuffer + GL::Texture2D(RGBA16F) 离屏渲染，阈值>0.8 → 4级降采样(1/2→1/16) ping-pong 高斯模糊 → 原图+bloom 叠加到默认 framebuffer。新建 Bloom.cpp/.h 封装，ViewerApp 只挂两三个调用。视口变化跟随 framebufferSize。
Step 2 — 发光材质：曲面 shader 加 emissive + 菲涅尔边缘光 glow=pow(1-max(dot(N,V),0),3)*glowColor，霓虹调色板。背景改近黑+微星点（数百点精灵，低亮度）。
Step 3 — 扭曲环面：SurfaceMesh 加 SurfaceKind::TwistedTorus，参数化：
    float ph = phi + twist*theta;                 // twist=0.5
    float R  = R0 + r*cos(lobes*theta);           // lobes=3
    P = {(R+r*cos(ph))*cos(theta), r*sin(ph), (R+r*cos(ph))*sin(theta)};
  法线用参数导数叉积。HUD 曲面按钮排4个：环面/双曲面/内环面/涡旋环。
Step 4 — 粒子流+拖尾+辐条：N≈3000~6000 粒子，轨道参数(θ0,φ0,speed,phase,colorIdx)，世界位置=环面表面点经 RotY(ωt)；渲染点精灵（Points+动态VBO，一帧一DrawCall，shader 径向渐变发光圆+additive）；拖尾=每粒子最近 K=12 历史位置拼 Lines（additive，亮度随新旧衰减）；辐条=从环面径向向外 1500~2500 根细线。粒子数据预生成一次，每帧只更新 t 与 RotY(ωt)。
Step 5 — T 键双模式：展示模式（默认，粒子流+拖尾+辐条+bloom+霓虹基底，节点降为发光点、标签淡出）；数据模式（原有效果，节点+白线+中文标签+属性面板，关 bloom 或留弱 bloom）。两模式都保留拾取/聚焦/曲面切换/锁定拖动/Esc/相机。
Step 6 — 性能：节点1 DrawCall；粒子/拖尾/辐条合计≤3；bloom≤8 draw；RTX4060 上 5 万实体 60fps。

## 验收标准
1. 默认展示模式：霓虹三瓣烟圈涡旋、粒子流光持续流动带拖尾、全屏 bloom，观感接近参考帧；
2. 按 T 切回数据模式：节点/标签/属性面板与升级前一致；
3. 左键拾取聚焦、HUD 属性关联正常；
4. 四种曲面可切换；
5. 5 万实体流畅，显存不暴涨。

## 不要做
- 不动 omstudio_core（Scene/Octree/Topology/OmStudio.cpp）；
- 不引新第三方库（不用 bgfx/Dear ImGui/后处理库）；
- 不改 TextCache 中文标签方案；
- 不牺牲拾取/聚焦/属性面板换取视觉效果；
- 不做地图/真实场景，保持抽象数据可视化风格。
```

要点提醒：把 `apps/viewer/viewer-shot.png` 和 `build/ref/frame_001/003/005/008.png` 一并拖进 Cursor 对话作视觉参照，Cursor 会按"先 bloom 骨架 → 发光材质 → 涡旋几何 → 粒子流 → 双模式"的顺序实现，每步可编译，不会一上来就崩。

**总结**：核心结论——你当前效果和视频4的差距**不在拓扑，在渲染风格层**。视频4（JEV Twisted Torus）的本质是四件套：**发光材质 + 粒子流光拖尾 + bloom 泛光 + 持续动态**，你现有的是"网格+节点+标签"的静态数据图。

已交付：

1. **`docs/cursor-prompt-render-upgrade.md`**——可直接复制的完整 Cursor Prompt（上面回复里也全文贴出），含工程路径、目标效果（7 条）、按可编译顺序的 6 个实现步骤（bloom → 发光材质 → 涡旋环 → 粒子流 → 双模式 → 性能）、验收标准 5 条、禁区 5 条；
2. **4 张参考视频帧**（build/ref/）——拖进 Cursor 对话当视觉锚点；
3. **升级管线示意图**（上面 renderer 块）——一眼看清 Cursor 会往哪 5 层动刀。

**给 Cursor 时最关键的两个动作**：把 `viewer-shot.png`（当前）和 `frame_001/003/005/008.png`（目标）一起拖进去，让它对着改；Prompt 里已要求"每步可编译 + 不动核心层 + 双模式 T 键"，这样即使 Cursor 中途跑偏，你切回数据模式就能继续编辑本体，不丢功能。

跑完回来如果效果还是不够"电影感"，把新截图发我，我按图给你第二轮微调 Prompt（一般是 bloom 强度、粒子密度、配色三处的参数问题）。