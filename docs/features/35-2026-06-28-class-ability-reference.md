---
issue: 35
type: feature
status: spec-review
opened: 2026-06-28
reporter: ScathachMDG
---

# Detailed, maintained class ability reference (#35)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/35

## Summary
A comprehensive, accurate, **kept-current** per-class ability reference on the site — every ability
with mechanics, numbers, durations, saves, costs, and cross-class interactions — so balance can be
checked without reading C. Skatha confirmed: full detail for every ability; balance fixes first
(done — #34/#36 shipped), then build this.

## What exists today
- **`world-maps/CLASSES.md`** — auto-generated from `class_table`/`skill_table` (`build:classes`):
  accurate ability *index* + rank/mana, but no formulas/interactions (not in the tables).
- **`docs/CLASS-{SAIYAN,FIST,PATRYN,SORCERER,MAZOKU}.md`** — narrative **player guides** (~80–100
  lines each, new-player audience). Not the place for exhaustive numbers.

## The core tension (must design around)
Mechanics/numbers live in the **C function bodies** (`src/{saiyan,fist,patryn,sorcerer,mazoku}.c`),
not in any table — so this reference is **hand-extracted and will drift** unless we keep it current
by process. A *wrong* balance reference is worse than none. Two parts: (1) build it accurately;
(2) keep it honest.

## Proposed design
1. **New per-class reference docs** — `docs/CLASS-<X>-ABILITIES.md`, one per class, **separate** from
   the narrative guides (keeps guides readable, references exhaustive). Add each to `USER_DOCS` in
   `web/scripts/sync-content.mjs` so they publish under "Class guides".
2. **Contents per ability:** name + command/chant, what it does, damage/effect **formula** (e.g.
   `dice(body, body*3/2)`), cost (power/mana), WAIT/lag, save behavior, durations, and notable
   **cross-class interactions** (e.g. Ryuken vs blocks/wards, Balus Wall vs Gaav Flare). Source of
   truth = the C function + `const.c` tables.
3. **Keep current (the important half):** add a **required step to the issue-ops build pipeline** —
   when a change touches an ability, update that ability's entry in the same PR. Encode it in
   `.claude/skills/issue-ops` so every future balance change refreshes the reference by construction.
4. **Build it once, per class:** large (Sorcerer ~87 functions, Patryn runeweave combos), so do it
   **class-by-class** — parallelizable via the orchestrate skill (one agent per class, each reads its
   `.c` file + tables). **Each class gets a verification pass** (numbers checked against source) —
   accuracy is the main risk of hand-extraction.

## Decisions for review (maintainer)
- **Separate reference docs vs enriching the guides** — recommend separate (`CLASS-<X>-ABILITIES.md`).
- **Per-class docs vs one big doc** — recommend per class (smaller diffs, parallel build, easier upkeep).
- **Pipeline step** — OK to make "update the ability reference" a required part of issue-ops builds?
- **Order** — class-by-class; any class to do first as a template (Saiyan is smallest/cleanest)?

## Acceptance criteria
- [ ] A `CLASS-<X>-ABILITIES.md` per class, published on the site, covering **every** ability with
      mechanics + numbers + interactions, verified against the source.
- [ ] `CLASSES.md` index still builds; new docs appear under Class guides.
- [ ] issue-ops updated so future ability changes update the reference in the same PR.

## Risk & balance notes
- **Accuracy** is the headline risk (hand-extracted from C) → per-class verification pass, and the
  pipeline step prevents future drift. Large effort but pure docs — zero gameplay risk.
- Numbers shift with balance patches; the pipeline step is what makes "up to date" real.

## Out of scope
- Auto-generating formulas from C (not feasible); changing any ability behavior.
