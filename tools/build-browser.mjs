#!/usr/bin/env node
/*
 * build-browser.mjs -- self-contained searchable world browser.
 *
 * Bundles world.json + items.json + bestiary.json + spawns.json into a single
 * world-maps/world-browser.html (data embedded inline, so it opens straight
 * from disk -- no server, no network). Search rooms / items / creatures,
 * click through exits, and see what spawns/drops where.
 *
 * Usage:  node tools/build-browser.mjs   (run build:data first)
 */

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const OUT = path.join(ROOT, 'world-maps');
const readJson = (f) => { try { return JSON.parse(fs.readFileSync(path.join(OUT, f), 'utf8')); } catch { return null; } };

function main() {
  const world = readJson('world.json');
  const items = readJson('items.json');
  const bestiary = readJson('bestiary.json');
  const spawns = readJson('spawns.json');
  if (!world) { console.error('world.json missing -- run build:world first.'); process.exit(1); }

  // slim, link-ready bundle
  const data = {
    rooms: world.rooms,                                   // vnum -> {vnum,name,area,sector,exits}
    items: Object.fromEntries((items?.items || []).map((o) => [o.vnum, o])),
    mobs: Object.fromEntries((bestiary?.mobs || []).map((m) => [m.vnum, m])),
    mobSpawns: spawns?.mobSpawns || {},                   // vnum -> {name,level,rooms[]}
    itemSources: spawns?.itemSources || {},               // vnum -> {name,type,sources[]}
  };

  const html = `<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Static Chaos -- World Browser</title>
<style>
  :root{color-scheme:dark}
  *{box-sizing:border-box}
  body{margin:0;font:14px/1.5 system-ui,Segoe UI,sans-serif;background:#14171c;color:#d7dce3}
  header{padding:12px 16px;background:#0f1216;border-bottom:1px solid #2a2f37;position:sticky;top:0;z-index:2}
  h1{font-size:16px;margin:0 0 8px}
  #q{width:100%;max-width:520px;padding:8px 10px;font-size:15px;background:#1c2128;border:1px solid #2a2f37;border-radius:6px;color:#fff}
  .hint{color:#7c8696;font-size:12px;margin-left:6px}
  main{display:grid;grid-template-columns:340px 1fr;gap:0;height:calc(100vh - 86px)}
  #list{overflow:auto;border-right:1px solid #2a2f37}
  #detail{overflow:auto;padding:16px 20px}
  .row{padding:7px 12px;border-bottom:1px solid #20242b;cursor:pointer;display:flex;gap:8px;align-items:baseline}
  .row:hover{background:#1c2128}
  .tag{font-size:10px;text-transform:uppercase;letter-spacing:.04em;padding:1px 6px;border-radius:10px;flex:none}
  .room{background:#1b3a5b;color:#8fc1ff}.item{background:#3a2f12;color:#ffd479}.mob{background:#3a1b2b;color:#ff9ec4}
  .rn{color:#9aa4b2;font-variant-numeric:tabular-nums;flex:none}
  .nm{overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
  a{color:#8fc1ff;cursor:pointer;text-decoration:none}a:hover{text-decoration:underline}
  h2{font-size:18px;margin:0 0 4px}.sub{color:#7c8696;margin:0 0 14px}
  table{border-collapse:collapse;margin:8px 0;width:100%}td,th{border:1px solid #2a2f37;padding:4px 8px;text-align:left;vertical-align:top}
  th{background:#1c2128;font-weight:600}.k{color:#7c8696;font-size:12px;margin:14px 0 4px;text-transform:uppercase;letter-spacing:.05em}
  .pill{display:inline-block;background:#1c2128;border:1px solid #2a2f37;border-radius:5px;padding:1px 7px;margin:2px 4px 2px 0}
  .exits a{display:inline-block;background:#16263a;border:1px solid #244;border-radius:5px;padding:2px 8px;margin:2px 4px 2px 0}
</style></head><body>
<header><h1>Static Chaos — World Browser <span class="hint">search rooms, items &amp; creatures · click to explore · all data embedded, works offline</span></h1>
<input id="q" placeholder="Search by name or vnum… (try: tabard, larsen, nexus, 800)" autofocus></header>
<main><div id="list"></div><div id="detail"><p class="sub">Type to search, or click a result. ${data.world ? '' : ''}Everything below is read straight from the live area files.</p></div></main>
<script>
const W = ${JSON.stringify(data)};
// invert spawns for room views
const roomMobs = {}, roomItems = {};
for (const [v, s] of Object.entries(W.mobSpawns)) for (const r of s.rooms) (roomMobs[r] ??= []).push(+v);
for (const [v, s] of Object.entries(W.itemSources)) for (const src of s.sources) if (src.room) (roomItems[src.room] ??= []).push(+v);
const esc = (s) => String(s ?? '').replace(/[&<>]/g, (c) => ({'&':'&amp;','<':'&lt;','>':'&gt;'}[c]));
const DIRW = {N:'north',E:'east',S:'south',W:'west',U:'up',D:'down'};

// build search index
const idx = [];
for (const r of Object.values(W.rooms)) idx.push({kind:'room', id:r.vnum, name:r.name, area:r.area, hay:(r.name+' '+r.vnum).toLowerCase()});
for (const o of Object.values(W.items)) idx.push({kind:'item', id:o.vnum, name:o.name, area:o.area, hay:(o.name+' '+o.vnum).toLowerCase()});
for (const m of Object.values(W.mobs)) idx.push({kind:'mob', id:m.vnum, name:m.name, area:m.area, hay:(m.name+' '+m.vnum).toLowerCase()});

const listEl = document.getElementById('list'), detEl = document.getElementById('detail'), qEl = document.getElementById('q');
function search(q){
  q = q.trim().toLowerCase();
  const res = !q ? [] : idx.filter(x => x.hay.includes(q)).slice(0, 300);
  listEl.innerHTML = res.map(x =>
    '<div class="row" onclick="show(\\''+x.kind+'\\','+x.id+')"><span class="tag '+x.kind+'">'+x.kind+'</span><span class="rn">#'+x.id+'</span><span class="nm">'+esc(x.name)+'</span></div>'
  ).join('') || (q ? '<p class="sub" style="padding:12px">No matches.</p>' : '');
}
function link(kind, id){ const n = kind==='room'?W.rooms[id]?.name : kind==='item'?W.items[id]?.name : W.mobs[id]?.name; return '<a onclick="show(\\''+kind+'\\','+id+')">'+esc(n||('#'+id))+'</a>'; }

function show(kind, id){
  if (kind==='room') return showRoom(W.rooms[id]);
  if (kind==='item') return showItem(W.items[id], id);
  if (kind==='mob')  return showMob(W.mobs[id], id);
}
function showRoom(r){
  if(!r){detEl.innerHTML='<p>Unknown room.</p>';return;}
  let h = '<h2>'+esc(r.name)+'</h2><p class="sub">room #'+r.vnum+' · area '+esc(r.area)+' · sector '+r.sector+'</p>';
  const ex = Object.entries(r.exits||{});
  h += '<div class="k">Exits</div><div class="exits">'+(ex.length?ex.map(([d,to])=>'<a onclick="show(\\'room\\','+to+')">'+DIRW[d]+' → '+(W.rooms[to]?esc(W.rooms[to].name):'#'+to)+'</a>').join(''):'<span class="sub">none</span>')+'</div>';
  const mobs = roomMobs[r.vnum]||[], its = roomItems[r.vnum]||[];
  if(mobs.length){h+='<div class="k">Creatures here</div>'+mobs.map(v=>'<span class="pill">'+link('mob',v)+(W.mobs[v]?' (lvl '+W.mobs[v].level+')':'')+'</span>').join('');}
  if(its.length){h+='<div class="k">Items here</div>'+its.map(v=>'<span class="pill">'+link('item',v)+'</span>').join('');}
  detEl.innerHTML = h;
}
function showItem(o, id){
  o = o || {vnum:id}; const src = W.itemSources[id];
  let h = '<h2>'+esc(o.name||('#'+id))+'</h2><p class="sub">item #'+(o.vnum||id)+' · '+esc(o.type||'?')+' · area '+esc(o.area||'?')+'</p>';
  if(o.wear?.length) h+='<div class="k">Worn</div>'+o.wear.map(w=>'<span class="pill">'+esc(w)+'</span>').join('');
  if(o.value) h+='<div class="k">Value</div><span class="pill">'+esc(o.value)+'</span>';
  if(o.affects?.length) h+='<div class="k">Affects</div>'+o.affects.map(a=>'<span class="pill">'+esc(a.stat)+'</span>').join('');
  if(o.flags?.length) h+='<div class="k">Flags</div>'+o.flags.map(f=>'<span class="pill">'+esc(f)+'</span>').join('');
  if(src?.sources?.length){
    h+='<div class="k">Where to get it</div><table><tr><th>How</th><th>Where</th></tr>';
    for(const s of src.sources){ const room=s.room?(W.rooms[s.room]?link('room',s.room)+' (#'+s.room+')':'#'+s.room):''; const mob=s.mob?link('mob',s.mob):''; h+='<tr><td>'+esc(s.kind)+(mob?' — '+mob:'')+'</td><td>'+room+'</td></tr>'; }
    h+='</table>';
  }
  detEl.innerHTML = h;
}
function showMob(m, id){
  m = m || {vnum:id}; const sp = W.mobSpawns[id];
  let h = '<h2>'+esc(m.name||('#'+id))+'</h2><p class="sub">creature #'+(m.vnum||id)+' · level '+(m.level??'?')+' · area '+esc(m.area||'?')+'</p>';
  if(m.behaviour?.length) h+='<div class="k">Behaviour</div>'+m.behaviour.map(b=>'<span class="pill">'+esc(b)+'</span>').join('');
  if(m.sighting) h+='<div class="k">Sighting</div><p>'+esc(m.sighting)+'</p>';
  if(sp?.rooms?.length){ h+='<div class="k">Spawns in</div>'+sp.rooms.map(r=>'<span class="pill">'+(W.rooms[r]?link('room',r):'#'+r)+'</span>').join(''); }
  // what it drops
  const drops = Object.entries(W.itemSources).filter(([,s])=>s.sources.some(x=>x.mob===id)).map(([v])=>+v);
  if(drops.length) h+='<div class="k">Carries / drops</div>'+drops.map(v=>'<span class="pill">'+link('item',v)+'</span>').join('');
  detEl.innerHTML = h;
}
qEl.addEventListener('input', e => search(e.target.value));
</script></body></html>`;

  fs.writeFileSync(path.join(OUT, 'world-browser.html'), html);
  const kb = Math.round(Buffer.byteLength(html) / 1024);
  console.log(`browser : world-browser.html (${kb} KB, ${Object.keys(data.rooms).length} rooms + ${Object.keys(data.items).length} items + ${Object.keys(data.mobs).length} mobs embedded)`);
  console.log(`          -> world-maps/world-browser.html`);
}
main();
