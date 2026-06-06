# Diku Redferne's Residence  `(redferne)`

[← back to world map](../WORLD-MAP.md) · 17 rooms · vnums 7900–7918

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R7900["Outside Redferne's residence<br/>#7900"]
  R7901["The Southern end of the hall<br/>#7901"]
  R7902["Redferne's Library<br/>#7902"]
  R7903["The Artifact room of Naris<br/>#7903"]
  R7904["The Northern end of the hall<br/>#7904"]
  R7905["The Sitting room of Naris<br/>#7905"]
  R7906["The Kitchen of Naris<br/>#7906"]
  R7907["The Larder<br/>#7907"]
  R7908["The Fridge<br/>#7908"]
  R7909["On the stairs<br/>#7909"]
  R7910["The Treasure room<br/>#7910"]
  R7911["Redferne's Bedroom<br/>#7911"]
  R7912["The Balcony of Redferne's Residence<br/>#7912"]
  R7913["The Monster Pen<br/>#7913"]
  R7914["On the Huge Chain<br/>#7914"]
  R7916["On the Great Chain of Naris<br/>#7916"]
  R7918["The Mighty Chain of Naris<br/>#7918"]
  R7900 -->|N| R7901
  R7900 -->|D| R7918
  R7901 -->|N| R7904
  R7901 -->|E| R7910
  R7901 -->|S| R7900
  R7901 -->|W| R7902
  R7901 -->|U| R7909
  R7902 -->|E| R7901
  R7903 -->|W| X0
  R7904 -->|N| R7906
  R7904 -->|E| R7913
  R7904 -->|S| R7901
  R7904 -->|W| R7905
  R7905 -->|E| R7904
  R7906 -->|N| R7907
  R7906 -->|E| R7908
  R7906 -->|S| R7904
  R7907 -->|S| R7906
  R7908 -->|W| R7906
  R7909 -->|U| R7911
  R7909 -->|D| R7901
  R7910 -->|E| X0
  R7910 -->|W| R7901
  R7911 -->|S| R7912
  R7911 -->|D| R7909
  R7912 -->|N| R7911
  R7912 -->|D| X0
  R7913 -->|W| R7904
  R7914 -->|U| R7916
  R7914 -->|D| X3120
  R7916 -->|U| R7918
  R7918 -->|U| R7900
  X0["?? broken<br/>#0"]:::ext
  X3120["Road Crossing<br/>midgaard #3120"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 7900 | Outside Redferne's residence | N→7901 D→7918 |
| 7901 | The Southern end of the hall | N→7904 E→7910 S→7900 W→7902 U→7909 |
| 7902 | Redferne's Library | E→7901 |
| 7903 | The Artifact room of Naris | W→0 |
| 7904 | The Northern end of the hall | N→7906 E→7913 S→7901 W→7905 |
| 7905 | The Sitting room of Naris | E→7904 |
| 7906 | The Kitchen of Naris | N→7907 E→7908 S→7904 |
| 7907 | The Larder | S→7906 |
| 7908 | The Fridge | W→7906 |
| 7909 | On the stairs | U→7911 D→7901 |
| 7910 | The Treasure room | E→0 W→7901 |
| 7911 | Redferne's Bedroom | S→7912 D→7909 |
| 7912 | The Balcony of Redferne's Residence | N→7911 D→0 |
| 7913 | The Monster Pen | W→7904 |
| 7914 | On the Huge Chain | U→7916 D→3120 |
| 7916 | On the Great Chain of Naris | U→7918 |
| 7918 | The Mighty Chain of Naris | U→7900 |

