#include "omstudio/Scene.h"
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace om {

namespace {
constexpr float kWorldHalf = 64.0f; // 八叉树世界范围（投影空间）
}

Scene::Scene(std::unique_ptr<Topology> topology)
    : topology_(std::move(topology)),
      octree_(new Octree({-kWorldHalf, -kWorldHalf, -kWorldHalf},
                         {kWorldHalf, kWorldHalf, kWorldHalf})) {}

Scene::~Scene() = default;

void Scene::rebuildIndex() {
    index_.clear();
    for (size_t i = 0; i < entities_.size(); ++i) index_[entities_[i].id] = i;
}

void Scene::rebuildOctree() { octree_->rebuild(entities_); }

uint64_t Scene::addEntity(const Entity& e) {
    Entity ent = e;
    if (ent.id == 0) ent.id = nextId_++;
    ent.pos3 = topology_->project(ent.highDim);
    index_[ent.id] = entities_.size();
    entities_.push_back(std::move(ent));
    octree_->insert(entities_.back().id, entities_.back().pos3);
    return entities_.back().id;
}

bool Scene::removeEntity(uint64_t id) {
    auto it = index_.find(id);
    if (it == index_.end()) return false;
    // 清理相关关系
    relations_.erase(std::remove_if(relations_.begin(), relations_.end(),
                                    [id](const Relation& r) { return r.a == id || r.b == id; }),
                     relations_.end());
    // swap-pop 删除（紧凑内存），随后重建索引与空间索引
    size_t i = it->second;
    entities_[i] = std::move(entities_.back());
    entities_.pop_back();
    if (selected_ == id) selected_ = 0;
    rebuildIndex();
    rebuildOctree();
    return true;
}

const Entity* Scene::find(uint64_t id) const {
    auto it = index_.find(id);
    return it == index_.end() ? nullptr : &entities_[it->second];
}

Entity* Scene::find(uint64_t id) {
    auto it = index_.find(id);
    return it == index_.end() ? nullptr : &entities_[it->second];
}

void Scene::updateEntity(uint64_t id, const VecN& highDim, float radius, uint32_t color) {
    Entity* e = find(id);
    if (!e) return;
    e->highDim = highDim;
    e->radius = radius;
    e->color = color;
    e->pos3 = topology_->project(e->highDim);
    rebuildOctree();
}

std::vector<uint64_t> Scene::childrenOf(uint64_t parentId) const {
    std::vector<uint64_t> out;
    for (const auto& e : entities_)
        if (e.parentId == parentId) out.push_back(e.id);
    return out;
}

void Scene::setRelation(uint64_t a, uint64_t b, float strength) {
    if (a == b || !find(a) || !find(b)) return;
    if (a > b) std::swap(a, b);
    for (auto& r : relations_)
        if (r.a == a && r.b == b) { r.strength = strength; return; }
    relations_.push_back({a, b, strength});
}

bool Scene::removeRelation(uint64_t a, uint64_t b) {
    if (a > b) std::swap(a, b);
    auto it = std::remove_if(relations_.begin(), relations_.end(),
                             [&](const Relation& r) { return r.a == a && r.b == b; });
    if (it == relations_.end()) return false;
    relations_.erase(it);
    return true;
}

void Scene::setTopology(std::unique_ptr<Topology> t) {
    topology_ = std::move(t);
    reprojectAll();
}

void Scene::reprojectAll() {
    for (auto& e : entities_) e.pos3 = topology_->project(e.highDim);
    rebuildOctree();
}

void Scene::select(uint64_t id) {
    selected_ = id;
    const Entity* e = find(id);
    if (e) {
        topology_->setCenter(e->highDim);
        reprojectAll();
    }
}

void Scene::clearSelection() {
    selected_ = 0;
    VecN zero(topology_->dim(), 0.0f);
    topology_->setCenter(zero);
    reprojectAll();
}

Vec3 Scene::centerPos() const {
    const Entity* e = find(selected_);
    return e ? e->pos3 : Vec3{};
}

void Scene::collectVisible(const ViewFrustum& frustum, const Vec3& camPos,
                           float maxDist, float edgeStrengthThreshold,
                           std::vector<RenderNode>& outNodes,
                           std::vector<RenderEdge>& outEdges) const {
    outNodes.clear();
    outEdges.clear();

    // 1) 视锥剔除：只遍历八叉树相交节点，不触碰全部实体
    std::vector<uint64_t> visible;
    octree_->queryFrustum(frustum, visible);

    // 2) 距离 LOD 分级
    const float lodNear = maxDist * 0.35f;
    const float lodMid = maxDist * 0.70f;
    std::unordered_set<uint64_t> visSet(visible.begin(), visible.end());

    for (uint64_t id : visible) {
        auto it = index_.find(id);
        if (it == index_.end()) continue;
        const Entity& e = entities_[it->second];
        float d = (e.pos3 - camPos).length();
        if (d > maxDist) continue;
        int lod = d < lodNear ? 0 : (d < lodMid ? 1 : 2);
        float r = e.radius * (lod == 2 ? 0.35f : (lod == 1 ? 0.65f : 1.0f));
        outNodes.push_back({e.id, e.pos3, r, e.color, lod});
    }

    // 3) 边：仅当两端均可见、强度达阈值、且不超中距离（远区不渲染边）
    for (const auto& rel : relations_) {
        if (rel.strength < edgeStrengthThreshold) continue;
        if (!visSet.count(rel.a) || !visSet.count(rel.b)) continue;
        auto ia = index_.find(rel.a), ib = index_.find(rel.b);
        if (ia == index_.end() || ib == index_.end()) continue;
        const Vec3& pa = entities_[ia->second].pos3;
        const Vec3& pb = entities_[ib->second].pos3;
        float da = (pa - camPos).length(), db = (pb - camPos).length();
        if (std::max(da, db) > lodMid) continue;
        outEdges.push_back({pa, pb, rel.strength});
    }
}

uint64_t Scene::pick(const Ray& ray) const {
    std::vector<uint64_t> cands;
    octree_->queryRay(ray, cands); // 空间索引过滤候选，避免 O(N) 全量求交
    uint64_t best = 0;
    float bestT = 1e30f;
    for (uint64_t id : cands) {
        auto it = index_.find(id);
        if (it == index_.end()) continue;
        const Entity& e = entities_[it->second];
        Vec3 oc = e.pos3 - ray.origin;
        float proj = oc.dot(ray.dir);
        if (proj < 0.0f) continue;
        float perp2 = oc.lengthSq() - proj * proj;
        float r2 = e.radius * e.radius;
        if (perp2 <= r2) {
            float t = proj - std::sqrt(std::max(0.0f, r2 - perp2));
            if (t < bestT) { bestT = t; best = id; }
        }
    }
    return best;
}

} // namespace om
