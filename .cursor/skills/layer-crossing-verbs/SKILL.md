---
name: layer-crossing-verbs
description: >-
  经验四层（gerber、CNC、BZOX、CT，自下而上）之间禁止 part_of / composes / constitutes。
  物因、本因、极因是角色，不是层名；每一层都可以有这三种。
  跨层只允许 construct_of（构成）、从属于服务关系，或由主语驱动并指向上下文的算子（如铜环柱）。
  Use when adding or drawing a relation that crosses gerber/CNC/BZOX/CT, when a part_of
  edge becomes a via after a lift, or when the user says 层间、构成、construct_of、从属于、升层.
---

# 层间动词

OrcheEditor 有同一份技能。改一份必须同步另一份。

## 原话

层间不可能是part_of, part_of这样的同类假的聚合指向，没有必可，也没有可能会导致升层。
要么是构成（construct_of），要么是从属于服务关系，必定是底层的极因，上层的物因之一。或者由指向上下文的重要主语驱动，例如铜环柱。

## 四个层

框从下向上是 gerber、CNC、BZOX、CT。物因、本因、极因是角色，不是框的名字。每一层都可以有这三种对象。下一层的极因，可以成为上一层的一种物因。

| 层 | 经验图上的框 | 现在放着 |
|---|---|---|
| gerber | 最下面的宽框 | 信号层（铜层）、焊盘、走线，以及离焊盘间距等条件。极因是背钻 |
| CNC | 往上 | 孔、背钻段 |
| BZOX | 再往上 | Z抽出 |
| CT | 最上面 | 铜环柱、残桩 |

电路板边车、左侧主语列不是层。`part_of` 连到边车可以留下。主语列是树：自动编程软件（BDEditor，Z抽出 的动因主语）、生产方，其下是刀具（残桩的主语，也就是钻刀），以及 CT机。CNC 和 CT 机做不了 Z抽出。背钻的工艺要求从 gerber 指向 CNC 的孔。

## 动词

| 跨层？ | 算子 | 中文 | 何时用 |
|---|---|---|---|
| 否 | `part_of` `composes` `constitutes` | 是部分 / 组成 | 只在同一层。升层之后若仍是这些动词，删掉这根跨层线，不要改个中文标签了事 |
| 是 | `construct_of` | 构成 | 跨层的构成。不要写成已有的 `constructs_from`（那是「在有限条件下构建」） |
| 是 | `subordinate_to` | 从属于 | 服务关系。底层一端是极因，上层一端是物因之一 |
| 是 | 主语驱动的算子 | 边上写英文算子 | 主语在层外。`plate_barrel`、`volumizes` 都从背钻孔指向铜环柱。`Put`（放置）从 BDEditor 指向 Z抽出，只在选中 Z抽出 时画上。`specifies` 从 gerber 的背钻指向 CNC 的孔 |

`serves` 在动词表里仍是提议。未批准前不要写进世界文件。

## 落笔前

1. 看两端落在哪一层。同层才允许 `part_of` / `composes` / `constitutes`。
2. 要跨层时先选定上表三种之一，并写上英文算子。OID 只追加，不重编号。
3. 经验图：层间线用曲线，颜色与层内线不同。同一对端点的两条算子向两侧弯开。
