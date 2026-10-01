#include "omstudio/OmStudio.h"
#include "omstudio/Scene.h"
#include "omstudio/Topology.h"
#include <cstring>
#include <mutex>
#include <string>

// 全局单例（C# 编辑器单视图场景；多视图可扩展为实例表）
static om::Scene* g_scene = nullptr;
static std::mutex g_mutex;
static om::Mat4 g_viewProj = om::Mat4::identity();
static om::Mat4 g_invViewProj = om::Mat4::identity();
static om::Mat4 g_view = om::Mat4::identity(); // 视图矩阵，用于反推相机位置

static om::Scene* needScene() {
    if (!g_scene) g_scene = new om::Scene(om::makeTorusTopology(6));
    return g_scene;
}

extern "C" {

void om_init(const char* topology, int dim) {
    std::lock_guard<std::mutex> lock(g_mutex);
    delete g_scene;
    g_scene = nullptr;
    if (dim < 4) dim = 4;
    if (topology && std::strcmp(topology, "hyperbolic") == 0)
        g_scene = new om::Scene(om::makeHyperbolicTopology(dim));
    else
        g_scene = new om::Scene(om::makeTorusTopology(dim));
}

void om_shutdown(void) {
    std::lock_guard<std::mutex> lock(g_mutex);
    delete g_scene;
    g_scene = nullptr;
}

void om_set_topology(const char* topology) {
    std::lock_guard<std::mutex> lock(g_mutex);
    om::Scene* s = needScene();
    int dim = s->topology().dim();
    if (topology && std::strcmp(topology, "hyperbolic") == 0)
        s->setTopology(om::makeHyperbolicTopology(dim));
    else
        s->setTopology(om::makeTorusTopology(dim));
}

const char* om_topology_name(void) {
    std::lock_guard<std::mutex> lock(g_mutex);
    return needScene()->topologyName();
}

void om_reproject(void) {
    std::lock_guard<std::mutex> lock(g_mutex);
    needScene()->reprojectAll();
}

uint64_t om_add_entity(const om_entity* e) {
    std::lock_guard<std::mutex> lock(g_mutex);
    om::Scene* s = needScene();
    om::Entity ent;
    ent.id = e->id;
    if (e->dims && e->dim_count > 0)
        ent.highDim.assign(e->dims, e->dims + e->dim_count);
    ent.radius = e->radius > 0 ? e->radius : 0.08f;
    ent.color = e->color ? e->color : 0x88CCFF;
    ent.parentId = e->parent_id;
    if (e->label) ent.label = e->label;
    return s->addEntity(ent);
}

int om_remove_entity(uint64_t id) {
    std::lock_guard<std::mutex> lock(g_mutex);
    return needScene()->removeEntity(id) ? 1 : 0;
}

int om_get_entity(uint64_t id, om_entity* out) {
    std::lock_guard<std::mutex> lock(g_mutex);
    const om::Entity* e = needScene()->find(id);
    if (!e || !out) return 0;
    out->id = e->id;
    out->dim_count = (int)e->highDim.size();
    if (out->dims && !e->highDim.empty())
        std::memcpy(out->dims, e->highDim.data(), e->highDim.size() * sizeof(float));
    out->radius = e->radius;
    out->color = e->color;
    out->parent_id = e->parentId;
    out->label = e->label.c_str();
    return 1;
}

void om_update_entity(uint64_t id, const float* dims, int dim_count, float radius, uint32_t color) {
    std::lock_guard<std::mutex> lock(g_mutex);
    om::VecN v;
    if (dims && dim_count > 0) v.assign(dims, dims + dim_count);
    needScene()->updateEntity(id, v, radius, color);
}

uint64_t om_entity_count(void) {
    std::lock_guard<std::mutex> lock(g_mutex);
    return (uint64_t)needScene()->entityCount();
}

uint64_t om_selected(void) {
    std::lock_guard<std::mutex> lock(g_mutex);
    return needScene()->selected();
}

void om_select(uint64_t id) {
    std::lock_guard<std::mutex> lock(g_mutex);
    needScene()->select(id);
}

void om_clear_selection(void) {
    std::lock_guard<std::mutex> lock(g_mutex);
    needScene()->clearSelection();
}

void om_set_relation(uint64_t a, uint64_t b, float strength) {
    std::lock_guard<std::mutex> lock(g_mutex);
    needScene()->setRelation(a, b, strength);
}

int om_remove_relation(uint64_t a, uint64_t b) {
    std::lock_guard<std::mutex> lock(g_mutex);
    return needScene()->removeRelation(a, b) ? 1 : 0;
}

void om_set_camera(float eyeX, float eyeY, float eyeZ,
                   float tgtX, float tgtY, float tgtZ,
                   float upX, float upY, float upZ,
                   float fovYDeg, float aspect, float zn, float zf) {
    std::lock_guard<std::mutex> lock(g_mutex);
    om::Mat4 proj = om::Mat4::perspective(fovYDeg, aspect, zn, zf);
    om::Mat4 view = om::Mat4::lookAt({eyeX, eyeY, eyeZ}, {tgtX, tgtY, tgtZ}, {upX, upY, upZ});
    g_viewProj = proj * view;
    g_invViewProj = g_viewProj.inverted();
    g_view = view;
}

uint64_t om_pick_screen(float ndcX, float ndcY) {
    std::lock_guard<std::mutex> lock(g_mutex);
    om::Ray ray = om::Mat4::unproject(g_invViewProj, ndcX, ndcY);
    return needScene()->pick(ray);
}

void om_collect_visible(float maxDist, float edgeStrengthThreshold,
                        om_render_node* nodes, int* nodeCount,
                        om_render_edge* edges, int* edgeCount) {
    std::lock_guard<std::mutex> lock(g_mutex);
    om::Scene* s = needScene();
    om::ViewFrustum frustum = om::Mat4::extractFrustum(g_viewProj);
    // 相机世界位置 = 视图矩阵逆的平移部分
    om::Vec3 camPos = g_view.inverted().transformPoint({0, 0, 0});

    std::vector<om::RenderNode> ns;
    std::vector<om::RenderEdge> es;
    s->collectVisible(frustum, camPos, maxDist, edgeStrengthThreshold, ns, es);

    int nc = *nodeCount, ec = *edgeCount;
    *nodeCount = (int)ns.size();
    *edgeCount = (int)es.size();
    for (int i = 0; i < nc && i < (int)ns.size(); ++i) {
        nodes[i].id = ns[i].id;
        nodes[i].x = ns[i].pos.x; nodes[i].y = ns[i].pos.y; nodes[i].z = ns[i].pos.z;
        nodes[i].radius = ns[i].radius;
        nodes[i].color = ns[i].color;
        nodes[i].lod = ns[i].lod;
    }
    for (int i = 0; i < ec && i < (int)es.size(); ++i) {
        edges[i].ax = es[i].pa.x; edges[i].ay = es[i].pa.y; edges[i].az = es[i].pa.z;
        edges[i].bx = es[i].pb.x; edges[i].by = es[i].pb.y; edges[i].bz = es[i].pb.z;
        edges[i].strength = es[i].strength;
    }
}

} // extern "C"
