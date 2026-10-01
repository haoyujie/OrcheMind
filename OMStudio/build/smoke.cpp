#include "omstudio/Scene.h"
#include "omstudio/Topology.h"
#include <cstdio>
#include <cstdlib>
#include <random>
int main(int argc, char** argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 5000;
    om::Scene s(om::makeTorusTopology(6));
    std::mt19937 rng(1); std::normal_distribution<float> nd(0,1);
    for (int i = 0; i < n; ++i) {
        om::Entity e; e.highDim.resize(6);
        for (auto& x : e.highDim) x = nd(rng);
        om::normalizeInPlace(e.highDim);
        e.radius = 0.05f; e.color = 0x88CCFF; e.label = "smoke";
        s.addEntity(e);
    }
    s.select(s.entities()[n/2].id); // 全量重投影 + 八叉树重建
    const auto& ents = s.entities();
    om::Vec3 c = ents[n/2].pos3;
    om::Ray r{c - om::Vec3{0,0,3}, {0,0,1}};
    uint64_t hit = s.pick(r);
    std::printf("entities=%zu selected_at_origin=%d pick_hit=%llu\n",
                s.entityCount(), (int)(ents[n/2].pos3.length() < 1e-3f), (unsigned long long)hit);
    return hit ? 0 : 1;
}
