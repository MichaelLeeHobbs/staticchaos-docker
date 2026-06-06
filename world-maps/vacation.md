# Alathon Pikachu's Vacation  `(vacation)`

[← back to world map](../WORLD-MAP.md) · 20 rooms · vnums 6600–6619

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R6600["On a trail<br/>#6600"]
  R6601["Entrance to the Vacation<br/>#6601"]
  R6602["A happy field<br/>#6602"]
  R6603["A happy field<br/>#6603"]
  R6604["A happy field<br/>#6604"]
  R6605["A happy field<br/>#6605"]
  R6606["A pleasant forest<br/>#6606"]
  R6607["A quiet forest<br/>#6607"]
  R6608["A quiet forest<br/>#6608"]
  R6609["A quiet forest<br/>#6609"]
  R6610["A quiet forest<br/>#6610"]
  R6611["A pristine mountainside<br/>#6611"]
  R6612["A pristine mountainside<br/>#6612"]
  R6613["A pristine mountainside<br/>#6613"]
  R6614["A pristine mountainside<br/>#6614"]
  R6615["A pristine mountainside<br/>#6615"]
  R6616["A sparkling lake<br/>#6616"]
  R6617["A sparkling lake<br/>#6617"]
  R6618["A sparkling lake<br/>#6618"]
  R6619["A sparkling lake<br/>#6619"]
  R6600 -->|N| X6500
  R6600 -->|S| R6601
  R6601 -->|N| R6600
  R6601 -->|S| R6602
  R6602 -->|N| R6601
  R6602 -->|E| R6603
  R6602 -->|S| R6605
  R6602 -->|W| R6606
  R6603 -->|S| R6604
  R6603 -->|W| R6602
  R6604 -->|N| R6603
  R6604 -->|E| R6616
  R6604 -->|W| R6605
  R6605 -->|N| R6602
  R6605 -->|E| R6604
  R6605 -->|W| R6611
  R6606 -->|E| R6602
  R6606 -->|W| R6607
  R6607 -->|N| R6608
  R6607 -->|E| R6606
  R6607 -->|W| R6610
  R6608 -->|S| R6607
  R6608 -->|W| R6609
  R6609 -->|E| R6608
  R6609 -->|S| R6610
  R6610 -->|N| R6609
  R6610 -->|E| R6607
  R6611 -->|E| R6605
  R6611 -->|W| R6612
  R6612 -->|E| R6611
  R6612 -->|S| R6615
  R6612 -->|W| R6613
  R6613 -->|E| R6612
  R6613 -->|S| R6614
  R6614 -->|N| R6613
  R6614 -->|E| R6615
  R6615 -->|N| R6612
  R6615 -->|W| R6614
  R6616 -->|S| R6617
  R6616 -->|W| R6604
  R6617 -->|N| R6616
  R6617 -->|E| R6618
  R6618 -->|S| R6619
  R6618 -->|W| R6617
  R6619 -->|N| R6618
  X6500["Path to Dwarven Village<br/>dwarven #6500"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 6600 | On a trail | N→6500 S→6601 |
| 6601 | Entrance to the Vacation | N→6600 S→6602 |
| 6602 | A happy field | N→6601 E→6603 S→6605 W→6606 |
| 6603 | A happy field | S→6604 W→6602 |
| 6604 | A happy field | N→6603 E→6616 W→6605 |
| 6605 | A happy field | N→6602 E→6604 W→6611 |
| 6606 | A pleasant forest | E→6602 W→6607 |
| 6607 | A quiet forest | N→6608 E→6606 W→6610 |
| 6608 | A quiet forest | S→6607 W→6609 |
| 6609 | A quiet forest | E→6608 S→6610 |
| 6610 | A quiet forest | N→6609 E→6607 |
| 6611 | A pristine mountainside | E→6605 W→6612 |
| 6612 | A pristine mountainside | E→6611 S→6615 W→6613 |
| 6613 | A pristine mountainside | E→6612 S→6614 |
| 6614 | A pristine mountainside | N→6613 E→6615 |
| 6615 | A pristine mountainside | N→6612 W→6614 |
| 6616 | A sparkling lake | S→6617 W→6604 |
| 6617 | A sparkling lake | N→6616 E→6618 |
| 6618 | A sparkling lake | S→6619 W→6617 |
| 6619 | A sparkling lake | N→6618 |

