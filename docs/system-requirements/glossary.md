# 术语表

哲学术语以 [`noumenon-phenomenon-projection/reference.md`](../../.cursor/skills/noumenon-phenomenon-projection/reference.md) 为准；这里只列本组需求文档新引入或常用的词。机器标识符不翻译。

| 中文 | English | 标识符 | 意思 |
|---|---|---|---|
| 知识层 | knowledge layer | `KnowledgeLayer` | 粒子 / 连接 / 结构三层之一 |
| 粒子 | particle | `Particle` | 输入；当前焦点下的最小粒度 |
| 连接 | connection | `Connection` | 把粒子组织起来的中间对象 |
| 结构 | structure | `Structure` | 得到的整体、种类、规范 |
| 大层分界 | major-layer boundary | — | 焦点范围的边界；其上第一层是最小粒度 |
| 最小粒度 | minimal granularity | — | 大层分界之上第一层的通用对象，随焦点而变 |
| 层级值 / 营养级 | trophic level | `Level` | 广义营养级 \(h\)，归一化为 0..1 |
| 营养一致性 | trophic coherence | `Incoherence` | \(F_0\)，越小层次越清楚 |
| 权重 / 认可度 | weight / acknowledgement | `Weight` | 不同认可方的数目，对数归一化 |
| 认可方 | acknowledger | — | 引用一个节点的不同主体；最小是人，现以宇宙、结构、文件代替 |
| 四因 | four causes | `CauseRoles` | 质料、形式、动力、目的；质料多元说明处在大层中部 |
| 宇宙 | universe | `UniverseInfo` | 一套视图或世界的全部对象 |
| 物自体表 / 本原表 | world file | `urn:odaaf:world:pcb` | 共享的 N 层文件 |
| 统一的物自体空间 | unified world space | — | 跨领域的 N 层，相当于波普尔"世界 3" |
| 裁判 / 运动员 | referee / athlete | `RefereeCheck` | 世界里的规则 / 受检的视图 |
| 焦点 | focus | — | 当前关注的宇宙或转换，决定呈现模式 |
| 漫游模式 | roam mode | `roam` | 一个宇宙内部，按知识层分平面 |
| 翻译模式 | translate mode | `translate` | 源 / 世界 / 目标三平面 |
| 平面堆叠 | plane stack | `PlaneStack` | 2.5D：每层一个倾斜平面 |
| 倾角 | tilt | `--tilt` | 平面倾斜角，0° 即二维 |
| 竖线 / 过孔 | via | — | 跨平面的边 |
| 已对上 / 缺口 / 私有 | matched / gap / private | — | 翻译模式的三类标注 |
| 模块弧 | module arc | `moduleArc` | 每个文件在 `私有根.2` 下的编号 |
| 模块登记表 | module registry | `modules.registry.xml` | 模块弧的唯一分配处 |
| 退役弧号 | retired arc | `odaaf:RetiredArcs` | 用过的弧号，永不复用 |
| 位置向量 | position vector | — | 由 OID 派生，按版本固定，可优化 |
| 坐标方案 | coordinate scheme | `coord_scheme` | OID 到相位的算法版本 |
| 场景导出 | scene export | `odaaf.scene` | ODAAF 给 OMStudio / OrcheMind 的 JSON |
