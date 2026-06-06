# Valgarv Cith Deux Headquarters  `(cithdeux)`

[← back to world map](../WORLD-MAP.md) · 18 rooms · vnums 9600–9617

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R9600["A cave entrance<br/>#9600"]
  R9601["Inside the cave<br/>#9601"]
  R9602["Continuing into the cave<br/>#9602"]
  R9603["The transmission chamber<br/>#9603"]
  R9604["The healing chamber<br/>#9604"]
  R9605["A fissure<br/>#9605"]
  R9606["A beam of light!<br/>#9606"]
  R9607["Chamber of the Crystal Dragon<br/>#9607"]
  R9608["a Secret Room<br/>#9608"]
  R9609["Xantos' Ownage Chamber<br/>#9609"]
  R9610["A beam of light!<br/>#9610"]
  R9611["A beam of light!<br/>#9611"]
  R9612["A beam of light!<br/>#9612"]
  R9613["A beam of light!<br/>#9613"]
  R9614["A beam of light!<br/>#9614"]
  R9615["A beam of light!<br/>#9615"]
  R9616["The teleportation pad<br/>#9616"]
  R9617["The Oubliette<br/>#9617"]
  R9600 -->|N| R9608
  R9600 -->|E| R9601
  R9600 -->|S| R9609
  R9600 -->|W| X838
  R9600 -->|D| R9617
  R9601 -->|E| R9602
  R9601 -->|W| R9600
  R9602 -->|N| R9607
  R9602 -->|E| R9603
  R9602 -->|S| R9604
  R9602 -->|W| R9601
  R9602 -->|D| R9605
  R9603 -->|E| R9616
  R9603 -->|W| R9602
  R9604 -->|N| R9602
  R9605 -->|U| R9602
  R9606 -->|E| R9610
  R9606 -->|W| R9616
  R9607 -->|S| R9602
  R9608 -->|S| R9600
  R9609 -->|N| R9600
  R9610 -->|E| R9611
  R9610 -->|W| R9606
  R9611 -->|E| R9612
  R9611 -->|W| R9610
  R9612 -->|E| R9613
  R9612 -->|W| R9611
  R9613 -->|E| R9614
  R9613 -->|W| R9612
  R9614 -->|E| R9615
  R9614 -->|W| R9613
  R9615 -->|W| R9614
  R9616 -->|E| R9606
  R9616 -->|W| R9603
  R9617 -->|U| R9600
  X838["The Nexus of Clans<br/>apoc #838"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 9600 | A cave entrance | N→9608 E→9601 S→9609 W→838 D→9617 |
| 9601 | Inside the cave | E→9602 W→9600 |
| 9602 | Continuing into the cave | N→9607 E→9603 S→9604 W→9601 D→9605 |
| 9603 | The transmission chamber | E→9616 W→9602 |
| 9604 | The healing chamber | N→9602 |
| 9605 | A fissure | U→9602 |
| 9606 | A beam of light! | E→9610 W→9616 |
| 9607 | Chamber of the Crystal Dragon | S→9602 |
| 9608 | a Secret Room | S→9600 |
| 9609 | Xantos' Ownage Chamber | N→9600 |
| 9610 | A beam of light! | E→9611 W→9606 |
| 9611 | A beam of light! | E→9612 W→9610 |
| 9612 | A beam of light! | E→9613 W→9611 |
| 9613 | A beam of light! | E→9614 W→9612 |
| 9614 | A beam of light! | E→9615 W→9613 |
| 9615 | A beam of light! | W→9614 |
| 9616 | The teleportation pad | E→9606 W→9603 |
| 9617 | The Oubliette | U→9600 |

