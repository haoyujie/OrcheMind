// 本体实体与关系的数据结构（渲染必需字段；完整业务属性保留在 C# 编辑器侧）
#pragma once
#include "omstudio/math.h"
#include <cstdint>
#include <string>
#include <vector>

namespace om {

// 一个本体对象（语素）。C++ 渲染层只持有绘制/拾取必需字段：
// 高维向量、半径、颜色、父节点；属性面板内容由 C# 侧按 id 拉取。
struct Entity {
    uint64_t id = 0;
    VecN highDim;              // 5~6 维语素向量（本体语义坐标，源自在 C# 编辑器中的定义）
    float radius = 0.08f;      // 显示半径（近大远小由 LOD 控制）
    uint32_t color = 0x88CCFF; // 0xRRGGBB
    uint64_t parentId = 0;     // 0 = 根（用于层级着色与"向外生长"）
    std::string label;

    Vec3 pos3;                 // 投影缓存（拓扑投影后，渲染/拾取/剔除用）
};

// 一条关联边（RCC8 关系种类由 C# 语义层维护，这里只存绘制所需强度）
struct Relation {
    uint64_t a = 0, b = 0;
    float strength = 1.0f;     // (0,1]，用于强度阈值过滤与粗细/透明度映射
};

// 渲染层收到的紧凑节点（AoS，直接灌 GPU 实例缓冲）
struct RenderNode {
    uint64_t id;
    Vec3 pos;
    float radius;
    uint32_t color;
    int lod;                   // 0=近(全细节) 1=中(简化) 2=远(小点)
};

struct RenderEdge {
    Vec3 pa, pb;
    float strength;
};

} // namespace om
