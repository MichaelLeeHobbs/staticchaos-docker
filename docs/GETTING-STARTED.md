# Getting Started — never played a MUD before?

Welcome! This walks you from zero to standing in the game, even if you've never
touched a MUD. It takes about 10 minutes.

## What is a MUD?

A **MUD** (Multi-User Dungeon) is a multiplayer game made of **text**. Instead of
graphics, the game describes rooms, creatures, and items in words, and you type
short commands to act — `look`, `north`, `kill rat`, `say hello`. Think of it as a
shared, living text adventure that other people are playing at the same time.

**Static Chaos** is our MUD: an anime/fantasy world with classes like Saiyan,
Sorcerer, and more. You explore areas, fight monsters, gear up, and grow stronger.

## Step 1 — Get a MUD client

You *can* connect with a plain telnet client, but a real MUD client is far nicer —
it gives you a command line, colours, the auto-map, and status gauges. We recommend
**Mudlet** (free, Windows / macOS / Linux).

1. Go to **[mudlet.org](https://www.mudlet.org/)** and download the installer for
   your operating system.
2. Install it like any normal app and open it.

## Step 2 — Connect to the server

When Mudlet opens you'll see the **Profiles** / connection screen ("Select a
profile"). Make a new connection:

1. Click **New** (or "Add new profile").
2. Fill in:
   - **Profile name:** `Static Chaos` (anything you like)
   - **Server address:** `rustycatz.fortiddns.com`
   - **Port:** `4000`
3. Click **Connect**.

A black window opens with the **Static Chaos** title screen. You're connected!

## Step 3 — Create your character

The game will prompt you, step by step. Just read each line and type your answer,
pressing **Enter** after each:

1. **"By what name do you wish to be known?"** — type the name you want, then
   confirm with `Y`.
2. **Password** — pick one (at least 5 characters), then type it again to confirm.
   *(It's a game password — don't reuse a real/important one.)*
3. **Sex** — `M`, `F`, or `N`.
4. **Class** — choose your path when asked (Saiyan, Patryn, Fist, Sorcerer, or
   Mazoku). They play very differently, so skim the **class guides** in Docs to
   pick one — new players can choose anything and learn as they go. (The
   **Gearing Guide** explains how power works here: mostly *training*, not gear.)
5. Follow any remaining prompts (press **Enter** to get through the message of the
   day) until you're dropped into a room with a description.

You're in the game.

## Step 4 — Add the Mudlet add-ons (automatic)

We publish ready-made Mudlet packages — **HP / Mana / Move + class-resource gauges,
an auto-mapper, and Tab-completion**. The easiest way to get them:

**When you connect, the server tells Mudlet to install them — accept the prompt that
pops up.** The add-ons download and install themselves, and update automatically when
we improve them. (Mudlet asks first, for safety — just say yes.)

Prefer to do it by hand? Grab `StaticChaos.xml` from the **Mudlet / GMCP** page and
install it via **Settings → Packages → Install**.

## Step 5 — Learn to move and look around

A handful of commands gets you started:

| Type this | What it does |
|---|---|
| `look` (or `l`) | Describe the room you're in again |
| `north` / `south` / `east` / `west` / `up` / `down` (or `n s e w u d`) | Move |
| `exits` | List the exits from this room |
| `who` | See who else is online |
| `say hello` | Speak out loud in your room |
| `inventory` (or `i`) | See what you're carrying |
| `score` | See your character's stats |
| `help` | The in-game help system |
| `quit` | Save and leave the game safely |

Don't worry about memorising these — type `help` any time, and the Tab-completion
add-on will finish commands for you.

## Where to go next

- **World Map** (this site) — explore every area and how the rooms connect.
- **Browser** — search items, monsters, and shops.
- **Gearing Guide** — how to actually get stronger (and which gear is worth it).
- **Mobile Suits** — climb into a Gundam-style mech; a free trainer Leo (and ammo) waits in the Temple where you start.
- **Mudlet / GMCP** — the add-ons and how to install them.

Have fun, and don't be shy — `say hi` when you see someone around.
