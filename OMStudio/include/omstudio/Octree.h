// 简易八叉树：投影 3D 空间的空间索引
// 用途：① 视锥剔除候选过滤（不遍历全部实体）② 射线拾取候选过滤（不逐个求交）
// 大数据量优化方向：rebuild 可并行（分块构建后合并）、增量 insert/remove。
#pragma once
#include "omstudio/Entity.h"
#include "omstudio/math.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace om {

class Octree {
public:
    Octree(const Vec3& min, const Vec3& max, int maxDepth = 8, int maxLeaf = 32);
    ~Octree();
    Octree(const Octree&) = delete;
    Octree& operator=(const Octree&) = delete;

    void insert(uint64_t id, const Vec3& pos);
    void remove(uint64_t id);
    // 全量重建（拓扑重投影/中心切换后调用）
    void rebuild(const std::vector<Entity>& entities);
    void clear();

    void queryFrustum(const ViewFrustum& f, std::vector<uint64_t>& out) const;
    void queryRay(const Ray& ray, std::vector<uint64_t>& out) const;

    size_t nodeCount() const { return nodeCount_; }

private:
    struct Node {
        Vec3 min, max;
        Node* child[8] = {};
        std::vector<uint64_t> ids;
        bool leaf = true;
    };

    std::unordered_map<uint64_t, Vec3> posMap_; // id→pos，分裂/删除时按位置分派
    Vec3 min_, max_;
    int maxDepth_, maxLeaf_;
    Node* root_ = nullptr;
    size_t nodeCount_ = 0;

    Node* newNode(const Vec3& lo, const Vec3& hi);
    void subdivide(Node* n);
    void insertRec(Node* n, uint64_t id, const Vec3& pos, int depth);
    bool removeRec(Node* n, uint64_t id, const Vec3& pos);
    void destroy(Node* n);

    static int childIndex(const Node* n, const Vec3& p);
    static bool rayAabb(const Ray& r, const Vec3& lo, const Vec3& hi, float& tNear);
    static bool aabbOutsideFrustum(const Node* n, const ViewFrustum& f);

    void queryFrustumRec(const Node* n, const ViewFrustum& f, std::vector<uint64_t>& out) const;
    void queryRayRec(const Node* n, const Ray& r, std::vector<uint64_t>& out) const;
};

} // namespace om
