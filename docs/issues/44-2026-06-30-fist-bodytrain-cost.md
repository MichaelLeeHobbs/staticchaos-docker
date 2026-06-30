---
issue: 44
type: bug
status: spec-review
opened: 2026-06-30
reporter: code-audit (#35)
---

# Fist bodytrain: arms/legs cost uses the wrong part (#44)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/44

## The bug (confirmed in source)
`do_bodytrain` (`src/fist.c`) prices each body part as
`cost = 1500 * (powers[part]+1) * isquare(powers[part]+1)`. Torso and hands are correct, but **arms
and legs use `isquare(powers[F_HANDS]+1)`** for the second factor instead of their own part — a
copy-paste slip:
```
arms: cost = 1500 * (powers[F_ARMS]+1) * isquare(powers[F_HANDS]+1);   // BUG: F_HANDS
legs: cost = 1500 * (powers[F_LEGS]+1) * isquare(powers[F_HANDS]+1);   // BUG: F_HANDS
```

## Fix
In the arms branch use `isquare(powers[F_ARMS]+1)`; in the legs branch use `isquare(powers[F_LEGS]+1)`.
One token each. The intended self-referential curve is obvious from the torso/hands branches.

## ⚠️ Balance impact (maintainer decision)
This changes advancement costs. Today the arms/legs `isquare` factor tracks the player's **hands**
level, so:
- a Fist with **low hands** trains arms/legs unusually **cheaply** (the curve never steepens);
- with **high hands**, arms/legs are **over**priced.
Fixing it makes arms/legs follow their own escalating curve (more expensive to push to high levels).
No player likely optimized around this, but it does retroactively change the economy. **Confirm: fix
as-is** (recommended — it's clearly unintended), or leave it?

## Acceptance criteria
- [ ] Arms cost scales with `F_ARMS`, legs cost with `F_LEGS` (verify a few values by hand).
- [ ] Torso/hands unchanged. Compiles (32-bit Docker).

## Risk
Trivial code change; the only risk is the balance/economy shift above.
