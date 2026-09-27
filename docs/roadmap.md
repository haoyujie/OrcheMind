# OrcheMind 路线

理论以 `resources/01dialog/summary/OrcheMind开发结论.md` 为准。本文件只记录工程阶段。

## V01 `v01_raw_graph`（已固化）

裸拓扑：Node 只有 `id` 与 `name`，Link 只有 `source_id` 与 `target_id`。没有端口、类型、方向、向量，也不训练。

图的内容是康德四大类、十二范畴，以及挂在「协同性（共存）」下的 RCC8 八个关系。工作文件在 `data/ontology_graph/v01_graph.json`。快照在 `snapshot/v01_raw_graph/`，只读。

RCC8 公理包和第一批抽象三元组已写好，见 `docs/next-rcc8-rotate.md`。尚未下载 RotatE 预训练权重，也还没有把 PCB / ODAAF 当作基础语料。

## V02 `v02_typed_link`（已固化）

在 V01 的同一批节点和 20 条边上增加 `link_type`、`directed` 和端口。四大类到十二范畴是 `has_category`，协同性到八个 RCC8 是 `has_rcc8`，TPP/TPPi 与 NTPP/NTPPi 各有一条无方向的 `inverse_of`。工作文件在 `data/ontology_graph/v02_graph.json`。快照在 `snapshot/v02_typed_link/`，只读。仍不训练。

## 视图（当前）

`view/index.html` 两层：本体地图（V02）；第一谐波圆上空心点为 `angles_f0` 语义扇区，实心点为 V03 学到的算子角。

```text
venv\Scripts\python.exe src\orchemind_graph\serve_view.py
```

然后打开 http://127.0.0.1:8765/view/index.html 。

## V03 `v03_embedding_base`（已验收）

残差旋转：展示角与算子先验分离；各复维可在较大 `max_delta` 内散开，展示角用 fidelity 拉回先验。损失为过滤式全实体 CE + 弱簇约束 + fidelity，再接 `L_alignment`（Luxray nomic 嵌入，可学习 `W_align`，不微调大模型）。

验收（`eval_v03.py`）：算子漂移 ≤8.5°；过滤 Hit@1 ≥85%（当前 35/35）。

```text
python src\orchemind_embedding\train_v03.py
python src\orchemind_embedding\fetch_llm_targets.py
python src\orchemind_embedding\train_v03_align.py
python src\orchemind_embedding\eval_v03.py
python src\orchemind_graph\export_figures.py
```

导出：`data/ontology_graph/v03_embedding.json`，快照：`snapshot/v03_embedding_base/`（含关系圆图）。不改 V02 图拓扑，不加载 WN18RR 权重。

公开库图示约定见 `docs/figures/README.md`。
