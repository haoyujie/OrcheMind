// OMStudio 核心自测（无第三方框架）：
// 拓扑投影正确性、中心聚焦、拾取命中/未命中、CRUD、视锥剔除、C API 冒烟
#include "omstudio/OmStudio.h"
#include "omstudio/Scene.h"
#include "omstudio/Topology.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

static int g_fail = 0;
#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (cond) {                                                            \
            std::printf("  [ok] %s\n", msg);                                   \
        } else {                                                               \
            std::printf("  [FAIL] %s (line %d)\n", msg, __LINE__);             \
            ++g_fail;                                                          \
        }                                                                      \
    } while (0)

namespace {

om::VecN makeVec(int dim, float a, float b, float c, float d, float e = 0, float f = 0) {
    om::VecN v;
    if (dim >= 1) v.push_back(a);
    if (dim >= 2) v.push_back(b);
    if (dim >= 3) v.push_back(c);
    if (dim >= 4) v.push_back(d);
    if (dim >= 5) v.push_back(e);
    if (dim >= 6) v.push_back(f);
    return v;
}

void testTorus() {
    std::printf("[test] Clifford torus topology\n");
    auto t = om::makeTorusTopology(6);
    CHECK(t->dim() == 6, "dim()==6");
    CHECK(std::string(t->name()) == "clifford-torus", "name");
    auto p = t->project(makeVec(6, 1, 0, 0, 1, 0.5f, 0.2f));
    CHECK(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z), "project finite");
    // 中心聚焦：选中 (1,0,0,1) 后它应投影到原点附近（其角度被对齐到 0）
    t->setCenter(makeVec(6, 1, 0, 0, 1));
    auto pc = t->project(makeVec(6, 1, 0, 0, 1, 0.0f, 0.0f));
    CHECK(pc.length() < 1e-3f, "setCenter -> center projects to origin");
}

void testHyperbolic() {
    std::printf("[test] Poincare ball topology\n");
    auto t = om::makeHyperbolicTopology(6);
    CHECK(std::string(t->name()) == "poincare-ball", "name");
    auto p = t->project(makeVec(6, 0.5f, 0.3f, 0.2f, 0.1f, 0.0f, 0.0f));
    CHECK(p.length() < 4.1f, "embedding stays inside ball (scale=4)");
    t->setCenter(makeVec(6, 0.4f, 0.2f, 0.1f, 0.0f, 0.0f, 0.0f));
    auto pc = t->project(makeVec(6, 0.4f, 0.2f, 0.1f, 0.0f, 0.0f, 0.0f));
    CHECK(pc.length() < 0.05f, "Mobius translation moves center to origin");
    float d = t->distance(makeVec(6, 0, 0, 0, 0, 0, 0), makeVec(6, 0.5f, 0, 0, 0, 0, 0));
    CHECK(d > 0.0f && std::isfinite(d), "Poincare distance positive");
}

void testScene() {
    std::printf("[test] Scene CRUD / pick / collect\n");
    om::Scene scene(om::makeTorusTopology(6));

    om::Entity a;
    a.highDim = makeVec(6, 1, 0, 0, 0, 0, 0);
    a.radius = 0.2f;
    a.label = "A";
    om::Entity b;
    b.highDim = makeVec(6, 0, 1, 0, 0, 0, 0);
    b.radius = 0.2f;
    b.label = "B";
    uint64_t idA = scene.addEntity(a);
    uint64_t idB = scene.addEntity(b);
    CHECK(idA != 0 && idB != 0, "addEntity assigns ids");
    CHECK(scene.entityCount() == 2, "entityCount");

    scene.setRelation(idA, idB, 0.9f);
    CHECK(scene.relations().size() == 1, "setRelation");

    // 拾取：射线直穿 A 球心
    const om::Entity* ea = scene.find(idA);
    om::Ray ray{ea->pos3 - om::Vec3{0, 0, 5}, {0, 0, 1}};
    CHECK(scene.pick(ray) == idA, "pick hits A");

    // 未命中：反向射线
    om::Ray miss{ea->pos3 + om::Vec3{0, 0, 5}, {0, 0, 1}};
    CHECK(scene.pick(miss) == 0, "pick miss");

    // 选中聚焦：A 成为拓扑中心
    scene.select(idA);
    CHECK(scene.selected() == idA, "select");
    const om::Entity* ea2 = scene.find(idA);
    CHECK(ea2->pos3.length() < 1e-3f, "selected entity at origin");

    // 删除
    CHECK(scene.removeEntity(idA), "removeEntity");
    CHECK(scene.entityCount() == 1, "count after remove");
    CHECK(scene.find(idA) == nullptr, "find removed returns null");
}

void testCapi() {
    std::printf("[test] C API smoke\n");
    om_init("torus", 6);
    CHECK(om_entity_count() == 0, "empty scene");
    float dims[6] = {1, 0, 0, 0, 0, 0};
    om_entity e{};
    e.dims = dims;
    e.dim_count = 6;
    e.radius = 0.2f;
    e.color = 0xFF0000;
    uint64_t id = om_add_entity(&e);
    CHECK(id != 0, "add entity");
    CHECK(om_entity_count() == 1, "count==1");

    om_select(id); // 聚焦：实体被移到拓扑原点
    om_set_camera(0, 0, 5, 0, 0, 0, 0, 1, 0, 45, 1.6f, 0.05f, 100);
    // 拾取视锥中心（射线指向原点，应命中被聚焦的实体）
    uint64_t hit = om_pick_screen(0.0f, 0.0f);
    CHECK(hit == id, "pick screen center hits focused entity");
    om_shutdown();
}

void testCollect() {
    std::printf("[test] collectVisible frustum culling\n");
    om::Scene scene(om::makeTorusTopology(6));
    for (int i = 0; i < 50; ++i) {
        om::Entity e;
        e.highDim = makeVec(6, (float)(i % 7), (float)(i % 5), (float)(i % 3), 1.0f, 0, 0);
        om::normalizeInPlace(e.highDim);
        e.radius = 0.1f;
        scene.addEntity(e);
    }
    // 全视锥：左右上下各半空间 + 近/远覆盖全部 z
    om::ViewFrustum all;
    all.planes[0] = {{1, 0, 0}, 1e5f};  // x >= -1e5
    all.planes[1] = {{-1, 0, 0}, 1e5f}; // x <= +1e5
    all.planes[2] = {{0, 1, 0}, 1e5f};  // y >= -1e5
    all.planes[3] = {{0, -1, 0}, 1e5f}; // y <= +1e5
    all.planes[4] = {{0, 0, 1}, 1e5f};  // z >= -1e5
    all.planes[5] = {{0, 0, -1}, 1e5f}; // z <= +1e5
    std::vector<om::RenderNode> nodes;
    std::vector<om::RenderEdge> edges;
    scene.collectVisible(all, {0, 0, -10}, 100.0f, 0.0f, nodes, edges);
    CHECK(nodes.size() == scene.entityCount(), "all entities visible in full frustum");
}

} // namespace

int main() {
    std::printf("OMStudio core self-test\n");
    testTorus();
    testHyperbolic();
    testScene();
    testCollect();
    testCapi();
    if (g_fail == 0) {
        std::printf("ALL PASSED\n");
        return 0;
    }
    std::printf("%d FAILED\n", g_fail);
    return 1;
}
