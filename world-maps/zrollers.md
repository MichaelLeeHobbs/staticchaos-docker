# Trunker Z-Rollers Headquarters  `(zrollers)`

[← back to world map](../WORLD-MAP.md) · 6 rooms · vnums 9900–9905

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R9900["Entrance to the Mansion<br/>#9900"]
  R9901["The Lounge<br/>#9901"]
  R9902["The Fishtank<br/>#9902"]
  R9903["Observatory<br/>#9903"]
  R9904["A dark passageway<br/>#9904"]
  R9905["A small kitchen<br/>#9905"]
  R9900 -->|N| X849
  R9900 -->|S| R9901
  R9901 -->|N| R9900
  R9901 -->|S| R9902
  R9901 -->|U| R9903
  R9902 -->|N| R9901
  R9902 -->|W| R9904
  R9903 -->|D| R9901
  R9904 -->|E| R9902
  R9904 -->|S| R9905
  R9905 -->|N| R9904
  X849["The Nexus of Clans<br/>apoc #849"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 9900 | Entrance to the Mansion | N→849 S→9901 |
| 9901 | The Lounge | N→9900 S→9902 U→9903 |
| 9902 | The Fishtank | N→9901 W→9904 |
| 9903 | Observatory | D→9901 |
| 9904 | A dark passageway | E→9902 S→9905 |
| 9905 | A small kitchen | N→9904 |

