---
name: noumenon-phenomenon-projection
description: >-
  Models one objective thing seen through several independent representations
  (Kant's thing-in-itself / appearance, Plato's Form / image), and builds the
  objective world with Kant's forms of intuition and table of categories.
  Use when modeling 物自体、现象、本体、客观世界、超集、投射、抽象自 (abstracted_from),
  when CNC / XML / PCB (CT) data describe the same physical board, when deciding
  whether a leaf belongs to the objective world or to a view, when splitting an
  objective-world ontology from a project ontology, or when the user cites Kant,
  the Critique of Pure Reason, Plato, the cave, or Theory of Forms.
---

# 物自体、现象与抽象投射

## 一句话

一块真实电路板只有一个。CNC、XML、PCB（CT）是三台不同仪器对它的独立表象，像从三个机位拍的照片。表象之间不互相映射；每片表象叶子只说明自己抽象自客观世界的哪一片叶子。

## 三层，不能并成两层

按《纯粹理性批判》严格分，用户口中的“物自体”其实分成两层。带几何、层数、孔径的那一层，在康德那里仍是**经验对象**，不是先验意义上的物自体。

| 层 | 康德 | 柏拉图 | 工程里是什么 | 能有叶子吗 |
|---|---|---|---|---|
| X 先验对象 | 物自体（消极意义的本体），只可思维、不可认识；界限概念 | 无对应：柏拉图的理念可以被理性认识，康德的物自体不行 | 每类实物一个空锚点，只有身份 | **不能**。无 DataProperty，无经验关系 |
| N 客观世界 | 经验对象 / 自然；经验上的“事物自身”（雨相对于彩虹） | 可感的个别物，分有（methexis）理念；其几何骨架属线喻的 dianoia | 实物板：轮廓、叠层、铜、孔、背钻段、焊盘、走线、基准点 | 能。只放物理事实 |
| V 表象 | 经某一立场、某一仪器给出的表象 | eikones，影像；《理想国》598a：同一张床从侧面、正面看只是“显得”不同 | CNC 制造、XML 中间、PCB 检测三个视图 | 能。允许私有叶子 |

柏拉图的理念（eidos）对应 N 层的**种类**（Class 本身），不对应 X。不要把理念当成物自体。

## 动词分三域，互不越界

| 域 | 动词 | 方向 | 用在哪 |
|---|---|---|---|
| 经验域（存在） | `part_of` `composes` `constitutes` `causes` `inheres_in`，RCC8 | 同一层内部 | N 内部；V 内部各自 |
| 表示域 | `abstracted_from`（抽象自） | V 叶子 → N 叶子 | 只跨 V→N |
| 界限 | `appearance_of`（是……的显现） | N 种类 → X | 只思维，不推理 |

规则：

1. **V 与 V 之间不连线**。同形词也不连。要比较，就开临时比较上下文：两个视图各自 `abstracted_from` 到同一片 N 叶子，才算同一客观对象的两次显现。
2. **N 不指向 V**。没有 `generates`、`produces`。客观对象不生成文件；是仪器表象它。
3. **X 上不加范畴**。康德只许范畴用于可能经验（A246/B303）。X 不能 `causes`、不能 `part_of`、不能有量和质。`appearance_of` 只说明“显现总要有某物显现”（Bxxvi–xxvii），不能借它推任何事实。
4. **`abstracted_from` 不是继承，也不是部分**。它不传递、不派生子类、不进环层级。多对一合法：CNC 孔、PCB WIN 都可抽象自同一个实物孔。
5. **私有叶子不连**。stop 值、刀号、检测阈值、文件段落只属于视图；没有 `abstracted_from` 就是私有，不要硬造客观对应物。
6. **“超集”只对可抽象的部分成立**。各视图的私有叶子在 N 之外，所以 N 不是各视图的字面并集。

## 按《纯粹理性批判》建 N 层

N 层不是随手列物件，而是按经验的条件来组织。

**直观形式（先验感性论）**

- 空间：所有 N 叶子共用一个板坐标系，单位毫米。区域关系用 RCC8。视图坐标先换到这个坐标系再比较。
- 时间：制造是有先后的事件（层压、钻孔、电镀、背钻）。这些是 Perdurant，不是板的属性。

**范畴表（A80/B106）落到叶子上**

| 范畴 | 工程落点 | 例 |
|---|---|---|
| 量：单一、多数、全体 | 外延量、计数 | 一块板；孔数；板宽、板高、板厚（mm） |
| 质：实在、否定、限制 | 内包量、程度 | 孔壁有铜＝实在；背钻去掉镀铜＝否定；残桩高度＝限制（剩下的程度） |
| 关系：实体与偶性 | 持存者与它的属性 | 板与铜层持存；孔径是孔的属性（`inheres_in`） |
| 关系：因果 | 事件引起变化，须有时间先后 | 背钻 `causes` 背钻段；电镀 `causes` 孔壁铜 |
| 关系：协同（共存） | 同时并存的相互关系 | 各铜层同时并存；孔与层的 RCC8 关系 |
| 模态：可能、现实、必然 | **不是叶子** | 见下 |

**模态不写成对象属性。** 康德说模态不给概念的内容加任何东西，只说它与认识能力的关系（A219/B266）。所以“设计上允许”（可能）、“实测如此”（现实）、“按规律必然”（必然）写在证据、判决或身份状态上，不写成板的 DataProperty。

**图型法（A137/B176）：每个范畴要有可测规则才落地。** 量对应 mm 与计数，质对应程度与阈值，因果对应时间先后，共存对应同一时刻的 RCC8 判定。写不出可测规则的范畴叶子，先不建。

**三条经验类比**

1. 实体持存（第一类比）：板在全部工序中是同一个持存者；工序改变的是它的状态，不换身份 UUID。
2. 因果（第二类比）：只有事件之间才 `causes`。物件之间不用 `causes`。
3. 协同（第三类比）：同时存在的部分之间用 RCC8 或共存关系，不用因果。

## 文件怎么拆

- **客观世界单独一个文件**，与具体项目无关：不带 Omron、BZOX、CNC、XML、PCB 的词，也不带任何文件格式字段。别的项目也能用它。
- **项目本体只放视图（V）**，通过 `abstracted_from` 引用客观世界的叶子。
- 工程文件 `.odaafproj` 同时列出两个本体：客观世界只读、为主；项目视图引用它。
- OID 只要求在一个 `.odaaf` / `.mib` 里不重复。跨文件引用用 UUID 或 URI，不靠 OID 前缀。

## 改动前后自查

```
- [ ] 新叶子先判层：X / N / V
- [ ] N 叶子里没有文件格式、软件名、项目名
- [ ] X 节点没有 DataProperty，也没有经验域关系
- [ ] V→V 没有新增直接关系
- [ ] abstracted_from 只从 V 指向 N，宾语是叶子，不是环
- [ ] causes 只连事件；物件之间用 part_of / inheres_in / RCC8
- [ ] 模态写在证据或状态上，不写成叶子
- [ ] 视图私有叶子没有被硬塞进 N
```

## 常见错误

- 把带属性的实物板叫“物自体”，却给它几何和层数。几何属于经验对象；严格的物自体没有属性。
- 认为三个视图是三个宇宙，再在它们之间搭桥。
- 用继承表达抽象：CNC 孔不是实物孔的子类。
- 让客观世界“生成”视图。
- 把柏拉图的理念与康德的物自体当成同一个东西。理念可被理性认识；物自体不可认识。

## 原典出处

页码、原意与使用边界见 [reference.md](reference.md)。
