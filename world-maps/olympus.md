# Generic Olympus  `(olympus)`

[← back to world map](WORLD-MAP.md) · 50 rooms · vnums 901–954

Grey dashed nodes leave the area; green dashed nodes (`▸ Part X`) continue on another sub-map below.

_This area is split into 4 sub-maps for legibility._

## Map — Part A (24 rooms: #901–#924)

```mermaid
graph LR
  R901["Mountain<br/>#901"]
  R902["Mountain<br/>#902"]
  R903["Mountain<br/>#903"]
  R904["Entrance<br/>#904"]
  R905["Zeus Street<br/>#905"]
  R906["Zeus Street<br/>#906"]
  R907["Zeus Street<br/>#907"]
  R908["Zeus Street<br/>#908"]
  R909["Castle Entryway<br/>#909"]
  R910["Throne Room<br/>#910"]
  R911["Western Wing<br/>#911"]
  R912["Western Wing<br/>#912"]
  R913["Western Wing<br/>#913"]
  R914["Kitchen<br/>#914"]
  R915["Eastern Wing<br/>#915"]
  R916["Eastern Wing<br/>#916"]
  R917["Eastern Wing<br/>#917"]
  R918["Dungeon<br/>#918"]
  R919["Dungeon<br/>#919"]
  R920["Cell<br/>#920"]
  R921["Cell<br/>#921"]
  R922["Cell<br/>#922"]
  R923["Stairway<br/>#923"]
  R924["Bakery<br/>#924"]
  R901 -->|S| X324
  R901 -->|U| R902
  R902 -->|U| R903
  R902 -->|D| R901
  R903 -->|N| R904
  R903 -->|D| R902
  R904 -->|N| R905
  R904 -->|S| R903
  R905 -->|N| R906
  R905 -->|S| R904
  R905 -->|W| R924
  R906 -->|N| R907
  R906 -->|E| X925
  R906 -->|S| R905
  R906 -->|W| X926
  R907 -->|N| R908
  R907 -->|S| R906
  R907 -->|W| X927
  R908 -->|N| R909
  R908 -->|E| X928
  R908 -->|S| R907
  R909 -->|N| R910
  R909 -->|E| R915
  R909 -->|S| R908
  R909 -->|W| R911
  R910 -->|N| R923
  R910 -->|S| R909
  R910 -->|D| R918
  R911 -->|E| R909
  R911 -->|W| R912
  R912 -->|N| R913
  R912 -->|E| R911
  R912 -->|S| X930
  R912 -->|W| X929
  R913 -->|N| R914
  R913 -->|S| R912
  R914 -->|S| R913
  R915 -->|E| R916
  R915 -->|W| R909
  R916 -->|N| R917
  R916 -->|E| X931
  R916 -->|S| X932
  R916 -->|W| R915
  R917 -->|N| X933
  R917 -->|S| R916
  R918 -->|N| R919
  R918 -->|U| R910
  R919 -->|N| R920
  R919 -->|E| R922
  R919 -->|S| R918
  R919 -->|W| R921
  R920 -->|S| R919
  R921 -->|E| R919
  R922 -->|W| R919
  R923 -->|S| R910
  R923 -->|U| X939
  R924 -->|E| R905
  X324["The steep foothills<br/>plains #324"]:::ext
  X925["▸ Part B: Residence<br/>#925"]:::part
  X926["▸ Part B: Residence<br/>#926"]:::part
  X927["▸ Part B: Mystic Shop<br/>#927"]:::part
  X928["▸ Part C: Ares Street<br/>#928"]:::part
  X930["▸ Part B: Storage Room<br/>#930"]:::part
  X929["▸ Part B: Guest Quarters<br/>#929"]:::part
  X931["▸ Part B: Guest Quarters<br/>#931"]:::part
  X932["▸ Part B: Storage Room<br/>#932"]:::part
  X933["▸ Part B: Stable<br/>#933"]:::part
  X939["▸ Part D: Second Floor<br/>#939"]:::part
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
  classDef part fill:#16313a,stroke:#5aa,color:#bfe,stroke-dasharray:4 2;
```

## Map — Part B (8 rooms: #925–#933)

```mermaid
graph LR
  R925["Residence<br/>#925"]
  R926["Residence<br/>#926"]
  R927["Mystic Shop<br/>#927"]
  R929["Guest Quarters<br/>#929"]
  R930["Storage Room<br/>#930"]
  R931["Guest Quarters<br/>#931"]
  R932["Storage Room<br/>#932"]
  R933["Stable<br/>#933"]
  R925 -->|W| X906
  R926 -->|E| X906
  R927 -->|E| X907
  R929 -->|E| X912
  R930 -->|N| X912
  R931 -->|W| X916
  R932 -->|N| X916
  R933 -->|S| X917
  X906["▸ Part A: Zeus Street<br/>#906"]:::part
  X907["▸ Part A: Zeus Street<br/>#907"]:::part
  X912["▸ Part A: Western Wing<br/>#912"]:::part
  X916["▸ Part A: Eastern Wing<br/>#916"]:::part
  X917["▸ Part A: Eastern Wing<br/>#917"]:::part
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
  classDef part fill:#16313a,stroke:#5aa,color:#bfe,stroke-dasharray:4 2;
```

