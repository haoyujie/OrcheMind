# RCC8 与 RotatE：下一步行动计划

日期：2026-09-27  
依据：`resources/01dialog/summary/OrcheMind开发结论.md`，以及豆包讨论 46、47。  
V01 裸图已经校验并通过，快照在 `snapshot/v01_raw_graph/`。本文件只安排下一批准备，不开始训练。

执行状态（2026-09-27）：A、B、C 已完成。工作图与快照字节一致；`validate_v01.py` 与 `check_rcc8_composition.py` 通过。公理包在 `data/rcc8/`，35 条抽象三元组和 10 道组合题在 `data/triples/`。V02 和 RotatE 沙箱尚未开始。

## 结论

现在做三件事，并且按这个顺序：

1. 复查 V01 快照仍在，不重建、不改节点。
2. 在本地写成 RCC8 公理包：八个关系、逆对、组合表。这是 Schema，不是训练集。不必先从网上抓一份 OWL。
3. 手写一小份抽象区域三元组，用组合表当验收题。训练仍排在这之后。

PCB 背钻工程和 ODAAF 是以后的应用层。它们提供领域名词和工程动词，不能充当已经固化的基础模型，也不能充当第一次训练和第二次训练的材料。这两份材料目前都还没有准备。

RotatE 只借鉴「关系是复平面上的旋转」。不下载、不加载任何在 WN18RR 上预训练的权重。WN18RR 若要用来确认旋转公式能在 4060 上跑通，必须放在隔离的沙箱目录，不得写入 `data/ontology_graph/`。

## 文稿里需要改口的三处

豆包 46 把 RCC8 和 GeoSPARQL 写成了同一套谓词。实际是两套：

- RCC8（Randell, Cui, Cohn, 1992）的八个关系互斥且完备：任意两个面区域恰好落在其中一种。代号是 DC、EC、PO、EQ、TPP、TPPi、NTPP、NTPPi。
- GeoSPARQL / Simple Features 里的 `crosses`、`contains`、`overlaps`、`within` 是另一套拓扑谓词。`intersects` 还覆盖多种相交，并不互斥。

「穿过、贯穿、穿透」是我们要挂在空间簇旁边的工程动词，不是 RCC8 的第八个以外的基本关系。通孔穿过焊盘涉及层叠和三维，RCC8 只谈二维区域是否连通、重叠、包含。焊盘区域与孔的平面投影可以试着用 NTPP / TPP 描述；「穿过信号层」要另立谓词。

WN18RR 是 WordNet 词义三元组，约 4.1 万实体、11 种词汇关系、训练集约 8.7 万条。那 11 种是 `hypernym`、`has_part`、`member_meronym`、`instance_hypernym`、`verb_group`、`similar_to` 一类，没有 DC/NTPP，也几乎没有 PCB 拓扑。它是算法基准，不是谓词库。

RotatE（Sun 等，ICLR 2019）的公式是：实体在复数空间里，关系的每一维都是单位复数旋转，`h ∘ r ≈ t`（Hadamard 积）。这和「一个动词一个角度」同族，但标准 RotatE 是很多维各自一个角度，不是单圆上的一个角度。PyKEEN 能下载 WN18RR 并实现这个模型。它没有残差、没有簇原型、没有簇内/簇间损失。公开发表的检查点若存在，学的是 WordNet 词义空间，不能读进 OrcheMind。

## 现有成果测什么

V01 已经证明的只有：24 个节点、20 条无类型边、从四大类能走到十二范畴和八个 RCC8 名字、快照与工作文件字节一致。

它还没有证明：

- TPP 与 TPPi、NTPP 与 NTPPi 是互逆；
- 八个关系互斥且完备；
- 组合表：已知 A–R1–B、B–R2–C，A 与 C 的可能关系集合；
- 任何嵌入或训练。

所以「测试实步成果」的下一项不是再跑一遍建图，而是用组合表做逻辑验收。例子：A NTPPi B 且 B NTPPi C，则 A 与 C 只能是 NTPPi。这个例子不需要 GPU，也不需要下载。

## 两类本地工程各自是什么

OmronXmlGenerator 的对象关系文档描述背钻编程里的 Land、Component、Part、Spec 和图层。ODAAF 的四因（质料、形式、动力、目的）、永久 UUID、机器名和 `version-manager.ps1` 描述的是规则发布版本，不是 OrcheMind 的快照进化。

以后的接法：

- PCB 名词（焊盘、通孔、铜皮、信号层）作为领域实体，挂到已经冻结的空间关系上，不反向定义那八个关系。
- 「穿过」作为扩展动词，与 RCC8 同簇、不同节点。
- ODAAF 四因是审查框架。动力因以后可以对照康德的因果性，形式因对照 Schema 本身。现在不把两棵树合并，也不把 ODAAF 的 CSV 规则当成三元组语料。

