# 系统需求

这组文档写清 OrcheMind / ODAAF 要做成什么样，以及为什么这样分层、这样呈现。它是需求，不是实现说明；实现细节在代码和各仓库的 skill 里。

**读者**：维护 ODAAF（编辑器）、OrcheMind（推理与坐标）、OMStudio（3D 总览）的人，以及帮助他们的代理。

## 索引

| 文件 | 回答的问题 |
|---|---|
| [01-goals.md](01-goals.md) | 最终要达到什么；人、MIB、代码各管什么 |
| [02-knowledge-layers.md](02-knowledge-layers.md) | 三层知识模型；层与权重；原子和上帝怎么区分；最小粒度 |
| [03-spaces.md](03-spaces.md) | 宇宙、物自体表、X/N/V 三层；统一的物自体空间；裁判与运动员 |
| [04-presentation.md](04-presentation.md) | 焦点、漫游与翻译两种模式、2.5D 平面堆叠、交互与验收 |
| [05-coordinates.md](05-coordinates.md) | OID 是永久主键；位置向量由 OID 派生；给 OrcheMind 的 Q 位置契约 |
| [06-3d-overview.md](06-3d-overview.md) | OMStudio 的定位；场景导出 JSON 契约 |
| [glossary.md](glossary.md) | 中英术语表 |

## 与其它文档的关系

- [`../roadmap.md`](../roadmap.md) 是 OrcheMind 推理引擎的路线图；本组文档是它第 2 阶段"全局坐标"和 ODAAF 呈现的上位需求。
- 哲学术语（物自体、现象、抽象自）以 [`noumenon-phenomenon-projection`](../../.cursor/skills/noumenon-phenomenon-projection/SKILL.md) skill 为准，本组文档只引用，不重新定义。
- 数据文件在 ODAAF 数据仓库（`OmronXmlGenerator/ODAAF`）：`world/pcb-world.odaaf`（物自体表）、`omron/omron-nodes.odaaf`（视图）、`modules.registry.xml`（模块登记表）。

## 已定的取舍

1. 知识层先用三层（粒子、连接、结构），七层以后在"大层分界"上扩展。
2. 层与权重是两个独立的量。
3. 先做 2.5D（ODAAF 内），后做 3D（OMStudio，只读总览）。
4. OID 全局唯一，是永久主键；位置向量不是主键，随本体版本优化。
5. 层间连接少而贵：每一条跨层边都要有理由，由裁判检查。
