# Merc Troll Den  `(trollden)`

[← back to world map](WORLD-MAP.md) · 5 rooms · vnums 2801–2805

Grey dashed nodes leave the area; green dashed nodes (`▸ Part X`) continue on another sub-map below.

## Map

```mermaid
graph LR
  R2801["Wastedump<br/>#2801"]
  R2802["Troll's Larder<br/>#2802"]
  R2803["Troll's Living Room<br/>#2803"]
  R2804["The Playpen<br/>#2804"]
  R2805["Troll's Bedroom<br/>#2805"]
  R2801 -->|N| X6124
  R2801 -->|S| R2803
  R2802 -->|E| R2803
  R2803 -->|N| R2801
  R2803 -->|E| R2804
  R2803 -->|S| R2805
  R2803 -->|W| R2802
  R2804 -->|W| R2803
  R2805 -->|N| R2803
  X6124["A dead end path in the deep, dark forest<br/>haon #6124"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
  classDef part fill:#16313a,stroke:#5aa,color:#bfe,stroke-dasharray:4 2;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 2801 | Wastedump | N→6124 S→2803 |
| 2802 | Troll's Larder | E→2803 |
| 2803 | Troll's Living Room | N→2801 E→2804 S→2805 W→2802 |
| 2804 | The Playpen | W→2803 |
| 2805 | Troll's Bedroom | N→2803 |

