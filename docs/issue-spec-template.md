---
issue: <#>
type: feature | bug | balance | content
status: spec-review        # spec-review -> approved -> in-progress -> in-review -> deployed -> done
opened: <YYYY-MM-DD>
reporter: <github-handle>  # GitHub handle only -- never Discord aliases (public repo)
---

# <Short title>  (#<issue>)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/<#>

## Summary
<One paragraph: what we're doing and why.>

## Reporter's request
<Condensed from the issue + form fields. Quote the essentials.>

## Clarifications
<The Q&A captured from the question/answer loop. "None — request was clear" if direct.>

## Proposed design
<The plan: mechanics and behavior, plus which subsystems/files are touched
(src/ C, area/ data, tools/ Node, web/ React). Note any coupling
(e.g. tools/lib/area.mjs must mirror src/core/db.c).>

## Acceptance criteria
- [ ] <Observable behavior that proves it's done, and how to verify it on dev.>

## Risk & balance notes
<Gotchas, balance implications, migration/rollback considerations, blast radius.>

## Out of scope
<What this explicitly does not do, to keep the change bounded.>
