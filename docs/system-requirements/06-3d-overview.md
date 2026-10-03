# 06 3D 总览

## 定位

OMStudio（`OMStudio/`，C++ / Magnum）是**只读**的全局总览：

- 同屏显示全部宇宙：四个视图 donut 加一个世界 donut；世界放在中心，视图围一圈。
- 用来找到感兴趣的区域。找到后点选节点，带着 OID 跳回 ODAAF（`--select <OID 或 id>`），在 2.5D 里放大、编辑。
- 不编辑，不校验。所有修改都回到 ODAAF。

## 场景导出（JSON 契约）

ODAAF 导出，OMStudio 和 OrcheMind 读取。文件名建议 `<项目>.scene.json`，放在 `%TEMP%\odaaf-auto\` 或项目旁。

```json
{
  "format": "odaaf.scene",
  "formatVersion": 1,
  "project": "urn:odaaf:omron:nodes",
  "graphVersion": "1.10.0",
  "universes": [
    { "id": "str-ring-cnc", "label": "CNC", "rootOid": "...", "isWorld": false,
      "center": [x, y], "radius": r }
  ],
  "nodes": [
    { "oid": "1.3.6.1.4.1.55555.2.863.1.45", "id": "cls-Tool", "label": "刀具",
      "module": "urn:odaaf:omron:nodes", "imported": false,
      "universe": 0, "layer": 0, "level": 0.12, "weight": 0.66,
      "torus": [phi, depth, theta], "xy": [x, y] }
  ],
  "edges": [
    { "oid": "...", "from": "cls-Tool", "to": "cls-pw-drill-bit", "verb": "abstracted_from",
      "kind": "projection", "crossLayer": true }
  ]
}
```

字段约定：

- `layer`：0 粒子、1 连接、2 结构（[02-knowledge-layers.md](02-knowledge-layers.md)）。
- `kind`：`structural`、`associative`、`projection`、`companion`、`inherit`。
- `torus`：\(\varphi\) 弧度（宇宙方位），`depth` 环深（整数），\(\theta\) 弧度（环内角）。
- 节点按 `oid` 唯一；`id` 只供 ODAAF 内部回查。

## 当前实现边界

ODAAF 已导出 `.scene.json`（`SceneExport`）。OMStudio 仍是单流形演示；读取该 JSON、同屏五个 donut 是下一步，不阻塞 2.5D 编辑。找到区域后仍用 `--select <OID 或 id>` 跳回 ODAAF。

- 每个宇宙一个 donut（环面）。donut 中心落在 `universes[i].center`；世界在原点。
- 节点在环面上的位置：大圆角 = \(\theta\)，小圆角由 `layer` 决定（粒子在内侧、结构在外侧），离环面中心的距离按 `depth` 稍向外推。
- `projection` 边是 donut 之间的"过孔"，单独着色；层间连接少，所以这些线稀疏、可读。
- 权重决定节点大小。
