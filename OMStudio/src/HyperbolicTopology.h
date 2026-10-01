// 庞加莱双曲球拓扑：非欧几何，层级分支自然"向外生长"
#pragma once
#include "omstudio/Topology.h"

namespace om {

class HyperbolicTopology final : public Topology {
public:
    explicit HyperbolicTopology(int dim, float curvature);
    const char* name() const override { return "poincare-ball"; }
    Vec3 project(const VecN& highDim) const override;
    float distance(const VecN& a, const VecN& b) const override;
    int dim() const override { return dim_; }
    void setCenter(const VecN& center) override;

private:
    int dim_;
    float curvature_; // 曲率 c，越大球面收缩越快（空间容量越大）
    float scale_ = 4.0f; // 投影到视觉尺度
    Vec3 mobiusA_;       // 莫比乌斯平移参数：把中心点移到原点

    // 高维向量 → 球内嵌入（范数 < 1）
    Vec3 embed(const VecN& highDim) const;
};

} // namespace om
