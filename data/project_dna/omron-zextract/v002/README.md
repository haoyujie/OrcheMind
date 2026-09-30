# Omron Z 抽出 DNA v002

上一版 `v001` 冻结。本版对齐 `zextract-rules.odaaf` **graphVersion 1.2.0**，按 `rule/07_ZExtraction/definitions` 展开单信号层三元绑定。

- 信号层承载背钻孔；背钻孔构成焊盘（1:1）。
- 焊盘组成近区与信号线（同级）；走线承载 Z 窗；近区禁止 Z 中心。
- 焊盘与 Z 为 M:N；焊盘从属于焊盘对；焊盘对以配对走线为目的。
- 放置管线、规则参数、PCB WIN296 仍在。

椭圆简图 / 星形 / 树形布局以本图为硬靶。坐标见 `coordinates.json`（声明谐波，非训练嵌入）。
