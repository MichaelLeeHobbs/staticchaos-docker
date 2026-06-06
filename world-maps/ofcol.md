# Alfa Ofcol  `(ofcol)`

[← back to world map](WORLD-MAP.md) · 8 rooms · vnums 5550–5577

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R5550["Village street<br/>#5550"]
  R5551["Village street<br/>#5551"]
  R5552["Village square<br/>#5552"]
  R5553["The small alley<br/>#5553"]
  R5554["The future training room<br/>#5554"]
  R5555["Luxan's mixed shop<br/>#5555"]
  R5556["The Local Inn<br/>#5556"]
  R5577["Ravan's hideout<br/>#5577"]
  R5550 -->|N| R5551
  R5550 -->|S| X314
  R5551 -->|N| R5552
  R5551 -->|E| R5553
  R5551 -->|S| R5550
  R5552 -->|N| R5556
  R5552 -->|S| R5551
  R5552 -->|W| R5555
  R5553 -->|E| X601
  R5553 -->|S| R5554
  R5553 -->|W| R5551
  R5554 -->|N| R5553
  R5555 -->|E| R5552
  R5556 -->|S| R5552
  X314["Outside Ofcol<br/>plains #314"]:::ext
  X601["The Big Intersection<br/>ofcol2 #601"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 5550 | Village street | N→5551 S→314 |
| 5551 | Village street | N→5552 E→5553 S→5550 |
| 5552 | Village square | N→5556 S→5551 W→5555 |
| 5553 | The small alley | E→601 S→5554 W→5551 |
| 5554 | The future training room | N→5553 |
| 5555 | Luxan's mixed shop | E→5552 |
| 5556 | The Local Inn | S→5552 |
| 5577 | Ravan's hideout | — |

