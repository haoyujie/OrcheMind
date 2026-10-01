// 场景：实体 CRUD、关联、拓扑切换、选中聚焦、可见性收集、拾取
// 内存优化核心：实体 AoS 紧凑存储 + id→下标哈希；完整属性留在 C# 侧，按 id 拉取。
#pragma once
#include "omstudio/Entity.h"
#include "omstudio/Octree.h"
#include "omstudio/Topology.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace om {

class Scene {
public:
    explicit Scene(std::unique_ptr<Topology> topology);
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // ---- 本体 CRUD ----
    uint64_t addEntity(const Entity& e);   // 返回分配的 id
    bool removeEntity(uint64_t id);        // 同时清理其关系与空间索引
    const Entity* find(uint64_t id) const;
    Entity* find(uint64_t id);
    void updateEntity(uint64_t id, const VecN& highDim, float radius, uint32_t color);
    std::vector<uint64_t> childrenOf(uint64_t parentId) const;

    // ---- 关联 ----
    void setRelation(uint64_t a, uint64_t b, float strength, const std::string& verb = {});
    bool removeRelation(uint64_t a, uint64_t b);
    const std::vector<Relation>& relations() const { return relations_; }

    // ---- 拓扑 ----
    const Topology& topology() const { return *topology_; }
    const char* topologyName() const { return topology_->name(); }
    void setTopology(std::unique_ptr<Topology> t); // 运行时切换构型（环面/双曲）
    void reprojectAll();                           // 全量重投影（大数据可放后台线程）

    // ---- 选择与聚焦 ----
    void select(uint64_t id); // 选中并自动以该对象为拓扑中心
    void highlight(uint64_t id); // 只标记选中，不重投影、不挪动其他对象
    void pinPosition(uint64_t id, const Vec3& pos); // 解锁后拖动：钉住世界坐标
    void clearSelection();
    uint64_t selected() const { return selected_; }
    Vec3 centerPos() const;   // 当前拓扑中心的 3D 位置（无选中 → 原点）

    // ---- 可见性收集：视锥剔除 + 距离 LOD + 边强度过滤（渲染层每帧调用）----
    void collectVisible(const ViewFrustum& frustum, const Vec3& camPos,
                        float maxDist, float edgeStrengthThreshold,
                        std::vector<RenderNode>& outNodes,
                        std::vector<RenderEdge>& outEdges) const;

    // ---- 拾取：投影空间 3D 射线（由屏幕坐标反投影后调用）----
    uint64_t pick(const Ray& ray) const;

    size_t entityCount() const { return entities_.size(); }
    const std::vector<Entity>& entities() const { return entities_; }

private:
    void rebuildIndex();
    void rebuildOctree();

    uint64_t nextId_ = 1;
    std::unique_ptr<Topology> topology_;
    std::vector<Entity> entities_;                // AoS 紧凑存储
    std::unordered_map<uint64_t, size_t> index_;  // id → 下标
    std::vector<Relation> relations_;
    uint64_t selected_ = 0;
    std::unique_ptr<Octree> octree_;
};

} // namespace om
