# Example: Z extraction site (world first)

The thing-in-itself is `cls-pw-z-site` in `g:\myfuture\OntoLib\BzoxOdaaf\world\pcb-world.odaaf`.
It is a basic element: the copper patch already exists on the trace. It is found late, after the
backdrill hole, the pad where that hole meets the signal layer, and the trace leaving that pad
are all there. Used or unused, it is still there.

Views only represent it:

| Layer | Class | Relation | Used on BZOX-978? |
|---|---|---|---|
| World (N) | `cls-pw-z-site` | — | yes (class) |
| CNC | — | none | **unused** (no Z window in Gerber / drill) |
| XML | `cls-bzoxClZExtractionWindow` | `rel-xml-zwindow-abstracts-zsite` | 2049 windows |
| PCB | `cls-bzoxClPcbWin296` (`parentClassId=cls-WIN`, TypeFlag=296) | `rel-pcb-win296-abstracts-zsite` | 2049 WIN 296 |

`cls-WIN` is a computer carrier, not the ontology. TypeFlag=254 (`cls-bzoxClPcbWin254`) abstracts
from the hole; only 296 abstracts from the Z site. ISU is the cake cut from the CT film; algorithms
only serve a WIN. That constraint stays in the PCB view.

The information group is `str-pw-z-site-group` (P8). Pattern P3 applies at the world: the site is
found, not created. Pattern P2 applies only to a view artefact (XML window, PCB WIN 296).

## Standard phrasing (four causes + 5W1H)

```text
[信号层焊盘] 找到自己的 [Z 抽出点位]。
Z 抽出点位 的输入由 背钻孔、背钻段、信号层焊盘、焊盘引出的走线、信号层铜 组成。
Z 抽出点位 是什么：世界：Z 抽出点位；XML环：Z抽出窗；PCB环：PCB窗296；CNC环：未用到。
如何得到：沿走线向外，离焊盘不小于最小距离；只为背钻孔找；多个焊盘可共享同一点位。
在哪里：走线铜面的一部分（毫米坐标是 DataProperty）。
何时：背钻之后、残桩判定之前的 Z 测量。
目的：点位铜面的 Z 是残桩长度计算的零基准。
```

Grammar (world / find):

```text
find ZExtractionSite WITH Hole, BackdrillSegment, Pad, Trace, CopperLayer
  BY ZSiteClearOfPad, ZSiteOnBackdrilledPad, ZSiteShareable
  IN Trace
  WHEN ZMeasuring, Backdrilling
  FOR Stub
```

## Instance: BZOX-978 Land 33 / 34 (BL2600128A)

Board `F:\pcb\issues\BZOX-978-jw-gerber\2609141339-pl0271551d10`.

| View | Fact |
|---|---|
| CNC | Land 34 = `V6_34_X112992Y074549_TC02_6`, tool TC02, Ø 0.400 mm, depth 0.180, stop L3. Same XY has Land 2156 / TS03 / stop L24 (a second site, other copper layer). |
| World | Two Z sites at one XY: one on L3, one on L24. Each is on its own trace, DC from its pad. |
| XML | Window 368: `LandId=33` `SharedLandIds=34` `SignalLayer=L3` `ToolId=TC02` box ≈ (111.161,72.554)–(112.061,74.154) mm. Land 34 has **no** own `LandId`. |
| PCB | One WIN TypeFlag=296 per XML window (2049 = 2049). The 296 is relative to its ISU; 254 lands stay 4442. |
| Grade | Land 33 = A / StrokeMarchOut gap 1.21 mm. Land 34 = E / NoResult in `holes.bits.txt`, but it is covered by window 368 as a shared land. |

Board totals: holes 4442 = A 1886 + B 163 + E 2393. A+B = 2049 = XML windows = PCB 296.
1530 of those windows carry `SharedLandIds`. `keptWindows=3690` in the header counts coverage
after share/merge, not distinct sites.

CNC remains unused: no Z-site leaf in the CNC ring. That is not a missing world class.

## Chain (P9)

```text
G-world  find ZExtractionSite   →  material of G-xml
G-xml    create ZExtractionWindow from the site (CNC mm)
G-pcb    create PcbWin296 from the XML window (relative to ISU)
G-insp   WIN 296 + items 1747/7971  →  stub measurement (0 = site.surfaceZMm)
```

`rel-constitutesPcbWin` (XML window → WIN 296) is a view-to-view production edge inside one
pipeline. Cross-view *identity* still goes through the world leaf, never through `corresponds`.
