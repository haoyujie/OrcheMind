# 原典出处与使用边界

## 三语术语表

中文译名在不同译本里不统一，以德文原词为准。机器标识符一律用英文，不翻译。

### 康德

| 中文 | Deutsch | English | 本系统 |
|---|---|---|---|
| 物自体 / 自在之物 | Ding an sich | thing in itself | X 层；只有身份 |
| 本体（消极意义） | Noumenon im negativen Verstande | noumenon in the negative sense | X 层（B307） |
| 本体（积极意义） | Noumenon im positiven Verstande | noumenon in the positive sense | **不建**：需要理智直观，人没有 |
| 先验对象 = X | transzendentaler Gegenstand = X | transcendental object = X | X 层锚点（A109） |
| 界限概念 | Grenzbegriff | limiting concept | X 层存在的理由（A255/B310） |
| 现象 / 显现 | Erscheinung | appearance | V 层每片叶子；N 层在先验上也是现象 |
| 现象体 | Phaenomenon | phenomenon | 经范畴规定后的现象 |
| 表象 | Vorstellung | representation | V 层：仪器给出的表象 |
| 经验对象 | Gegenstand der Erfahrung | object of experience | N 层 |
| 自然 | Natur | nature | N 层整体 |
| 经验实在、先验观念 | empirische Realität, transzendentale Idealität | empirical reality, transcendental ideality | N 层工程上真实，但不冒充物自体（A28/B44） |
| 直观 | Anschauung | intuition | 可测量的给予 |
| 直观形式 | Form der Anschauung | form of intuition | 空间、时间 |
| 空间 | Raum | space | 板坐标系，毫米；RCC8 |
| 时间 | Zeit | time | 工序先后；事件 |
| 先验感性论 | transzendentale Ästhetik | transcendental aesthetic | 空间、时间一节 |
| 先验分析论 | transzendentale Analytik | transcendental analytic | 范畴、图型、原理 |
| 知性 | Verstand | understanding | 给出范畴 |
| 理性 | Vernunft | reason | 产生理念；只作调节用 |
| 范畴 | Kategorie | category | 量、质、关系、模态 |
| 范畴表 | Tafel der Kategorien | table of categories | A80/B106 |
| 量：单一、多数、全体 | Einheit, Vielheit, Allheit | unity, plurality, totality | 序号、计数、整体 |
| 质：实在、否定、限制 | Realität, Negation, Limitation | reality, negation, limitation | 有铜、去铜、残桩程度 |
| 实体与偶性 | Substanz und Akzidenz (Inhärenz und Subsistenz) | substance and accident (inherence and subsistence) | 持存者；`inheres_in` |
| 因果 | Kausalität und Dependenz | causality and dependence | `causes`，只连事件 |
| 协同（共存） | Gemeinschaft (Wechselwirkung) | community (reciprocity) | RCC8；并存 |
| 模态：可能、现实、必然 | Möglichkeit, Dasein, Notwendigkeit | possibility, existence (actuality), necessity | 写在证据与状态上，不是叶子 |
| 外延量 | extensive Größe | extensive magnitude | 长度、面积、计数 |
| 内包量 / 度 | intensive Größe, Grad | intensive magnitude, degree | 程度；残桩高度 |
| 图型法 | Schematismus | schematism | 范畴须有可测规则 |
| 经验类比 | Analogien der Erfahrung | analogies of experience | 持存、相继、共存 |
| 经验思维的公设 | Postulate des empirischen Denkens | postulates of empirical thought | 模态的处理（A218/B265） |
| 调节性使用 | regulativer Gebrauch | regulative use | 理念只引导，不给对象 |
| 构成性 | konstitutiv | constitutive | 范畴对经验是构成性的 |
| 准则 | Maxime | maxim | 视图自己的做法（运动员）；《实践理性批判》 |
| 法则 | Gesetz | law | 世界文件里的规则（裁判）；准则要拿法则检验 |

### 柏拉图

| 中文 | Greek | English | 本系统 |
|---|---|---|---|
| 理念 / 形式 | εἶδος (eidos), ἰδέα (idea) | Form, Idea | N 层的种类（Class），不是 X |
| 分有 | μέθεξις (methexis) | participation | 个别板分有板的种类 |
| 影像 | εἰκών (eikōn) | image | V 层的文件 |
| 想象（线喻最低段） | εἰκασία (eikasia) | imagination, image-thinking | 只看文件 |
| 信念 | πίστις (pistis) | belief, conviction | 看到实物板 |
| 数理思维 | διάνοια (dianoia) | thought, reasoning | 板的几何模型 |
| 理性直观 | νόησις (noesis) | understanding, intellection | 把握种类本身 |
| 洞穴 | — | the cave | 只看一面墙上的影子 |

### 本系统标识符（不翻译）

| 标识符 | 中文 | 意思 |
|---|---|---|
| `abstracted_from` | 抽象自 | V 叶子 → N 叶子；表示域 |
| `appearance_of` | 是……的显现 | N 种类 → X；界限，只可思维 |
| `x` / `nature` | 先验对象层 / 经验对象层 | 结构的 `layer` 值 |
| `urn:odaaf:world:pcb` | 物自体表 / 本原表 | 世界文件 `pcb-world.odaaf`；id 前缀 `cls-pw-` 等 |
| `odaaf:Imports` | 导入 | 项目文件只读引用世界文件；不保存、不分配身份 |
| `odaaf:RetiredArcs` | 退役弧号 | 移走条目用过的弧号，永不复用 |
| `part_of` `composes` `constitutes` `causes` `inheres_in` | 是部分、组成、构成、引起、依存于 | 经验域，同一层内部 |
| RCC8：`DC` `EC` `PO` `EQ` `TPP` `TPPi` `NTPP` `NTPPi` | 相离、外切、部分相交、等同、正切真子集及其逆、非切真子集及其逆 | 共存的空间形式 |

