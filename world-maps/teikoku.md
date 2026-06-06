# Tyu Teikoku Headquarters  `(teikoku)`

[← back to world map](WORLD-MAP.md) · 6 rooms · vnums 9800–9805

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R9800["Entrance of the Teikoku<br/>#9800"]
  R9801["Training Grounds<br/>#9801"]
  R9802["Center of the Circle<br/>#9802"]
  R9803["Hidden Area<br/>#9803"]
  R9804["Room of Healing<br/>#9804"]
  R9805["Watch Tower<br/>#9805"]
  R9800 -->|E| X847
  R9800 -->|S| R9805
  R9800 -->|W| R9801
  R9801 -->|N| R9804
  R9801 -->|E| R9800
  R9801 -->|W| R9802
  R9802 -->|E| R9801
  R9802 -->|S| R9803
  R9803 -->|N| R9802
  R9804 -->|S| R9801
  R9805 -->|N| R9800
  X847["The Nexus of Clans<br/>apoc #847"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 9800 | Entrance of the Teikoku | E→847 S→9805 W→9801 |
| 9801 | Training Grounds | N→9804 E→9800 W→9802 |
| 9802 | Center of the Circle | E→9801 S→9803 |
| 9803 | Hidden Area | N→9802 |
| 9804 | Room of Healing | S→9801 |
| 9805 | Watch Tower | N→9800 |

