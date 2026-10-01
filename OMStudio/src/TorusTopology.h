// Clifford 环面拓扑：5~6 维向量 → 3D 圆环基底 + 径向/纵向扰动"长头发"
#pragma once
#include "omstudio/Topology.h"

namespace om {

class TorusTopology final : public Topology {
public:
    TorusTopology(int dim, float majorRadius, float tubeRadius,
                  float radialSpread, float verticalSpread);
    const char* name() const override { return "clifford-torus"; }
    Vec3 project(const VecN& highDim) const override;
    float distance(const VecN& a, const VecN& b) const override;
    int dim() const override { return dim_; }
    void setCenter(const VecN& center) override;

private:
    int dim_;
    float majorRadius_, tubeRadius_, radialSpread_, verticalSpread_;
    // 环面等距旋转中心偏移（把选中对象的角度对齐到 0，而非简单平移）
    float centerTheta_ = 0.0f, centerPhi_ = 0.0f;
    // 中心点在无偏移参数下的投影位置：作为整体平移基准，使选中点落在原点
    Vec3 centerOffset_{0.0f, 0.0f, 0.0f};
};

} // namespace om
