# Dizz Divergent Headquarters  `(divergent)`

[← back to world map](WORLD-MAP.md) · 6 rooms · vnums 9700–9705

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R9700["Divergence<br/>#9700"]
  R9701["Path of Enlightenment<br/>#9701"]
  R9702["A Commencement<br/>#9702"]
  R9703["Atheneum of Wondrous Knowledge<br/>#9703"]
  R9704["Tranquility<br/>#9704"]
  R9705["Harmony<br/>#9705"]
  R9700 -->|E| X845
  R9700 -->|W| R9701
  R9701 -->|E| R9700
  R9701 -->|W| R9702
  R9702 -->|N| R9704
  R9702 -->|E| R9701
  R9702 -->|S| R9703
  R9702 -->|W| R9705
  R9703 -->|N| R9702
  R9704 -->|S| R9702
  R9705 -->|E| R9702
  X845["The Nexus of Clans<br/>apoc #845"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 9700 | Divergence | E→845 W→9701 |
| 9701 | Path of Enlightenment | E→9700 W→9702 |
| 9702 | A Commencement | N→9704 E→9701 S→9703 W→9705 |
| 9703 | [Atheneum of Wondrous Knowledge] | N→9702 |
| 9704 | Tranquility | S→9702 |
| 9705 | Harmony | E→9702 |

