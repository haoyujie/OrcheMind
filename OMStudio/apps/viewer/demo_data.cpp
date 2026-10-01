#include "demo_data.h"
#include "omstudio/Scene.h"
#include <random>
#include <string>

namespace om::demo {

void generate(Scene& scene, int count, unsigned seed) {
    if (count < 1) count = 1;
    std::mt19937 rng(seed);
    std::normal_distribution<float> nd(0.0f, 1.0f);
    std::uniform_real_distribution<float> ud(-1.0f, 1.0f);

    auto randVec = [&](int dim) {
        VecN v(dim);
        for (auto& x : v) x = nd(rng);
        normalizeInPlace(v);
        return v;
    };

    // 根对象
    Entity root;
    root.highDim = randVec(6);
    root.radius = 0.16f;
    root.color = 0xFFE08A;
    root.label = "root";
    uint64_t rootId = scene.addEntity(root);

    // 分层生长：子向量向父靠拢形成语义聚类；第5维加径向偏置（越深越"向外长"）
    const uint32_t layerColor[4] = {0xFFE08A, 0x7FB3FF, 0x7CE0A0, 0xF98FB3};
    const float depthBias[4] = {0.0f, 0.18f, 0.32f, 0.44f};
    std::vector<uint64_t> level{rootId};
    int total = 1;
    for (int depth = 1; depth < 4 && total < count; ++depth) {
        std::vector<uint64_t> next;
        for (uint64_t pid : level) {
            const Entity* pe = scene.find(pid);
            int kids = 2 + (int)(rng() % 3);
            for (int k = 0; k < kids && total < count; ++k) {
                VecN v(6);
                for (int d = 0; d < 6; ++d) v[d] = pe->highDim[d] * 0.72f + nd(rng) * 0.28f;
                v[4] += depthBias[depth]; // 径向"生长"偏置 → 发丝向外
                v[5] = ud(rng);           // 第6维：个体差异
                normalizeInPlace(v);

                Entity e;
                e.highDim = v;
                e.radius = 0.10f - 0.015f * depth;
                e.color = layerColor[depth % 4];
                e.parentId = pid;
                e.label = "morpheme_" + std::to_string(total + 1);
                uint64_t id = scene.addEntity(e);
                scene.setRelation(pid, id, 0.5f + (float)(rng() % 10) / 50.0f);
                ++total;
                next.push_back(id);
            }
        }
        level = std::move(next);
    }

    // 近邻强关联（按高维距离抽样）：形成网状簇（环面构型下的"平等关联"）
    const auto& ents = scene.entities();
    int tries = count * 2;
    for (int t = 0; t < tries; ++t) {
        uint64_t a = ents[rng() % ents.size()].id;
        uint64_t b = ents[rng() % ents.size()].id;
        if (a == b) continue;
        const Entity* ea = scene.find(a);
        const Entity* eb = scene.find(b);
        float d = scene.topology().distance(ea->highDim, eb->highDim);
        if (d < 0.45f) {
            float s = 1.0f / (1.0f + d);
            scene.setRelation(a, b, s > 0.95f ? 0.95f : s);
        }
    }
}

} // namespace om::demo
