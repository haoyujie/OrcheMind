// 高维 → 3D 拓扑投影抽象（可插拔后端：Clifford 环面 / 庞加莱双曲球）
// 设计要点：渲染层、拾取层、剔除层只依赖 project() 输出的 3D 坐标，
// 切换构型不影响上层代码 —— 对应"构型选择未定"的两难，运行时一键切换。
#pragma once
#include "omstudio/math.h"
#include <memory>

namespace om {

class Topology {
public:
    virtual ~Topology() = default;
    virtual const char* name() const = 0;

    // 高维语素向量 → 3D 投影坐标（投影空间 = 渲染/拾取/剔除空间）
    virtual Vec3 project(const VecN& highDim) const = 0;

    // 拓扑距离（供近邻查询、关联强度排序；语义随构型变化）
    virtual float distance(const VecN& a, const VecN& b) const = 0;

    // 期望的高维维度（5~6）
    virtual int dim() const = 0;

    // 把给定高维点设为拓扑原点（选中对象聚焦居中；实现各自定义等距变换）
    virtual void setCenter(const VecN& center) = 0;

    const VecN& center() const { return center_; }

protected:
    VecN center_; // 当前拓扑中心（高维坐标）
};

// Clifford 环面：S1×S1 ⊂ S3 的立体投影（欧氏流形，平等网状关联）
std::unique_ptr<Topology> makeTorusTopology(int dim, float majorRadius = 2.2f,
                                            float tubeRadius = 1.0f,
                                            float radialSpread = 0.35f,
                                            float verticalSpread = 0.25f);

// 庞加莱双曲球：非欧几何，天然支持层级分支"向外生长"
std::unique_ptr<Topology> makeHyperbolicTopology(int dim, float curvature = 0.9f);

} // namespace om
