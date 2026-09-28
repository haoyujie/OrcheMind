# 训练与测试材料在哪



OrcheMind 把「Schema（人定义）」和「训练语料（三元组）」分开存放。下面是当前盘点。



## 总览



| 用途 | 路径 | 说明 |

| --- | --- | --- |

| 第一层本体图（node/link） | `data/ontology_graph/v01_graph.json`、`v02_graph.json` | 康德四大类 + 十二范畴 + RCC8 名；V02 有类型边。**不是**三元组训练集 |

| 第一层只读快照 | `snapshot/v01_raw_graph/`、`snapshot/v02_typed_link/` | 已固化，勿改 |

| RCC8 Schema | `data/rcc8/relations.json`、`composition.json`、`not_rcc8.json` | 八关系、组合表、明确排除的词（穿过等） |

| 第二层角度先验 | `data/rcc8/angles_f0.json`（语义扇区）、`operator_prior.json`（旋转算子） | 空心点 / 实心点来源 |

| **当前正式训练三元组** | `data/triples/rcc8_abstract.jsonl` | 35 条，抽象区域，`split=train`；V03 用这份 |

| 组合验收题 | `data/triples/rcc8_composition_tests.jsonl` | 10 道；测组合表，不进旋转训练 |

| **更大演示/测试包** | `data/triples/rcc8_demo_large.jsonl` | 约 145 条、38 实体；八关系均有样本；`split=demo`，默认**不**进 `train_v03.py` |

| 视图场景按钮 | `data/demo/scenarios.json` | 第一层穷尽节点查询 + 第二层八动词预设 |

| 第二层动词目录 | `data/demo/layer2_verb_catalog.json` | 每动词的角、逆、示例三元组 |

| Luxray 对齐目标 | `data/rcc8/llm_verb_targets.json` | 八动词冻结嵌入（768 维） |

| 训练导出 | `data/ontology_graph/v03_embedding.json` | 学到的算子角等 |

| 权重（不入库） | `data/sandbox/v03/` | `model.pt` 等 |



## 两层各自测什么



1. **第一层 node–link**：打开视图，用 `scenarios.json` 里的预设点「量 / 质 / 关系 / 模态」和十二范畴、八个 RCC8 名；地图应高亮对应节点与连线。「穿过」会提示未入图（见 `not_rcc8.json`）。

2. **第二层关系圆**：同一视图右侧；`layer2_verb_catalog.json` 穷举 DC…NTPPi 的语义角、算子先验与演示三元组。大包 `rcc8_demo_large.jsonl` 保证每个动词都有足够 `(head, relation, tail)`。



## 生成 / 刷新演示包



```powershell

python src\orchemind_graph\build_demo_pack.py

```



若要把大包拿去重训，需改 `train_v03.py` 的输入路径或合并 jsonl；当前正式基线仍是 35 条 `rcc8_abstract.jsonl`。


