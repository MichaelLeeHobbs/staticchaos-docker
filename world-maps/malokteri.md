# Blah Malokteri Headquarters  `(malokteri)`

[← back to world map](../WORLD-MAP.md) · 13 rooms · vnums 9500–9512

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R9500["The Vestibule<br/>#9500"]
  R9501["The Avenue of the Malokteri<br/>#9501"]
  R9502["Continuing along the Avenue<br/>#9502"]
  R9503["Tresspasser's Chamber<br/>#9503"]
  R9504["Malokteri Mess Hall<br/>#9504"]
  R9505["Dragon's Lair<br/>#9505"]
  R9506["Malokteri's Treasure Room<br/>#9506"]
  R9507["The Alleyway<br/>#9507"]
  R9508["The Alleyway<br/>#9508"]
  R9509["A second story shop<br/>#9509"]
  R9510["A Padded Room<br/>#9510"]
  R9511["Malokteri Infirmary<br/>#9511"]
  R9512["Room of War and Flames<br/>#9512"]
  R9500 -->|N| R9501
  R9500 -->|E| R9503
  R9500 -->|S| X842
  R9500 -->|W| R9504
  R9500 -->|U| R9505
  R9500 -->|D| R9506
  R9501 -->|N| R9502
  R9501 -->|E| R9507
  R9501 -->|S| R9500
  R9501 -->|W| R9508
  R9501 -->|U| R9509
  R9502 -->|N| R9510
  R9502 -->|E| R9511
  R9502 -->|S| R9501
  R9502 -->|W| R9512
  R9503 -->|W| R9500
  R9504 -->|E| R9500
  R9505 -->|D| R9500
  R9506 -->|U| R9500
  R9507 -->|W| R9501
  R9508 -->|E| R9501
  R9509 -->|D| R9501
  R9510 -->|S| R9502
  R9511 -->|W| R9502
  R9512 -->|E| R9502
  X842["The Nexus of Clans<br/>apoc #842"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 9500 | The Vestibule | N→9501 E→9503 S→842 W→9504 U→9505 D→9506 |
| 9501 | The Avenue of the Malokteri | N→9502 E→9507 S→9500 W→9508 U→9509 |
| 9502 | Continuing along the Avenue | N→9510 E→9511 S→9501 W→9512 |
| 9503 | Tresspasser's Chamber | W→9500 |
| 9504 | Malokteri Mess Hall | E→9500 |
| 9505 | Dragon's Lair | D→9500 |
| 9506 | Malokteri's Treasure Room | U→9500 |
| 9507 | The Alleyway | W→9501 |
| 9508 | The Alleyway | E→9501 |
| 9509 | A second story shop | D→9501 |
| 9510 | A Padded Room | S→9502 |
| 9511 | Malokteri Infirmary | W→9502 |
| 9512 | Room of War and Flames | E→9502 |

