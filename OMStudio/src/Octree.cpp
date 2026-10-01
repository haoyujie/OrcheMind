#include "omstudio/Octree.h"
#include <algorithm>
#include <cmath>

namespace om {

namespace {
inline Vec3 cornerPos(const Vec3& lo, const Vec3& hi, int i) {
    return {(i & 1) ? hi.x : lo.x, (i & 2) ? hi.y : lo.y, (i & 4) ? hi.z : lo.z};
}
}

Octree::Octree(const Vec3& min, const Vec3& max, int maxDepth, int maxLeaf)
    : min_(min), max_(max), maxDepth_(maxDepth), maxLeaf_(maxLeaf) {
    root_ = newNode(min_, max_);
}

Octree::~Octree() { destroy(root_); }

Octree::Node* Octree::newNode(const Vec3& lo, const Vec3& hi) {
    auto* n = new Node;
    n->min = lo;
    n->max = hi;
    n->leaf = true;
    ++nodeCount_;
    return n;
}

void Octree::destroy(Node* n) {
    if (!n) return;
    for (auto* c : n->child) destroy(c);
    delete n;
}

void Octree::clear() {
    destroy(root_);
    nodeCount_ = 0;
    posMap_.clear();
    root_ = newNode(min_, max_);
}

int Octree::childIndex(const Node* n, const Vec3& p) {
    const Vec3 mid = (n->min + n->max) * 0.5f;
    int idx = 0;
    if (p.x >= mid.x) idx |= 1;
    if (p.y >= mid.y) idx |= 2;
    if (p.z >= mid.z) idx |= 4;
    return idx;
}

void Octree::subdivide(Node* n) {
    const Vec3 mid = (n->min + n->max) * 0.5f;
    n->leaf = false;
    for (int i = 0; i < 8; ++i) {
        Vec3 lo = n->min, hi = mid;
        if (i & 1) lo.x = mid.x; else hi.x = mid.x;
        if (i & 2) lo.y = mid.y; else hi.y = mid.y;
        if (i & 4) lo.z = mid.z; else hi.z = mid.z;
        n->child[i] = newNode(lo, hi);
    }
    // 旧 id 按位置分派到子节点
    for (uint64_t id : n->ids) {
        auto it = posMap_.find(id);
        if (it != posMap_.end())
            n->child[childIndex(n, it->second)]->ids.push_back(id);
        else
            n->ids.push_back(id); // 位置未知，保守留在父叶子
    }
    n->ids.clear();
}

void Octree::insertRec(Node* n, uint64_t id, const Vec3& pos, int depth) {
    if (n->leaf) {
        if ((int)n->ids.size() < maxLeaf_ || depth >= maxDepth_) {
            n->ids.push_back(id);
            return;
        }
        subdivide(n);
    }
    int c = childIndex(n, pos);
    insertRec(n->child[c], id, pos, depth + 1);
}

void Octree::insert(uint64_t id, const Vec3& pos) {
    posMap_[id] = pos;
    insertRec(root_, id, pos, 0);
}

void Octree::rebuild(const std::vector<Entity>& entities) {
    clear();
    for (const auto& e : entities) insert(e.id, e.pos3);
}

bool Octree::removeRec(Node* n, uint64_t id, const Vec3& pos) {
    if (!n) return false;
    if (n->leaf) {
        auto it = std::find(n->ids.begin(), n->ids.end(), id);
        if (it != n->ids.end()) {
            n->ids.erase(it);
            return true;
        }
        return false;
    }
    int c = childIndex(n, pos);
    return removeRec(n->child[c], id, pos);
}

void Octree::remove(uint64_t id) {
    auto it = posMap_.find(id);
    if (it == posMap_.end()) return;
    removeRec(root_, id, it->second);
    posMap_.erase(it);
}

bool Octree::rayAabb(const Ray& r, const Vec3& lo, const Vec3& hi, float& tNear) {
    float t0 = 0.0f, t1 = 1e30f;
    const float o[3] = {r.origin.x, r.origin.y, r.origin.z};
    const float d[3] = {r.dir.x, r.dir.y, r.dir.z};
    const float mn[3] = {lo.x, lo.y, lo.z};
    const float mx[3] = {hi.x, hi.y, hi.z};
    for (int i = 0; i < 3; ++i) {
        if (std::abs(d[i]) < 1e-9f) {
            if (o[i] < mn[i] || o[i] > mx[i]) return false;
        } else {
            float inv = 1.0f / d[i];
            float tA = (mn[i] - o[i]) * inv, tB = (mx[i] - o[i]) * inv;
            if (tA > tB) std::swap(tA, tB);
            t0 = std::max(t0, tA);
            t1 = std::min(t1, tB);
            if (t0 > t1) return false;
        }
    }
    tNear = t0;
    return true;
}

bool Octree::aabbOutsideFrustum(const Node* n, const ViewFrustum& f) {
    // 8 顶点全部在某个平面外侧 → 剔除（保守，宁多不漏）
    for (const auto& pl : f.planes) {
        bool allOutside = true;
        for (int i = 0; i < 8; ++i) {
            Vec3 c = cornerPos(n->min, n->max, i);
            if (pl.signedDist(c) >= 0.0f) { allOutside = false; break; }
        }
        if (allOutside) return true;
    }
    return false;
}

void Octree::queryFrustumRec(const Node* n, const ViewFrustum& f, std::vector<uint64_t>& out) const {
    if (!n) return;
    if (aabbOutsideFrustum(n, f)) return;
    if (n->leaf) {
        out.insert(out.end(), n->ids.begin(), n->ids.end());
        return;
    }
    for (auto* c : n->child) queryFrustumRec(c, f, out);
}

void Octree::queryFrustum(const ViewFrustum& f, std::vector<uint64_t>& out) const {
    queryFrustumRec(root_, f, out);
}

void Octree::queryRayRec(const Node* n, const Ray& r, std::vector<uint64_t>& out) const {
    if (!n) return;
    float t = 0;
    if (!rayAabb(r, n->min, n->max, t)) return;
    if (n->leaf) {
        out.insert(out.end(), n->ids.begin(), n->ids.end());
        return;
    }
    for (auto* c : n->child) queryRayRec(c, r, out);
}

void Octree::queryRay(const Ray& ray, std::vector<uint64_t>& out) const {
    queryRayRec(root_, ray, out);
}

} // namespace om
