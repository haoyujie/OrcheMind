#include "TorusTopology.h"
#include <cmath>

namespace om {

TorusTopology::TorusTopology(int dim, float majorRadius, float tubeRadius,
                             float radialSpread, float verticalSpread)
    : dim_(dim < 4 ? 4 : dim), majorRadius_(majorRadius), tubeRadius_(tubeRadius),
      radialSpread_(radialSpread), verticalSpread_(verticalSpread) {}

Vec3 TorusTopology::project(const VecN& highDim) const {
    // 1) 归一化高维向量（语素向量已是单位向量时无副作用）
    VecN v = highDim;
    if ((int)v.size() < 4) v.resize(4, 0.0f);
    if ((int)v.size() < dim_) v.resize(dim_, 0.0f);
    normalizeInPlace(v);

    // 2) 前 4 维 → Clifford 环面 S1×S1 ⊂ S3 的两个角度
    float th = std::atan2(v[1], v[0]);
    float ph = std::atan2(v[3], v[2]);

    // 3) 减去中心角偏移：这是环面上的等距旋转，保持距离结构
    th -= centerTheta_;
    ph -= centerPhi_;

    // 4) 参数化环面（= Clifford 环面立体投影的直观形式）：
    //    主圆半径 majorRadius_，管半径 tubeRadius_
    float cth = std::cos(th), sth = std::sin(th);
    float cph = std::cos(ph), sph = std::sin(ph);
    Vec3 p{
        (majorRadius_ + tubeRadius_ * cph) * cth,
        tubeRadius_ * sph,
        (majorRadius_ + tubeRadius_ * cph) * sth,
    };

    // 5) 第 5 维：沿径向向外"生长"（发丝方向之一）
    if (dim_ >= 5) {
        float v4 = v[4];
        p += p.normalized() * (v4 * radialSpread_);
    }
    // 6) 第 6 维：沿环面法向（Y 轴）扰动
    if (dim_ >= 6) {
        p.y += v[5] * verticalSpread_;
    }
    // 7) 整体平移：把拓扑中心点移到原点（选中对象聚焦）
    return p - centerOffset_;
}

float TorusTopology::distance(const VecN& a, const VecN& b) const {
    // 欧氏高维距离（若需要环面精确距离，应对每个角度取周期最小差；
    // 对关联强度排序而言欧氏距离已足够稳定）
    float s = 0;
    size_t n = a.size() < b.size() ? a.size() : b.size();
    for (size_t i = 0; i < n; ++i) {
        float d = a[i] - b[i];
        s += d * d;
    }
    return std::sqrt(s);
}

void TorusTopology::setCenter(const VecN& center) {
    center_ = center;
    VecN v = center;
    if ((int)v.size() < 4) v.resize(4, 0.0f);
    if ((int)v.size() < dim_) v.resize(dim_, 0.0f);
    normalizeInPlace(v);

    centerTheta_ = std::atan2(v[1], v[0]);
    centerPhi_ = std::atan2(v[3], v[2]);

    // 计算中心点"角度对齐到(0,0)后"在环面上的位置，作为整体平移基准：
    // 偏移后中心的角度为 (0,0)，对应环面最外点 (R+r, 0, 0)
    Vec3 p0{majorRadius_ + tubeRadius_, 0.0f, 0.0f};
    if (dim_ >= 5) p0 += p0.normalized() * (v[4] * radialSpread_);
    if (dim_ >= 6) p0.y += v[5] * verticalSpread_;
    centerOffset_ = p0;
}

std::unique_ptr<Topology> makeTorusTopology(int dim, float majorRadius,
                                            float tubeRadius, float radialSpread,
                                            float verticalSpread) {
    return std::make_unique<TorusTopology>(dim, majorRadius, tubeRadius,
                                           radialSpread, verticalSpread);
}

} // namespace om
