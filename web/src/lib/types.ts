// Shapes of the prebuilt JSON the tools generate (see repo tools/). Loaded at
// runtime from /data, /docs, /gmcp. Kept intentionally close to the JSON.

export interface AreaInfo {
  id: string;
  file: string;
  name: string;
  roomCount: number;
  vnumMin: number;
  vnumMax: number;
}

export interface Room {
  vnum: number;
  name: string;
  area: string;
  sector: number;
  exits: Record<string, number>; // {"N":305,"S":303,...}
}

export interface World {
  recallVnum: number;
  areas: AreaInfo[];
  rooms: Record<string, Room>;
  stats: Record<string, unknown>;
  reports: Record<string, unknown>;
}

export interface ItemAffect {
  stat: string; // e.g. "+500 HP"
}
export interface ItemSource {
  text?: string;
  kind?: string;
  [k: string]: unknown;
}
export interface Item {
  vnum: number;
  name: string;
  area: string;
  type: string;
  wear: string[];
  value: string;
  flags: string[];
  affects: ItemAffect[];
  weight: number;
  cost: number;
  obtainable: boolean;
  sources: ItemSource[];
}
export interface ItemsFile {
  stats: Record<string, unknown>;
  items: Item[];
}

export interface Mob {
  vnum: number;
  name: string;
  keywords: string;
  level: number;
  behaviour: string[];
  area: string;
  sighting: string;
}
export interface BestiaryFile {
  stats: Record<string, unknown>;
  mobs: Mob[];
}

export interface Shop {
  keeper: number;
  keeperName: string;
  area: string;
  room: number;
  buys: string[];
  profitBuy: number;
  profitSell: number;
  hours: string;
  [k: string]: unknown;
}
export interface ShopsFile {
  stats: Record<string, unknown>;
  shops: Shop[];
}

export interface SpawnItemSource {
  text?: string;
  [k: string]: unknown;
}
export interface SpawnsFile {
  stats: Record<string, unknown>;
  mobSpawns: Record<string, { name: string; level: number | null; rooms: number[] }>;
  itemSources: Record<string, { name: string; type?: string; sources: SpawnItemSource[] }>;
}

export interface ClassInfo {
  name: string;
  prime: string;
  guild: number;
  body: number;
  mind: number;
  spirit: number;
  will: number;
  manaUser: boolean;
}
export interface ClassesFile {
  stats: Record<string, unknown>;
  classNames: string[];
  classes: ClassInfo[];
  skills: Array<Record<string, unknown>>;
}

// Manifests written by scripts/sync-content.mjs
export interface DocEntry {
  file: string; // e.g. "GEARING-GUIDE.md"
  title: string; // first # heading, or prettified filename
  group: string; // "Guides" | "Atlas" | "Reference"
}
export interface GmcpEntry {
  file: string; // e.g. "StaticChaos.xml"
  bytes: number;
  desc: string;
}