## Map — Part C (7 rooms: #928–#954)

```mermaid
graph LR
  R928["Ares Street<br/>#928"]
  R934["Ares Street<br/>#934"]
  R935["Armoury<br/>#935"]
  R936["Weapon Shop<br/>#936"]
  R937["Smithy<br/>#937"]
  R938["The Planning Room<br/>#938"]
  R954["The Believer<br/>#954"]
  R928 -->|E| R934
  R928 -->|W| X908
  R934 -->|N| R937
  R934 -->|E| R936
  R934 -->|S| R935
  R934 -->|W| R928
  R935 -->|N| R934
  R936 -->|W| R934
  R937 -->|S| R934
  R937 -->|W| R938
  R938 -->|N| R954
  R938 -->|E| R937
  R954 -->|S| R938
  X908["▸ Part A: Zeus Street<br/>#908"]:::part
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
  classDef part fill:#16313a,stroke:#5aa,color:#bfe,stroke-dasharray:4 2;
```

## Map — Part D (11 rooms: #939–#951)

```mermaid
graph LR
  R939["Second Floor<br/>#939"]
  R940["Northern Hall<br/>#940"]
  R941["Southern Hall<br/>#941"]
  R942["Western Hall<br/>#942"]
  R943["Eastern Hall<br/>#943"]
  R944["Bed Room<br/>#944"]
  R945["Bed Room<br/>#945"]
  R946["Bed Room<br/>#946"]
  R947["Bed Room<br/>#947"]
  R948["Hall of Gods<br/>#948"]
  R951["The Portal<br/>#951"]
  R939 -->|N| R940
  R939 -->|E| R943
  R939 -->|S| R941
  R939 -->|W| R942
  R939 -->|U| R948
  R939 -->|D| X923
  R940 -->|N| R947
  R940 -->|S| R939
  R941 -->|N| R939
  R941 -->|S| R944
  R942 -->|E| R939
  R942 -->|W| R945
  R943 -->|E| R946
  R943 -->|W| R939
  R944 -->|N| R941
  R945 -->|E| R942
  R946 -->|W| R943
  R947 -->|S| R940
  R948 -->|N| R951
  R948 -->|D| R939
  R951 -->|S| R948
  X923["▸ Part A: Stairway<br/>#923"]:::part
  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;
  classDef part fill:#16313a,stroke:#5aa,color:#bfe,stroke-dasharray:4 2;
```

## Rooms

| VNUM | Name | Exits |
|---:|---|---|
| 901 | Mountain | S→324 U→902 |
| 902 | Mountain | U→903 D→901 |
| 903 | Mountain | N→904 D→902 |
| 904 | Entrance | N→905 S→903 |
| 905 | Zeus Street | N→906 S→904 W→924 |
| 906 | Zeus Street | N→907 E→925 S→905 W→926 |
| 907 | Zeus Street | N→908 S→906 W→927 |
| 908 | Zeus Street | N→909 E→928 S→907 |
| 909 | Castle Entryway | N→910 E→915 S→908 W→911 |
| 910 | Throne Room | N→923 S→909 D→918 |
| 911 | Western Wing | E→909 W→912 |
| 912 | Western Wing | N→913 E→911 S→930 W→929 |
| 913 | Western Wing | N→914 S→912 |
| 914 | Kitchen | S→913 |
| 915 | Eastern Wing | E→916 W→909 |
| 916 | Eastern Wing | N→917 E→931 S→932 W→915 |
| 917 | Eastern Wing | N→933 S→916 |
| 918 | Dungeon | N→919 U→910 |
| 919 | Dungeon | N→920 E→922 S→918 W→921 |
| 920 | Cell | S→919 |
| 921 | Cell | E→919 |
| 922 | Cell | W→919 |
| 923 | Stairway | S→910 U→939 |
| 924 | Bakery | E→905 |
| 925 | Residence | W→906 |
| 926 | Residence | E→906 |
| 927 | Mystic Shop | E→907 |
| 928 | Ares Street | E→934 W→908 |
| 929 | Guest Quarters | E→912 |
| 930 | Storage Room | N→912 |
| 931 | Guest Quarters | W→916 |
| 932 | Storage Room | N→916 |
| 933 | Stable | S→917 |
| 934 | Ares Street | N→937 E→936 S→935 W→928 |
| 935 | Armoury | N→934 |
| 936 | Weapon Shop | W→934 |
| 937 | Smithy | S→934 W→938 |
| 938 | The Planning Room | N→954 E→937 |
| 939 | Second Floor | N→940 E→943 S→941 W→942 U→948 D→923 |
| 940 | Northern Hall | N→947 S→939 |
| 941 | Southern Hall | N→939 S→944 |
| 942 | Western Hall | E→939 W→945 |
| 943 | Eastern Hall | E→946 W→939 |
| 944 | Bed Room | N→941 |
| 945 | Bed Room | E→942 |
| 946 | Bed Room | W→943 |
| 947 | Bed Room | S→940 |
| 948 | Hall of Gods | N→951 D→939 |
| 951 | The Portal | S→948 |
| 954 | The Believer | S→938 |

