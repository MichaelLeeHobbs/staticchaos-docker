// Tiny typed loaders for the static content under public/ (synced by
// scripts/sync-content.mjs). No backend — everything is fetch() of a file.

import type {
  World,
  ItemsFile,
  BestiaryFile,
  ShopsFile,
  SpawnsFile,
  ClassesFile,
  DocEntry,
  GmcpEntry,
} from './types';

const BASE = import.meta.env.BASE_URL; // "/" in this deployment

export async function loadJson<T>(rel: string): Promise<T> {
  const res = await fetch(`${BASE}${rel}`);
  if (!res.ok) throw new Error(`Failed to load ${rel}: ${res.status}`);
  return (await res.json()) as T;
}

export async function loadText(rel: string): Promise<string> {
  const res = await fetch(`${BASE}${rel}`);
  if (!res.ok) throw new Error(`Failed to load ${rel}: ${res.status}`);
  return res.text();
}

export const loadWorld = () => loadJson<World>('data/world.json');
export const loadItems = () => loadJson<ItemsFile>('data/items.json');
export const loadBestiary = () => loadJson<BestiaryFile>('data/bestiary.json');
export const loadShops = () => loadJson<ShopsFile>('data/shops.json');
export const loadSpawns = () => loadJson<SpawnsFile>('data/spawns.json');
export const loadClasses = () => loadJson<ClassesFile>('data/classes.json');

export const loadDocsIndex = () => loadJson<DocEntry[]>('docs/index.json');
export const loadDoc = (file: string) => loadText(`docs/${file}`);

export const loadGmcpIndex = () => loadJson<GmcpEntry[]>('gmcp/index.json');
export const gmcpUrl = (file: string) => `${BASE}gmcp/${file}`;
