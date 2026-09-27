# 公开库可视化展示方案

面向 GitHub 访客：首页一眼看到当前主图；每个重要版本在自己的快照页留下当时关键图。

## 两层结构

| 层 | 路径 | 作用 |
| --- | --- | --- |
| 首页主图 | `docs/figures/hero/` | README 顶部嵌入；始终等于「当前对外主展示」 |
| 版本图 | `snapshot/<version>/figures/` | 该版本固化时的关键画面；只读，随快照一起冻结 |

首页主图当前约定：

- `ontology_map.png`：V02 本体地图（四列康德 + RCC8）
- `relation_circle.png`：V03 第一谐波关系圆（实心=学到的算子角，空心=语义先验）

## 版本页约定

每个重要 `snapshot/<id>/` 至少：

1. `README.md`：一句话说明本版冻结了什么，并嵌入本目录 `figures/` 下的图。
2. `figures/`：本版关键图（地图和/或关系圆），由 `export_figures.py` 生成后入库。

已落地：

- V01：`snapshot/v01_raw_graph/figures/ontology_map.png`
- V02：地图 + `relation_circle_prior.png`（尚未训练时的语义扇区）
- V03：`relation_circle.png`（训练后算子角）

## 如何刷新

改完图数据或训练导出后：

```powershell
python src\orchemind_graph\export_figures.py
```

然后检查 README 与各 `snapshot/*/README.md` 的相对路径是否仍指向新文件。**不要手动画图替代脚本**，也不要把临时截图塞进 `hero/` 而不更新脚本。

本地交互页仍是：

```powershell
python src\orchemind_graph\serve_view.py
```

静态 PNG 给 GitHub 渲染；HTML 视图给本机点选。

## 以后每升一版怎么做

1. 固化 `snapshot/vXX_.../`（JSON + 说明）。
2. 在 `export_figures.py` 增加该版本的出图目标。
3. 跑导出，把 PNG 放进该 snapshot 的 `figures/`。
4. 写/改该版本 `README.md` 嵌图。
5. 若该版成为新的「对外主展示」，再覆盖 `docs/figures/hero/` 并改根 README 文案。

## 不做的事

- 不把浏览器实时截图当唯一真相（难复现）。
- 不把 `data/sandbox/*.pt` 或大模型权重放进图目录。
- 密钥不进仓库；Luxray 只用于生成可入库的小目标向量 `data/rcc8/llm_verb_targets.json`。
