# Generic Old Marsh  `(marsh)`

[← back to world map](WORLD-MAP.md) · 18 rooms · vnums 8301–8318

Grey dashed nodes leave the area; green dashed nodes (`▸ Part X`) continue on another sub-map below.

## Map

```mermaid
graph LR
  R8301["Muddy Path<br/>#8301"]
  R8302["Swamp's Edge<br/>#8302"]
  R8303["Marshy Forest<br/>#8303"]
  R8304["On a hill.<br/>#8304"]
  R8305["An Oozing Bog<br/>#8305"]
  R8306["An Oozing Bog<br/>#8306"]
  R8307["Swamp Thing's Lair<br/>#8307"]
  R8308["Pool of Quick Sand<br/>#8308"]
  R8309["A Dark Pool.<br/>#8309"]
  R8310["Gloomy Path Through the Marsh.<br/>#8310"]
  R8311["Cloud of Darkness.<br/>#8311"]
  R8312["Near Vegetation from Hell.<br/>#8312"]
  R8313["By the Monolith.<br/>#8313"]
  R8314["Murky Bog.<br/>#8314"]
  R8315["Northern Lake Side.<br/>#8315"]
  R8316["Southern Lake Side.<br/>#8316"]
  R8317["Beach<br/>#8317"]
  R8318["Before the Great Gates.<br/>#8318"]
  R8301 -->|N| X6125
  R8301 -->|S| R8302
  R8302 -->|N| R8301
  R8302 -->|E| R8303
  R8302 -->|S| R8304
  R8303 -->|E| R8305
  R8303 -->|W| R8302
  R8304 -->|N| R8302
  R8304 -->|E| R8309
  R8304 -->|S| R8310
  R8305 -->|E| R8306
  R8305 -->|W| R8303
  R8306 -->|W| R8305
  R8307 -->|W| R8308
  R8308 -->|E| R8307
  R8308 -->|S| R8314
  R8308 -->|W| R8309
  R8309 -->|E| R8308
  R8309 -->|S| R8311
  R8309 -->|W| R8304
  R8310 -->|N| R8304
  R8310 -->|S| R8312
  R8311 -->|N| R8309
  R8311 -->|S| R8313
  R8312 -->|N| R8310
  R8312 -->|E| R8313
  R8313 -->|N| R8311
  R8313 -->|E| R8314
  R8313 -->|W| R8312
  R8314 -->|N| R8308
  R8314 -->|E| R8315
  R8314 -->|W| R8313
  R8315 -->|S| R8316
  R8315 -->|W| R8314
  R8316 -->|N| R8315
  R8316 -->|W| R8317
  R8317 -->|E| R8316
  R8317 -->|W| R8318
  R8318 -->|E| R8317
  R8318 -->|S| X0
  X6125["A small path on the river bank in the deep, dark<br/>haon #6125"]:::ext
  X0["?? broken<br/>#0"]:::ext
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
  classDef part fill:#16313a,stroke:#5aa,color:#bfe,stroke-dasharray:4 2;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 8301 | Muddy Path | N→6125 S→8302 |
| 8302 | Swamp's Edge | N→8301 E→8303 S→8304 |
| 8303 | Marshy Forest | E→8305 W→8302 |
| 8304 | On a hill. | N→8302 E→8309 S→8310 |
| 8305 | An Oozing Bog | E→8306 W→8303 |
| 8306 | An Oozing Bog | W→8305 |
| 8307 | Swamp Thing's Lair | W→8308 |
| 8308 | Pool of Quick Sand | E→8307 S→8314 W→8309 |
| 8309 | A Dark Pool. | E→8308 S→8311 W→8304 |
| 8310 | Gloomy Path Through the Marsh. | N→8304 S→8312 |
| 8311 | Cloud of Darkness. | N→8309 S→8313 |
| 8312 | Near Vegetation from Hell. | N→8310 E→8313 |
| 8313 | By the Monolith. | N→8311 E→8314 W→8312 |
| 8314 | Murky Bog. | N→8308 E→8315 W→8313 |
| 8315 | Northern Lake Side. | S→8316 W→8314 |
| 8316 | Southern Lake Side. | N→8315 W→8317 |
| 8317 | Beach | E→8316 W→8318 |
| 8318 | Before the Great Gates. | E→8317 S→0 |

