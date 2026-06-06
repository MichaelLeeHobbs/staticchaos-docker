# Malucif The Renegades Headquarters  `(renegades)`

[← back to world map](../WORLD-MAP.md) · 11 rooms · vnums 9950–9960

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R9950["Slaughterhouse<br/>#9950"]
  R9951["Dojo<br/>#9951"]
  R9952["Sanctuary<br/>#9952"]
  R9953["QuestPalace<br/>#9953"]
  R9954["Bastille<br/>#9954"]
  R9955["Regen Room<br/>#9955"]
  R9956["Shelter<br/>#9956"]
  R9957["The Plains<br/>#9957"]
  R9958["The Tundra<br/>#9958"]
  R9959["Chaos<br/>#9959"]
  R9960["Memorial<br/>#9960"]
  R9950 -->|N| X851
  R9950 -->|S| R9951
  R9951 -->|N| R9950
  R9951 -->|E| R9953
  R9951 -->|S| R9952
  R9951 -->|W| R9955
  R9951 -->|D| R9959
  R9952 -->|N| R9951
  R9952 -->|S| R9954
  R9953 -->|W| R9951
  R9954 -->|N| R9952
  R9954 -->|S| R9957
  R9954 -->|U| R9956
  R9955 -->|E| R9951
  R9956 -->|E| R9960
  R9956 -->|D| R9954
  R9957 -->|N| R9954
  R9957 -->|S| R9958
  R9958 -->|N| R9957
  R9959 -->|U| R9951
  R9960 -->|W| R9956
  X851["The Nexus of Clans<br/>apoc #851"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 9950 | Slaughterhouse | N→851 S→9951 |
| 9951 | Dojo | N→9950 E→9953 S→9952 W→9955 D→9959 |
| 9952 | Sanctuary | N→9951 S→9954 |
| 9953 | QuestPalace | W→9951 |
| 9954 | Bastille | N→9952 S→9957 U→9956 |
| 9955 | Regen Room | E→9951 |
| 9956 | Shelter | E→9960 D→9954 |
| 9957 | The Plains | N→9954 S→9958 |
| 9958 | The Tundra | N→9957 |
| 9959 | Chaos | U→9951 |
| 9960 | Memorial | W→9956 |

