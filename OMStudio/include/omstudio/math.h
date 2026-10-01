// OMStudio 核心数学：极简自包含向量/矩阵/射线/视锥（无第三方依赖）
#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <array>

namespace om {

// ---------------- Vec3 ----------------
struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    float lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt(dot(*this)); }
    Vec3 normalized() const {
        float l = length();
        return l > 1e-12f ? (*this) * (1.0f / l) : Vec3{1.0f, 0.0f, 0.0f};
    }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
};
inline float dot(const Vec3& a, const Vec3& b) { return a.dot(b); }
inline Vec3 normalize(const Vec3& a) { return a.normalized(); }

// ---------------- VecN（动态维度，5~6维语素向量） ----------------
using VecN = std::vector<float>;

inline float norm(const VecN& v) {
    float s = 0;
    for (float x : v) s += x * x;
    return std::sqrt(s);
}
inline void normalizeInPlace(VecN& v) {
    float n = norm(v);
    if (n > 1e-12f) {
        for (auto& x : v) x /= n;
    } else if (!v.empty()) {
        v[0] = 1.0f;
    }
}

// ---------------- 射线 / 平面 / 视锥 ----------------
struct Ray {
    Vec3 origin;
    Vec3 dir; // 单位方向
};

struct Plane {
    Vec3 n;  // 单位法线
    float d; // 平面方程 n·x + d = 0
    float signedDist(const Vec3& p) const { return n.dot(p) + d; }
    // 内部约定：signedDist >= 0 为内侧
    bool contains(const Vec3& p, float radius) const { return signedDist(p) >= -radius; }
};

struct ViewFrustum {
    Plane planes[6]; // 0左 1右 2下 3上 4近 5远
    bool contains(const Vec3& p, float radius) const {
        for (const auto& pl : planes)
            if (!pl.contains(p, radius)) return false;
        return true;
    }
};

// ---------------- Mat4（列主序，OpenGL 裁剪空间，Z 范围 [-1,1]） ----------------
struct Mat4 {
    float v[16]; // v[col*4+row]

    static Mat4 identity() {
        Mat4 m{};
        for (int i = 0; i < 4; ++i) m.v[i * 4 + i] = 1.0f;
        return m;
    }
    static Mat4 perspective(float fovYDeg, float aspect, float zn, float zf) {
        Mat4 m{};
        float f = 1.0f / std::tan(fovYDeg * 3.14159265358979f / 360.0f);
        m.v[0] = f / aspect;
        m.v[5] = f;
        m.v[10] = (zf + zn) / (zn - zf);
        m.v[11] = -1.0f;
        m.v[14] = 2.0f * zf * zn / (zn - zf);
        return m;
    }
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalized();
        Vec3 s = f.cross(up).normalized();
        Vec3 u = s.cross(f);
        Mat4 m = identity();
        m.v[0] = s.x;  m.v[4] = s.y;  m.v[8]  = s.z;
        m.v[1] = u.x;  m.v[5] = u.y;  m.v[9]  = u.z;
        m.v[2] = -f.x; m.v[6] = -f.y; m.v[10] = -f.z;
        m.v[12] = -s.dot(eye);
        m.v[13] = -u.dot(eye);
        m.v[14] = f.dot(eye);
        return m;
    }
    Mat4 operator*(const Mat4& o) const {
        Mat4 r{};
        for (int c = 0; c < 4; ++c)
            for (int row = 0; row < 4; ++row) {
                float s = 0;
                for (int k = 0; k < 4; ++k) s += v[k * 4 + row] * o.v[c * 4 + k];
                r.v[c * 4 + row] = s;
            }
        return r;
    }
    // 变换齐次点（自动除以 w）
    Vec3 transformPoint(const Vec3& p) const {
        float w = v[3] * p.x + v[7] * p.y + v[11] * p.z + v[15];
        if (std::abs(w) < 1e-12f) w = 1e-12f;
        return {(v[0] * p.x + v[4] * p.y + v[8] * p.z + v[12]) / w,
                (v[1] * p.x + v[5] * p.y + v[9] * p.z + v[13]) / w,
                (v[2] * p.x + v[6] * p.y + v[10] * p.z + v[14]) / w};
    }
    // 伴随矩阵法求逆（通用 4x4）
    Mat4 inverted() const {
        const float* m = v;
        float inv[16];
        inv[0]  = m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
        inv[4]  = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
        inv[8]  = m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
        inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
        inv[1]  = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
        inv[5]  = m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
        inv[9]  = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
        inv[13] = m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
        inv[2]  = m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
        inv[6]  = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
        inv[10] = m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
        inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
        inv[3]  = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
        inv[7]  = m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
        inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11] - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
        inv[15] = m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10] + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];
        float det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
        if (std::abs(det) < 1e-12f) return identity();
        float invd = 1.0f / det;
        Mat4 r{};
        for (int i = 0; i < 16; ++i) r.v[i] = inv[i] * invd;
        return r;
    }
    // 由视锥矩阵（viewProj）提取 6 平面（Gribb–Hartmann），内部 signedDist>=0
    static ViewFrustum extractFrustum(const Mat4& vp) {
        std::array<std::array<float, 4>, 4> row{};
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                row[r][c] = vp.v[c * 4 + r];
        ViewFrustum f;
        const int comb[6][3] = { // {a, b, sign}: plane = row[a] + sign*row[b]
            {3, 0, 1}, {3, 0, -1}, {3, 1, 1}, {3, 1, -1}, {3, 2, 1}, {3, 2, -1}};
        for (int i = 0; i < 6; ++i) {
            const auto& ra = row[comb[i][0]];
            const auto& rb = row[comb[i][1]];
            float s = (float)comb[i][2];
            f.planes[i].n = {ra[0] + s * rb[0], ra[1] + s * rb[1], ra[2] + s * rb[2]};
            f.planes[i].d = ra[3] + s * rb[3];
        }
        return f;
    }
    // NDC 坐标（x,y ∈ [-1,1]，y 向上）反投影成一条 3D 射线
    static Ray unproject(const Mat4& invViewProj, float ndcX, float ndcY) {
        Vec3 near = invViewProj.transformPoint({ndcX, ndcY, -1.0f});
        Vec3 far  = invViewProj.transformPoint({ndcX, ndcY, 1.0f});
        Vec3 d = far - near;
        return {near, d.normalized()};
    }
};

// 0xRRGGBB → 三个 float 分量 [0,1]
inline void rgbToFloats(uint32_t rgb, float& r, float& g, float& b) {
    r = ((rgb >> 16) & 0xFF) / 255.0f;
    g = ((rgb >> 8) & 0xFF) / 255.0f;
    b = (rgb & 0xFF) / 255.0f;
}

} // namespace om