## 出处

康德按 A/B 页码（第一版 1781 / 第二版 1787）。柏拉图按 Stephanus 页码。公有领域译本见 `data/project_dna/critiques.md`：Meiklejohn 译《纯粹理性批判》，Project Gutenberg ebook 4280。下面只写原意，不长段引用。

### 康德《纯粹理性批判》

| 处 | 原意 | 在本系统里的用法 |
|---|---|---|
| Bxvi–xviii | 哥白尼式转向：不是认识依照对象，而是对象依照我们的认识方式 | 视图是仪器的表象方式；N 层按经验条件组织 |
| Bxxvi–xxvii | 物自体不可认识，但必须能被思维；否则会出现“有显现而无显现者”的荒谬 | X 层只留身份；`appearance_of` 只表达“显现总有所显现” |
| A20/B34 | 现象是经验直观的未规定对象；分质料与形式 | 表象有仪器给出的形式，也有来自对象的内容 |
| A22/B36 起 | 空间、时间是感性直观的先天形式 | N 层统一板坐标系（空间）与工序先后（时间） |
| A28/B44 | 空间经验上实在、先验上观念 | N 层在工程上是“真实的”，但不冒充先验物自体 |
| A45–46/B62–63 | 彩虹与雨：经验上，雨相对彩虹是“事物自身”；先验上雨也只是现象 | N 层正是这种经验意义上的“事物自身”；三个文件像彩虹 |
| A51/B75 | 思维无内容是空的，直观无概念是盲的 | N 层每片叶子既要有可测量（直观），也要有种类（概念） |
| A80/B106 | 十二范畴表 | N 层叶子按量、质、关系落位；模态单独处理 |
| A104–110（A109） | 先验对象 = X | X 层锚点 |
| A137/B176 | 图型法：范畴须经时间规定的图型才能用于现象 | 每个范畴要有可测规则才建叶子 |
| A158/B197 | 一般经验可能性的条件，同时是经验对象可能性的条件 | 坐标系、单位、工序先后是建 N 层的前提，不是可选项 |
| B224 / B232 / B256 | 三条经验类比：实体持存、因果相继、共存交互 | 持存不换 UUID；`causes` 只连事件；并存用 RCC8 |
| A218/B265；A219/B266 | 经验思维的公设；模态不给概念内容加任何东西 | 模态写在证据、判决、状态上，不写成叶子 |
| A235/B294 起 | 现象与本体之区分 | 三层划分的总依据 |
| A246/B303 | 范畴只可用于可能经验的对象 | X 上不加任何范畴关系 |
| B307 | 消极意义的本体：不是感性直观的对象 | X 层只能是空锚点 |
| A255/B310–311 | 本体是界限概念，限制感性的僭越 | X 层存在的意义是划界，不是存数据 |
| A313–320/B370–377 | 康德评柏拉图的“理念”，借用其词指理性概念 | 说明柏拉图理念与康德物自体不是一回事 |

### 康德《实践理性批判》

| 处 | 原意 | 在本系统里的用法 |
|---|---|---|
| §7（科学院版 5:30） | 纯粹实践理性的基本法则：准则须能同时作为普遍立法的原则 | 只借结构：视图的做法（准则）拿世界规则（法则）检验；裁判与运动员分开。不赋予规则道德含义 |

### 柏拉图

| 处 | 原意 | 用法 |
|---|---|---|
| 《理想国》509d–511e 线喻 | eikasia（影像）< pistis（可感物）< dianoia（数理）< noesis（理念） | 文件＝影像；实物板＝可感物；板的几何模型＝数理；种类＝理念 |
| 《理想国》514a–520a 洞穴 | 囚徒只见墙上的影子 | 只看某一个文件，就像只看一面墙上的影子 |
| 《理想国》596a–598d 三张床 | 理念的床、木匠的床、画家的床；598a：同一张床从侧面、正面看只是显得不同 | 三个视图是同一实物从不同角度的影像，实物本身并没有变 |
| 《斐多》100c–d | 美的东西因分有美本身而美 | 个别板“分有”板的种类；这是种类与个别，不是表象与对象 |

## 两家的分界（必须守住）

- 柏拉图：理念最真，理性可以认识。
- 康德：物自体不可认识；我们认识的是现象，但现象在经验上是实在的。
- 本系统取康德的三层划分为骨架，柏拉图只用来说明“影像与原物”“种类与个别”。不要写出“物自体可以被完全建模”。

## OrcheMind 已有与缺口（写这个技能时）

已有：

- 第一层本体图：康德四大类、十二范畴；RCC8 挂在“协同性（共存）”下（`data/ontology_graph/v01_graph.json`、`v02_graph.json`）。
- `verbs.json`：三大批判各对应 schema_only 动词（`constructs_from`、`forbids`、`obliges`、`aims_at`）。
- `resources/02cncxmlpcb/3mirros.md`：本体 / 三视图 / 比较上下文的讨论草稿。

缺：

- 物自体、现象、先验对象 X、直观形式（空间、时间）、图型法，都没有节点。
- 表示域动词 `abstracted_from`、界限动词 `appearance_of` 不在 `verbs.json` 里，也没有表示域这个簇。
