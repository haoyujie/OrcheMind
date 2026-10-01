// 演示数据生成：层级"生长" + 语义聚类 + 近邻强关联
#pragma once

namespace om {
class Scene;
}

namespace om::demo {

// count: 目标实体总数（根 + 分层生长）；seed: 随机种子
void generate(Scene& scene, int count, unsigned seed = 42);

} // namespace om::demo
