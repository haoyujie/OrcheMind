// 涡旋环参数。瓣振幅和管半径分开，避免原文公式自交。
#pragma once
#include <Magnum/Math/Vector3.h>
#include <cmath>

struct VortexShape {
    static constexpr float kR0 = 2.05f;
    static constexpr float kTube = 0.58f;
    static constexpr float kLobeAmp = 0.30f;
    static constexpr float kLobes = 3.0f;
    static constexpr float kTwist = 0.5f; // θ 走一圈，截面转半圈
};

inline float vortexRadius(float theta) {
    return VortexShape::kR0 + VortexShape::kLobeAmp * std::cos(VortexShape::kLobes * theta);
}

inline Magnum::Vector3 vortexPosition(float theta, float phi) {
    const float ph = phi + VortexShape::kTwist * theta;
    const float R = vortexRadius(theta);
    const float q = R + VortexShape::kTube * std::cos(ph);
    return {q * std::cos(theta), VortexShape::kTube * std::sin(ph), q * std::sin(theta)};
}

inline Magnum::Vector3 vortexNormal(float theta, float phi) {
    const float twist = VortexShape::kTwist;
    const float tube = VortexShape::kTube;
    const float lobes = VortexShape::kLobes;
    const float amp = VortexShape::kLobeAmp;
    const float ph = phi + twist * theta;
    const float R = vortexRadius(theta);
    const float q = R + tube * std::cos(ph);
    const float dR = -amp * lobes * std::sin(lobes * theta);
    const float dQdTh = dR - tube * std::sin(ph) * twist;
    const float dQdPh = -tube * std::sin(ph);
    const float ct = std::cos(theta), st = std::sin(theta);
    Magnum::Vector3 dTh{dQdTh * ct - q * st, tube * std::cos(ph) * twist, dQdTh * st + q * ct};
    Magnum::Vector3 dPh{dQdPh * ct, tube * std::cos(ph), dQdPh * st};
    Magnum::Vector3 n = Magnum::Math::cross(dTh, dPh);
    const float len = n.length();
    if (len < 1e-6f) return {ct, 0.0f, st};
    n = n / len;
    Magnum::Vector3 axis{R * ct, 0.0f, R * st};
    if (Magnum::Math::dot(n, vortexPosition(theta, phi) - axis) < 0.0f) n = -n;
    return n;
}
