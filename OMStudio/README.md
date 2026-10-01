# OMStudio — 高维本体关联图 3D 查看器（C++ 本地渲染）

为 OrcheMind 本体系统提供的 C++ 本地渲染引擎 + C# 互操作层。
把 5~6 维语素向量投影到 3D 拓扑流形（**Clifford 环面** / **庞加莱双曲球**，运行时一键切换），
支持海量节点视锥剔除、距离 LOD、射线拾取、选中聚焦、基于选中对象"生长"新对象。
全部运算本地，无服务器依赖。

```
include/omstudio/        核心公共头（math / Entity / Topology / Scene / Octree / OmStudio C API）
src/                     核心实现（TorusTopology / HyperbolicTopology / Scene / Octree / OmStudio.cpp）
apps/viewer/             Magnum 3D 查看器（ViewerApp + demo 数据 + main）
interop/OmStudioNative.cs  C# P/Invoke 封装示例（直接加入现有 C# 编辑器工程）
tests/core_test.cpp      核心自测（无第三方依赖）
docs/architecture.html   模块架构与数据流图
```

## 三个难点的代码落点

| 难点 | 实现位置 | 说明 |
|---|---|---|
| ① 构型选择（环面 vs 双曲） | `Topology.h` 抽象 + `TorusTopology.cpp` / `HyperbolicTopology.cpp` | 运行时 `Scene::setTopology()` 切换，渲染/拾取/剔除层零改动 |
| ② 海量对象的内存与渲染 | `Scene::collectVisible()` + `Octree.cpp` + 查看器实例化渲染 | 视锥剔除（八叉树）→ 距离 LOD → 边强度阈值过滤；节点一 DrawCall 实例化；边批量线段 |
| ③ 交互（查找/选中/聚焦/CRUD） | `Scene::select/pick/centerPos` + `ViewerApp` 输入 + C API | 八叉树射线候选过滤 → 精确球相交；选中即拓扑中心重投影；C# 侧只做业务属性 |

## 构建

### 1) 仅核心库 + 自测（无第三方依赖，MinGW g++ 直接编译）

```powershell
cd G:\myfuture\OrcheMind\OMStudio
g++ -std=c++17 -O2 -I include src/Scene.cpp src/Octree.cpp src/TorusTopology.cpp src/HyperbolicTopology.cpp tests/core_test.cpp -o build/core_test.exe
.\build\core_test.exe
```

### 2) C API 动态库（omstudio.dll，供 C# 编辑器 P/Invoke）

```powershell
g++ -std=c++17 -O2 -shared -fPIC -I include src/OmStudio.cpp src/Scene.cpp src/Octree.cpp src/TorusTopology.cpp src/HyperbolicTopology.cpp -o build\omstudio.dll
```
（MSVC 下用 CMake 的 `OMSTUDIO_BUILD_SHARED=ON` 目标 `omstudio`。）

### 3) Magnum 3D 查看器（需要 MSVC + vcpkg；MinGW 暂不保证）

```powershell
git clone https://github.com/microsoft/vcpkg
cd vcpkg && .\bootstrap-vcpkg.bat
.\vcpkg install magnum[gl,glfwapplication] --triplet x64-windows
cd G:\myfuture\OrcheMind\OMStudio
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<vcpkg>\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
.\build\Release\omstudio_viewer.exe 5000
```
> 说明：vcpkg 的 `magnum` 端口主要面向 MSVC（x64-windows triplet）。本机当前只有 MinGW g++，
> 核心库与 DLL 已用 g++ 验证；查看器建议安装 VS Build Tools 后构建。
> 若用 SDL2 后端，把 vcpkg.json 与 CMake 中的 `glfwapplication` 换成 `sdl2application`。

## 运行操作

| 按键/鼠标 | 动作 |
|---|---|
| 左键点击 | 拾取并选中；以该对象为拓扑中心聚焦 |
| 左键拖拽 | 旋转轨道相机 |
| 右键拖拽 | 平移目标 |
| 滚轮 | 缩放 |
| Tab | 切换构型：Clifford 环面 ↔ 庞加莱双曲球 |
| F | 聚焦选中对象 |
| Esc | 取消选中，回全局视图 |
| G | 演示：在选中对象下"生长"新对象并聚焦 |
| C | 控制台打印选中对象详情 |

## 与现有 C# 编辑器集成

1. 构建 `omstudio.dll`，放在编辑器输出目录。
2. 把 `interop/OmStudioNative.cs` 加入工程（命名空间 `OrcheMind.OMStudio`）。
3. 本体数据仍以 C# 为唯一数据源；C++ 只缓存渲染必需字段：
   - 编辑器里新增/修改/删除本体 → 调 `AddEntity / UpdateEntity / RemoveEntity` 增量同步；
   - 鼠标点击 → `Pick(ndcX, ndcY)`（内部八叉树加速，O(logN) 级）；
   - 双击/选中 → `SelectAndFocus(id)`，C++ 自动执行拓扑中心重投影；
   - 属性面板 → C# 自己查本体库展示，无需经过 C++；
   - "生长新对象" → `GrowChild(parentId, ...)` 后再把新对象落入本体库。

## 性能路线（十万级目标）

- 节点：已实例化（一 DrawCall）；远区 LOD 降到点精灵可再省一半。
- 边：强度阈值 + 距离剔除已内置；`O(N²)` 的边是唯一真正瓶颈，生产用边表索引（按强度 TopK）替代全量关系数组。
- 中心切换全量重投影：演示为同步；大数据量在 C++ 侧起后台线程，渲染主线程只读 `pos3` 快照。
