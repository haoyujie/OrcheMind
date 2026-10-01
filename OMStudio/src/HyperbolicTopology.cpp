#include "HyperbolicTopology.h"
#include <cmath>

namespace om {

namespace {
inline float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
}

HyperbolicTopology::HyperbolicTopology(int dim, float curvature)
    : dim_(dim < 3 ? 3 : dim), curvature_(curvature) {}

Vec3 HyperbolicTopology::embed(const VecN& highDim) const {
    Vec3 u{0, 0, 0};
    if (highDim.size() >= 3) u = {highDim[0], highDim[1], highDim[2]};
    float rho = u.length();
    if (rho < 1e-6f) return {};

    // 径向压缩到球内：|u| = tanh(c·rho) < 1
    u *= std::tanh(curvature_ * rho) / rho;

    // 第 4 维：层级因子，调制半径（父系层级越深越靠外）
    if (dim_ >= 4) {
        u *= 1.0f + 0.35f * clampf(highDim[3], -1.0f, 1.0f);
    }
    // 确保仍严格在球内（双曲度量要求 |u| < 1）
    float rr = u.length();
    if (rr > 0.97f) u *= 0.97f / rr;
    return u;
}

Vec3 HyperbolicTopology::project(const VecN& highDim) const {
    Vec3 u = embed(highDim);

    // 莫比乌斯平移：w = ((1+2<a,u>+|u|²)a + (1-|a|²)u) / (1+2<a,u>+|a|²|u|²)
    // 把选中对象移到球心（= 拓扑原点），视觉上整个双曲空间围绕它重新展开
    float au = mobiusA_.dot(u);
    float uu = u.lengthSq();
    float aa = mobiusA_.lengthSq();
    Vec3 num = mobiusA_ * (1.0f + 2.0f * au + uu) + u * (1.0f - aa);
    float den = 1.0f + 2.0f * au + aa * uu;
    Vec3 w = den > 1e-12f ? num / den : Vec3{};
    return w * scale_;
}

float HyperbolicTopology::distance(const VecN& a, const VecN& b) const {
    // 庞加莱距离：acosh(1 + 2|u-v|² / ((1-|u|²)(1-|v|²)))
    Vec3 u = embed(a), v = embed(b);
    float duv = (u - v).lengthSq();
    float den = (1.0f - u.lengthSq()) * (1.0f - v.lengthSq());
    if (den < 1e-12f) den = 1e-12f;
    float x = 1.0f + 2.0f * duv / den;
    return std::acosh(x > 1.0f ? x : 1.0f);
}

void HyperbolicTopology::setCenter(const VecN& center) {
    center_ = center;
    // 要把中心点 u0 移到球心：莫比乌斯平移参数 a = -u0（T_a(0)=a 的性质）
    Vec3 u0 = embed(center);
    mobiusA_ = -u0;
}

std::unique_ptr<Topology> makeHyperbolicTopology(int dim, float curvature) {
    return std::make_unique<HyperbolicTopology>(dim, curvature);
}

} // namespace om