## 下载清单

| 资源 | 这一步 | 放到哪里 | 原因 |
| --- | --- | --- | --- |
| RCC8 组合表 | 本地手写，不下载 | `data/rcc8/composition.json` | 八关系和 8×8 组合表是公开标准，网上的 OWL 常把 GeoSPARQL 混进来 |
| 抽象区域三元组 | 手写几十条 | `data/triples/rcc8_abstract.jsonl` | RCC8 自己没有训练集 |
| WN18RR | 暂不下载 | 若以后做沙箱：`data/sandbox/wn18rr/` | 词汇关系，不能进入本体图 |
| RotatE 预训练权重 | 不下载 | 无 | 角度不是人工扇区，且无簇约束 |
| PyTorch / PyKEEN | 这一步不装 | V03 之前再开一次环境 | 组合表和三元组还没验收 |
| Wikidata、完整 WordNet、SemRelData、康德全文、CCL | 不下载 | 无 | 体积大、谓词杂，不能当基础模型 |

## 行动顺序

### A. 确认 V01 未被动过

对照 `snapshot/v01_raw_graph/v01_graph.json` 与 `data/ontology_graph/v01_graph.json` 字节一致，再跑 `src/orchemind_graph/validate_v01.py`。不一致就停止，先查是谁改了工作文件。不要重新 `build_v01.py`，否则 UUID 会全部换掉，快照对不上。

### B. 写 RCC8 公理包（仍不训练）

新建 `data/rcc8/`，三个文件：

- `relations.json`：八个代号、英文全称、中文名、逆关系。逆对只有两对：TPP↔TPPi，NTPP↔NTPPi。DC、EC、PO、EQ 的逆是自己。
- `composition.json`：64 格。每一格是关系集合，不是单一关系。空集表示矛盾，用来当负例。
- `not_rcc8.json`：明确列出暂不进入八关系的词：穿过、贯穿、穿透、crosses、contains（GeoSPARQL）、图层、方向、距离。

验收：一个小脚本读组合表。输入 NTPPi 然后 NTPPi，输出必须只含 NTPPi。再抽 DC 与 PO 等几格做同样检查。脚本放在 `src/orchemind_graph/check_rcc8_composition.py`。通过前不改 V01 图。

### C. 写第一份训练/测试材料（仍然不训练）

`data/triples/rcc8_abstract.jsonl`，每行一条：

```json
{"head":"region_a","relation":"NTPP","tail":"region_b","split":"train"}
```

规模先 30 到 50 条，实体用 `region_a` 这类抽象名，不用焊盘、通孔。另写 10 条组合题放 `data/triples/rcc8_composition_tests.jsonl`：两条前提加一个允许的结论集合。这 10 条是测试题，不进训练。

这一步结束时，才算「基础模型要训的材料」有了第一版。它只覆盖协同性（共存）下的空间簇。量、质、模态、因果、实体与偶性仍然没有语料，这是预期，不在这一步补 Wikidata。

### D. 然后才是 V02，而不是立刻 RotatE

V02 在不动 V01 拓扑的前提下，给 Link 补 `directed` 和 `link_type`，并把逆对写成类型。组合表仍留在 `data/rcc8/`，不塞进节点名字里。完成后快照 `snapshot/v02_typed_link/`。

### E. RotatE 沙箱与正式训练分开

沙箱（可选，V03 之前的环境步骤）：Python 3.12 虚拟环境另装 PyTorch（CPU 或 4060 的 CUDA 择一），用 PyKEEN 在隔离目录跑 WN18RR 上的 RotatE，只确认 `h ∘ r ≈ t` 能收敛几个 epoch。跑完把目录留在 `data/sandbox/`，不合并权重。

正式训练（V03）：在我们自己的 `rcc8_abstract` 上从头训练。模型在 RotatE 的旋转之外加上残差、增量裁剪、簇原型，以及

```text
L = L_task + L_intra + L_inter + L_alignment
```

验收看组合题，不看 WN18RR 的 Hits@10。`L_alignment` 和公网千问仍晚于这次旋转原型。PCB 名词要等组合题稳定后再作为新实体加入，关系仍只能从八个 RCC8 谓词里选，或从已经登记的扩展动词里选。

## 这一步明确不做

- 不把 OmronXmlGenerator 或 ODAAF 导入本体图。
- 不下载 Wikidata、WordNet 全库、预训练 RotatE。
- 不微调千问。
- 不把「穿过」写进 RCC8 八关系。
- 不重跑 `build_v01.py`。
