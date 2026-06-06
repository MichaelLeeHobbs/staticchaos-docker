# Anon Drow City  `(drow)`

[← back to world map](../WORLD-MAP.md) · 51 rooms · vnums 5100–5150

Dashed nodes are exits that leave this area.

```mermaid
graph LR
  R5100["City Entrance<br/>#5100"]
  R5101["City street<br/>#5101"]
  R5102["City street<br/>#5102"]
  R5103["City street<br/>#5103"]
  R5104["3rd House<br/>#5104"]
  R5105["Throne Room<br/>#5105"]
  R5106["City street<br/>#5106"]
  R5107["City street<br/>#5107"]
  R5108["2nd House<br/>#5108"]
  R5109["Throne Room<br/>#5109"]
  R5110["Main Gate<br/>#5110"]
  R5111["1st House<br/>#5111"]
  R5112["Throne Room<br/>#5112"]
  R5113["Main Chamber<br/>#5113"]
  R5114["City street<br/>#5114"]
  R5115["City street<br/>#5115"]
  R5116["Cleric Academy<br/>#5116"]
  R5117["City street<br/>#5117"]
  R5118["Warrior's Academy<br/>#5118"]
  R5119["Entrance to the Temple of Lloth<br/>#5119"]
  R5120["Mage's Academy<br/>#5120"]
  R5121["City street<br/>#5121"]
  R5122["Slave Chamber<br/>#5122"]
  R5123["City street<br/>#5123"]
  R5124["4th house<br/>#5124"]
  R5125["Throne Room<br/>#5125"]
  R5126["Entrance Way<br/>#5126"]
  R5127["Hallway<br/>#5127"]
  R5128["Long Hallway<br/>#5128"]
  R5129["Long Hallway<br/>#5129"]
  R5130["Long Hallway<br/>#5130"]
  R5131["Long Hallway<br/>#5131"]
  R5132["Long Hallway<br/>#5132"]
  R5133["Warrior's Barracks<br/>#5133"]
  R5134["Long Hallway<br/>#5134"]
  R5135["Grand Stairway<br/>#5135"]
  R5136["Grand Hallway<br/>#5136"]
  R5137["Mage's Barracks<br/>#5137"]
  R5138["Grand Hallway<br/>#5138"]
  R5139["Cleric's Barracks<br/>#5139"]
  R5140["Grand Stairway<br/>#5140"]
  R5141["Main Chamber<br/>#5141"]
  R5142["Eastern side of Chamber<br/>#5142"]
  R5143["Sacrificial Pit<br/>#5143"]
  R5144["Western side of Chamber<br/>#5144"]
  R5145["The Altar<br/>#5145"]
  R5146["Slave Cells<br/>#5146"]
  R5147["Slave Pen<br/>#5147"]
  R5148["Dais<br/>#5148"]
  R5149["The Treasury<br/>#5149"]
  R5150["Weaponsmaster's Chamber<br/>#5150"]
  R5100 -->|W| R5101
  R5100 -->|U| X5270
  R5101 -->|N| R5123
  R5101 -->|E| R5100
  R5101 -->|S| R5102
  R5101 -->|W| R5122
  R5102 -->|N| R5101
  R5102 -->|W| R5103
  R5103 -->|E| R5102
  R5103 -->|S| R5104
  R5103 -->|W| R5106
  R5104 -->|N| R5103
  R5104 -->|S| R5105
  R5105 -->|N| R5104
  R5106 -->|N| R5118
  R5106 -->|E| R5103
  R5106 -->|W| R5107
  R5107 -->|N| R5110
  R5107 -->|E| R5106
  R5107 -->|S| R5108
  R5108 -->|N| R5107
  R5108 -->|W| R5109
  R5109 -->|E| R5108
  R5110 -->|N| R5114
  R5110 -->|S| R5107
  R5110 -->|W| R5111
  R5111 -->|E| R5110
  R5111 -->|W| R5112
  R5112 -->|N| R5113
  R5112 -->|E| R5111
  R5113 -->|S| R5112
  R5114 -->|N| R5115
  R5114 -->|E| R5117
  R5114 -->|S| R5110
  R5115 -->|S| R5114
  R5115 -->|W| R5116
  R5116 -->|E| R5115
  R5117 -->|N| R5119
  R5117 -->|E| R5121
  R5117 -->|W| R5114
  R5118 -->|S| R5106
  R5119 -->|S| R5117
  R5119 -->|D| R5126
  R5120 -->|S| R5121
  R5121 -->|N| R5120
  R5121 -->|E| R5123
  R5121 -->|S| R5122
  R5121 -->|W| R5117
  R5122 -->|N| R5121
  R5122 -->|E| R5101
  R5123 -->|N| R5124
  R5123 -->|S| R5101
  R5123 -->|W| R5121
  R5124 -->|E| R5125
  R5124 -->|S| R5123
  R5125 -->|W| R5124
  R5126 -->|N| R5127
  R5126 -->|U| R5119
  R5127 -->|N| R5135
  R5127 -->|E| R5128
  R5127 -->|S| R5126
  R5127 -->|W| R5131
  R5128 -->|E| R5129
  R5128 -->|W| R5127
  R5129 -->|N| R5150
  R5129 -->|E| R5130
  R5129 -->|W| R5128
  R5130 -->|E| R5134
  R5130 -->|W| R5129
  R5131 -->|E| R5127
  R5131 -->|W| R5132
  R5132 -->|E| R5131
  R5132 -->|S| R5133
  R5132 -->|W| R5134
  R5133 -->|N| R5132
  R5134 -->|E| R5132
  R5134 -->|W| R5130
  R5135 -->|S| R5127
  R5135 -->|U| R5136
  R5136 -->|N| R5138
  R5136 -->|E| R5137
  R5136 -->|D| R5135
  R5137 -->|W| R5136
  R5138 -->|S| R5136
  R5138 -->|W| R5139
  R5138 -->|D| R5140
  R5139 -->|E| R5138
  R5140 -->|N| R5141
  R5140 -->|U| R5138
  R5141 -->|E| R5142
  R5141 -->|S| R5140
  R5141 -->|W| R5144
  R5142 -->|N| R5145
  R5142 -->|E| R5146
  R5142 -->|S| R5141
  R5144 -->|N| R5145
  R5144 -->|S| R5141
  R5145 -->|N| R5148
  R5145 -->|E| R5142
  R5145 -->|W| R5144
  R5145 -->|D| R5143
  R5146 -->|N| R5147
  R5146 -->|W| R5142
  R5147 -->|S| R5146
  R5148 -->|S| R5145
  R5148 -->|W| R5149
  R5149 -->|E| R5148
  R5150 -->|S| R5129
  X5270["Down the path into a dead end<br/>thalos #5270"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 5100 | City Entrance | W→5101 U→5270 |
| 5101 | City street | N→5123 E→5100 S→5102 W→5122 |
| 5102 | City street | N→5101 W→5103 |
| 5103 | City street | E→5102 S→5104 W→5106 |
| 5104 | 3rd House | N→5103 S→5105 |
| 5105 | Throne Room | N→5104 |
| 5106 | City street | N→5118 E→5103 W→5107 |
| 5107 | City street | N→5110 E→5106 S→5108 |
| 5108 | 2nd House | N→5107 W→5109 |
| 5109 | Throne Room | E→5108 |
| 5110 | Main Gate | N→5114 S→5107 W→5111 |
| 5111 | 1st House | E→5110 W→5112 |
| 5112 | Throne Room | N→5113 E→5111 |
| 5113 | Main Chamber | S→5112 |
| 5114 | City street | N→5115 E→5117 S→5110 |
| 5115 | City street | S→5114 W→5116 |
| 5116 | Cleric Academy | E→5115 |
| 5117 | City street | N→5119 E→5121 W→5114 |
| 5118 | Warrior's Academy | S→5106 |
| 5119 | Entrance to the Temple of Lloth | S→5117 D→5126 |
| 5120 | Mage's Academy | S→5121 |
| 5121 | City street | N→5120 E→5123 S→5122 W→5117 |
| 5122 | Slave Chamber | N→5121 E→5101 |
| 5123 | City street | N→5124 S→5101 W→5121 |
| 5124 | 4th house | E→5125 S→5123 |
| 5125 | Throne Room | W→5124 |
| 5126 | Entrance Way | N→5127 U→5119 |
| 5127 | Hallway | N→5135 E→5128 S→5126 W→5131 |
| 5128 | Long Hallway | E→5129 W→5127 |
| 5129 | Long Hallway | N→5150 E→5130 W→5128 |
| 5130 | Long Hallway | E→5134 W→5129 |
| 5131 | Long Hallway | E→5127 W→5132 |
| 5132 | Long Hallway | E→5131 S→5133 W→5134 |
| 5133 | Warrior's Barracks | N→5132 |
| 5134 | Long Hallway | E→5132 W→5130 |
| 5135 | Grand Stairway | S→5127 U→5136 |
| 5136 | Grand Hallway | N→5138 E→5137 D→5135 |
| 5137 | Mage's Barracks | W→5136 |
| 5138 | Grand Hallway | S→5136 W→5139 D→5140 |
| 5139 | Cleric's Barracks | E→5138 |
| 5140 | Grand Stairway | N→5141 U→5138 |
| 5141 | Main Chamber | E→5142 S→5140 W→5144 |
| 5142 | Eastern side of Chamber | N→5145 E→5146 S→5141 |
| 5143 | Sacrificial Pit | — |
| 5144 | Western side of Chamber | N→5145 S→5141 |
| 5145 | The Altar | N→5148 E→5142 W→5144 D→5143 |
| 5146 | Slave Cells | N→5147 W→5142 |
| 5147 | Slave Pen | S→5146 |
| 5148 | Dais | S→5145 W→5149 |
| 5149 | The Treasury | E→5148 |
| 5150 | Weaponsmaster's Chamber | S→5129 |

