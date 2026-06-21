/*
 * trials.c -- Class Trials, Phase B: per-class, mechanic-forcing objectives.
 *
 * See docs/CLASS-TRIALS-PHASE-B-DESIGN.md.  Phase A shipped one level-scaled
 * mob for every tier; this module overlays a per-class *objective* on each
 * tier so the encounter forces the class's signature mechanic instead of a
 * pure survival check.
 *
 * The central primitive is a STATELESS, time-synced telegraph + a damage gate:
 *
 *   - A requirement enum (TRIAL_REQ_*) names a class "state" the player must
 *     hold (powered up, Ki Wall raised, Defense up, a ward up, ...).
 *
 *   - trial_table[class][tier] maps each tier to one requirement plus three
 *     MUD-voice strings: a lesson (shown on entry), a telegraph (the wind-up
 *     room echo), and a fail line (the corrective when the punish lands).
 *
 *   - trial_update() runs once per real second from second_update().  On a
 *     5-second cadence it telegraphs (current_time %% 5 == 0) and then resolves
 *     (current_time %% 5 == 2): if the player is in the required state they
 *     weather it, otherwise they take a scaled punish hit + the corrective.
 *
 *   - trial_damage_gate() runs near the top of damage(): a player can only
 *     hurt the tier mob while in the required state (the Ravenholm constraint
 *     from the design doc).  Out of state -> 0 damage + an occasional hint.
 *
 * NOTE on "active-action" tiers: the design's later tiers want true active
 * verbs (interrupt a cast, switch schools, weave the *right* runeweave).  We
 * have no per-ability event hook yet, so those tiers currently gate on a
 * PERSISTENT-STATE PROXY (e.g. "powered up" / "a ward is up") rather than the
 * exact action.  This is an accepted, tunable simplification (owner's call,
 * best-judgment) and can be upgraded later with per-ability hooks.  Every
 * requirement-to-tier assignment below is marked "tunable".
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

/*
 * Per-class objective table.  Indexed [ch->class][tier-1].
 *
 * Row 0 (CLASS_NONE) is all-NONE as a safety net: an NPC or an unclassed PC
 * standing in a trial room never gets gated or punished.
 *
 * The req matrix (tunable) follows the design doc's "introduce -> combine ->
 * mock-PvP" arc.  Early tiers teach the state; later tiers re-test it under
 * pressure / frame the hardest matchup at T8.
 */
const struct trial_def trial_table[MAX_CLASS][MAX_TRIALS] =
{
  /* ---- CLASS_NONE (0): safety -- never gated. ---- */
  {
    { TRIAL_REQ_NONE, "", "", "" },
    { TRIAL_REQ_NONE, "", "", "" },
    { TRIAL_REQ_NONE, "", "", "" },
    { TRIAL_REQ_NONE, "", "", "" },
    { TRIAL_REQ_NONE, "", "", "" },
    { TRIAL_REQ_NONE, "", "", "" },
    { TRIAL_REQ_NONE, "", "", "" },
    { TRIAL_REQ_NONE, "", "", "" },
  },

  /* ---- CLASS_SAIYAN (1): Ki resource, power-up, Ki Wall. ---- *
   * NONE, POWERUP, KIWALL, POWERUP, KIWALL, KIWALL, POWERUP, KIWALL  (tunable) */
  {
    { TRIAL_REQ_NONE,
      "`WLesson:`n Strike the dummy down.  No tricks this tier -- just fight.",
      "", "" },
    { TRIAL_REQ_POWERUP,
      "`WLesson:`n This foe shrugs off a calm fist.  `Ypower up`n -- it only bleeds while your ki burns.",
      "The brute sneers, `Ryou aren't even trying`R.  A backhand comes for you!",
      "`RToo soft.`n It hurts you because your ki is banked -- `Ypower up`n and keep it lit." },
    { TRIAL_REQ_KIWALL,
      "`WLesson:`n A heavy blow is coming each beat.  Raise your `YKi Wall`n and hold it up.",
      "The fighter winds up a crushing haymaker -- it telegraphs the swing!",
      "`RNo wall, full blow.`n Raise `YKi Wall`n BEFORE the swing lands, not after." },
    { TRIAL_REQ_POWERUP,
      "`WLesson:`n Sustain the burst.  Stay `Ypowered up`n the whole fight or you do nothing.",
      "It circles, baiting you to drop your guard and cool down -- then lunges!",
      "`RYour aura guttered out.`n Keep the power-up *running*; don't let it lapse." },
    { TRIAL_REQ_KIWALL,
      "`WLesson:`n Adds and pressure.  Keep your `YKi Wall`n up while you trade blows.",
      "Stone fists rain in from every side -- brace or be broken!",
      "`RThe barrage got through.`n Hold `YKi Wall`n through the pressure, every beat." },
    { TRIAL_REQ_KIWALL,
      "`WLesson:`n Burst timing.  Read the telegraph and have `YKi Wall`n up in the punish window.",
      "Energy gathers to a white point -- a burst is about to detonate!",
      "`RCaught flat.`n The burst is telegraphed; `YKi Wall`n must be up when it goes off." },
    { TRIAL_REQ_POWERUP,
      "`WLesson:`n Everything at once.  Stay `Ypowered up`n -- that is the price of admission now.",
      "The veteran tests you with a flurry meant for an unprepared Saiyan!",
      "`RUnprepared.`n Under this much pressure you MUST stay powered up to survive and hit." },
    { TRIAL_REQ_KIWALL,
      "`WLesson:`n Mock-PvP vs a chant-caster (your worst matchup).  Keep `YKi Wall`n up and close it out.",
      "The duelist begins a killing chant -- punish it before it finishes!",
      "`RThe chant landed.`n Against casters, `YKi Wall`n keeps you alive long enough to win." },
  },

  /* ---- CLASS_PATRYN (2): rune magic; defenses + the six wards. ---- *
   * NONE, DEFENSES, WARD, DEFENSES, WARD, WARD, DEFENSES, WARD  (tunable) */
  {
    { TRIAL_REQ_NONE,
      "`WLesson:`n Cut down the dummy.  No runes demanded this tier -- just fight.",
      "", "" },
    { TRIAL_REQ_DEFENSES,
      "`WLesson:`n Raise your rune `Ydefenses`n -- this foe only bleeds while they are up.",
      "The construct grinds forward, `Rprobing for an opening`R in your guard!",
      "`RGuard down.`n Raise your rune `Ydefenses`n; bare skin can't harm it or hold it off." },
    { TRIAL_REQ_WARD,
      "`WLesson:`n Read the element and put up the matching `Yward`n (wind/earth/flame/water/spirit/negative).",
      "Elemental force coils around the foe -- the right ward turns it aside!",
      "`RWrong element, no ward.`n A `Yward`n must be up to weather the strike and harm it." },
    { TRIAL_REQ_DEFENSES,
      "`WLesson:`n Hold your `Ydefenses`n under a steady beating -- don't let them lapse.",
      "Blows hammer your wards in a relentless cadence -- keep them fed!",
      "`RDefenses lapsed.`n Re-raise and KEEP your rune `Ydefenses`n up through the pressure." },
    { TRIAL_REQ_WARD,
      "`WLesson:`n Two threats, two elements.  Keep a `Yward`n up and switch it to the live threat.",
      "One foe sparks, the other smoulders -- ward the one that's about to strike!",
      "`RUnwarded.`n Keep the correct `Yward`n raised for whichever foe winds up." },
    { TRIAL_REQ_WARD,
      "`WLesson:`n Survive a telegraphed burst behind the proper defensive `Yward`n.",
      "Raw rune-force gathers to a burst -- only a `Yward`n will blunt it!",
      "`RNo ward, full burst.`n Have the right `Yward`n up the instant it detonates." },
    { TRIAL_REQ_DEFENSES,
      "`WLesson:`n Weave under pressure.  Your rune `Ydefenses`n must stay up while you cast.",
      "The foe presses, trying to shatter your weave mid-cast!",
      "`RBroken weave.`n Keep your `Ydefenses`n up so your runeweaves resolve under fire." },
    { TRIAL_REQ_WARD,
      "`WLesson:`n Mock-PvP vs your hardest matchup.  Keep the right `Yward`n up and out-weave it.",
      "Your rival's signature attack builds -- ward it and answer in kind!",
      "`ROut-played.`n The right `Yward`n is what lets you trade and win this matchup." },
  },

  /* ---- CLASS_FIST (3): Ki/spirit martial arts. ---- *
   * NONE, FISTKI x7  (tunable) -- every tier wants Ki built up. */
  {
    { TRIAL_REQ_NONE,
      "`WLesson:`n Beat down the dummy.  No Ki demanded this tier -- just land your strikes.",
      "", "" },
    { TRIAL_REQ_FISTKI,
      "`WLesson:`n `YBuild your Ki`n -- an empty fist barely scratches this foe.",
      "The sparring dummy taunts, `Rwhere is your spirit?`R -- and swats at you!",
      "`RKi too low.`n `YBuild your Ki`n; your strikes only bite when your spirit is high." },
    { TRIAL_REQ_FISTKI,
      "`WLesson:`n Keep your `YKi`n charged -- a defensive stance needs spirit to hold.",
      "A heavy blow telegraphs -- only a spirited stance turns it!",
      "`RNo spirit, no stance.`n Keep your `YKi`n up so your guard actually holds." },
    { TRIAL_REQ_FISTKI,
      "`WLesson:`n Charge `YKi`n for the interrupt -- dim-mak needs spirit to fire.",
      "The foe begins a focused technique -- disrupt it with a spirited strike!",
      "`RNot enough spirit.`n Keep your `YKi`n high so your interrupt actually lands." },
    { TRIAL_REQ_FISTKI,
      "`WLesson:`n Combo windows demand `YKi`n -- keep it topped to chain your kicks.",
      "An opening flickers -- only a charged fist can capitalize in time!",
      "`RWindow missed.`n Keep `YKi`n charged so you can punish the opening." },
    { TRIAL_REQ_FISTKI,
      "`WLesson:`n Adds and priority -- you need `YKi`n in reserve to deal with both.",
      "Two foes press in -- a depleted martial artist gets overrun!",
      "`RDrained dry.`n Keep `YKi`n in reserve to handle priority targets and adds." },
    { TRIAL_REQ_FISTKI,
      "`WLesson:`n Survive a burst -- a spirited `YKi`n guard is the difference.",
      "Force gathers for a burst -- brace behind your spirit!",
      "`RKi spent.`n Keep `YKi`n up so your guard soaks the burst." },
    { TRIAL_REQ_FISTKI,
      "`WLesson:`n Mock-PvP vs a caster (your worst matchup).  Keep `YKi`n high and close the gap.",
      "The caster begins a chant -- you must reach and break it!",
      "`ROutranged.`n Against casters, high `YKi`n is what lets you close and interrupt." },
  },

  /* ---- CLASS_SORCERER (4): fragile chant-caster. ---- *
   * NONE, DEFENSE, VASGLUUDO, DEFENSE, HOLYRESIST, HOLYRESIST, DEFENSE, DEFENSE  (tunable) */
  {
    { TRIAL_REQ_NONE,
      "`WLesson:`n Land your chants on the dummy.  No defense demanded this tier -- just cast.",
      "", "" },
    { TRIAL_REQ_DEFENSE,
      "`WLesson:`n Raise `YDefense`n -- this foe punishes a naked caster and only bleeds when you're shielded.",
      "The wraith's eyes flare, `RI'll silence your prattling!`R -- a wave builds!",
      "`RUnshielded.`n Raise `YDefense`n; your chant (and your skin) needs the barrier up." },
    { TRIAL_REQ_VASGLUUDO,
      "`WLesson:`n Invoke `YVas Gluudo`n to brace -- without it the foe's blow shatters your chant.",
      "The foe gathers a chant-breaking blast -- brace with Vas Gluudo!",
      "`RUnbraced.`n Bring up `YVas Gluudo`n before the blast or your casting collapses." },
    { TRIAL_REQ_DEFENSE,
      "`WLesson:`n Cast under fire -- hold `YDefense`n while the foe pressures you.",
      "Blows rain down to break your concentration -- shield and chant on!",
      "`RConcentration broken.`n Keep `YDefense`n up so your chants resolve under pressure." },
    { TRIAL_REQ_HOLYRESIST,
      "`WLesson:`n A black nuke is coming.  Raise `YHoly Resist`n -- the wrong defense won't save you.",
      "Black fire coalesces -- only Holy Resist will blunt this nuke!",
      "`RWrong resist.`n Raise `YHoly Resist`n vs the black nuke; a plain shield isn't enough." },
    { TRIAL_REQ_HOLYRESIST,
      "`WLesson:`n Sustained black damage -- keep `YHoly Resist`n up the whole fight.",
      "Waves of black flame roll in without pause -- keep the resist lit!",
      "`RResist dropped.`n Hold `YHoly Resist`n through every wave, not just the first." },
    { TRIAL_REQ_DEFENSE,
      "`WLesson:`n Long chant under counter-pressure -- hold `YDefense`n and finish it.",
      "The foe begins its own counter-cast -- shield up and out-chant it!",
      "`RChant lost.`n Keep `YDefense`n up so your long chant survives the counter." },
    { TRIAL_REQ_DEFENSE,
      "`WLesson:`n Mock-PvP vs a Saiyan Ki-Waver (your worst matchup).  Keep `YDefense`n up and protect the chant.",
      "The duelist's ki flares for a wave -- shield the chant and let it land!",
      "`RChant shattered.`n Against a Ki-Waver, `YDefense`n is what keeps your chant alive to win." },
  },

  /* ---- CLASS_MAZOKU (5): astral/demonic powers, focus. ---- *
   * NONE, FOCUS x7  (tunable) -- every tier wants Focus charged. */
  {
    { TRIAL_REQ_NONE,
      "`WLesson:`n Strike the dummy down.  No focus demanded this tier -- just attack.",
      "", "" },
    { TRIAL_REQ_FOCUS,
      "`WLesson:`n Gather your `YFocus`n -- an unfocused Mazoku barely marks this foe.",
      "The shade hisses, `Ryour will is scattered`R -- and lashes out!",
      "`RUnfocused.`n Gather your `YFocus`n; your power only bites when your will is gathered." },
    { TRIAL_REQ_FOCUS,
      "`WLesson:`n Hold `YFocus`n through a telegraphed hit -- it's your shield as much as your sword.",
      "A heavy blow telegraphs -- only gathered will turns it!",
      "`RFocus lost.`n Keep your `YFocus`n up so the blow glances off your will." },
    { TRIAL_REQ_FOCUS,
      "`WLesson:`n Charge `YFocus`n for the interrupt -- the release-blast needs it to fire.",
      "The foe begins a chant -- release a blast to shatter it!",
      "`RNot enough will.`n Keep `YFocus`n charged so your release-blast can interrupt." },
    { TRIAL_REQ_FOCUS,
      "`WLesson:`n Manage your `YFocus`n -- don't burn it all; keep the well from running dry.",
      "The foe baits you into spending recklessly, then strikes the gap!",
      "`RWell ran dry.`n Pace your `YFocus`n; keep enough gathered to act every beat." },
    { TRIAL_REQ_FOCUS,
      "`WLesson:`n Adds and priority -- keep `YFocus`n in reserve for both threats.",
      "Two foes converge -- a scattered Mazoku is swarmed!",
      "`RScattered.`n Keep `YFocus`n in reserve to handle priority targets and adds." },
    { TRIAL_REQ_FOCUS,
      "`WLesson:`n Survive a burst behind gathered `YFocus`n.",
      "Dark force gathers to a burst -- brace behind your will!",
      "`RFocus spent.`n Keep `YFocus`n gathered so your will soaks the burst." },
    { TRIAL_REQ_FOCUS,
      "`WLesson:`n Mock-PvP vs your hardest matchup.  Keep `YFocus`n high and break their rhythm.",
      "Your rival builds their signature strike -- focus, blast, and answer!",
      "`ROverwhelmed.`n High `YFocus`n is what lets you interrupt and out-tempo this matchup." },
  },
};

/*
 * trial_ready -- runtime resolver for a requirement.
 *
 * The gsn_* globals are assigned at boot (runtime), so they CANNOT live in the
 * const trial_table above; the table stores only the int req and this switch
 * resolves it live.  TRIAL_REQ_NONE is always satisfied.
 *
 * "active-action" reqs (KIWALL interrupt, school-switch, runeweave choice,
 * release-blast interrupt) currently resolve to a persistent-state proxy --
 * see the file header note.  (tunable)
 */
bool trial_ready( CHAR_DATA *ch, int req )
{
  if ( ch == NULL || IS_NPC(ch) || ch->pcdata == NULL )
    return TRUE;       /* never gate an NPC / unclassed actor */

  switch ( req )
  {
    default:
    case TRIAL_REQ_NONE:
      return TRUE;

    /* Saiyan: must be ACTIVELY powered up -- at least a third of max charge.
     * A resting Saiyan idles with a little S_POWER, so a bare ">0" would pass
     * without forcing a power-up; the threshold makes the tier teach the button.
     * (Tunable: raise the fraction for a steeper requirement.) */
    case TRIAL_REQ_POWERUP:
      return ch->pcdata->powers[S_POWER] * 3 >= UMAX( 1, ch->pcdata->powers[S_POWER_MAX] );

    /* Saiyan: Ki Wall raised (proxy for the timed wall / interrupt). */
    case TRIAL_REQ_KIWALL:
      return is_affected( ch, gsn_kiwall );

    /* Sorcerer: the Defense barrier is up. */
    case TRIAL_REQ_DEFENSE:
      return is_affected( ch, gsn_defense );

    /* Sorcerer: Vas Gluudo brace affect.  ( != 0 -- the AFF bit is high, so a
       bare IS_AFFECTED truncates to bool=0; force a real boolean. ) */
    case TRIAL_REQ_VASGLUUDO:
      return IS_AFFECTED( ch, AFF_VAS_GLUUDO ) != 0;

    /* Sorcerer: Holy Resist up (the right resist vs a black nuke). */
    case TRIAL_REQ_HOLYRESIST:
      return IS_AFFECTED( ch, AFF_HOLY_RESIST ) != 0;

    /* Sorcerer: Balus Wall up (reserved; not currently assigned in matrix). */
    case TRIAL_REQ_BALUSWALL:
      return is_affected( ch, gsn_balus_wall );

    /*
     * Fist: Ki built up.  powers[F_KI_MAX] is the player's STORED cap; if it's
     * set, require at least half the cap so an almost-empty bar doesn't count.
     * If no cap is recorded, fall back to "any Ki at all" ( > 0 ).
     */
    case TRIAL_REQ_FISTKI:
      if ( ch->pcdata->powers[F_KI_MAX] > 0 )
        return ch->pcdata->powers[F_KI] >= ch->pcdata->powers[F_KI_MAX] / 2;
      return ch->pcdata->powers[F_KI] > 0;

    /* Patryn: rune defenses raised (the P_DEFENSES bit in the P_BITS slot). */
    case TRIAL_REQ_DEFENSES:
      return IS_SET( ch->pcdata->powers[P_BITS], P_DEFENSES );

    /* Patryn: any of the six elemental wards is up. */
    case TRIAL_REQ_WARD:
      return is_affected( ch, gsn_wind_ward )
          || is_affected( ch, gsn_earth_ward )
          || is_affected( ch, gsn_flame_ward )
          || is_affected( ch, gsn_water_ward )
          || is_affected( ch, gsn_spirit_ward )
          || is_affected( ch, gsn_negative_ward );

    /* Mazoku: Focus gathered ( > 0 ). */
    case TRIAL_REQ_FOCUS:
      return ch->pcdata->powers[M_FOCUS] > 0;
  }
}

/*
 * trial_def_for -- fetch the objective for a PC standing in a trial room.
 * Returns NULL if ch is not a PC in a tier room.  *tier_out (1-based) and the
 * room's tier mob vnum are derived for the caller's convenience.
 */
static const struct trial_def *trial_def_for( CHAR_DATA *ch, int *tier_out )
{
  int rvnum, tier, cls;

  if ( ch == NULL || IS_NPC(ch) || ch->in_room == NULL )
    return NULL;

  rvnum = ch->in_room->vnum;
  if ( rvnum < TRIAL_ROOM_FIRST || rvnum >= TRIAL_ROOM_FIRST + MAX_TRIALS )
    return NULL;

  tier = rvnum - TRIAL_ROOM_FIRST + 1;          /* 1-based */
  cls  = ch->class;
  if ( cls < 0 || cls >= MAX_CLASS )
    cls = CLASS_NONE;

  if ( tier_out != NULL )
    *tier_out = tier;

  return &trial_table[cls][tier-1];
}

/*
 * trial_damage_gate -- the Ravenholm constraint.
 *
 * Called near the TOP of damage() (after the POS_DEAD guard, before any damage
 * reductions).  If a PC is attacking THIS tier's trial mob in the trial room
 * and the tier requires a class state the PC is NOT currently holding, the
 * attack does ZERO damage (with an occasional hint).  In-state attacks, and
 * any TRIAL_REQ_NONE tier, pass through unchanged.
 */
int trial_damage_gate( CHAR_DATA *ch, CHAR_DATA *victim, int dam )
{
  const struct trial_def *def;
  char buf[MAX_STRING_LENGTH];
  int tier, mvnum;

  if ( ch == NULL || victim == NULL )
    return dam;

  /* Victim must be an NPC tier mob. */
  if ( !IS_NPC(victim) || victim->pIndexData == NULL )
    return dam;
  if ( victim->pIndexData->vnum < TRIAL_MOB_FIRST
    || victim->pIndexData->vnum >= TRIAL_MOB_FIRST + MAX_TRIALS )
    return dam;

  /* Attacker must be a PC standing in the matching tier room. */
  def = trial_def_for( ch, &tier );
  if ( def == NULL || def->req == TRIAL_REQ_NONE )
    return dam;

  mvnum = TRIAL_MOB_FIRST + (tier-1);
  if ( victim->pIndexData->vnum != mvnum )
    return dam;       /* a different tier's mob (e.g. a stray add): no gate */

  if ( trial_ready( ch, def->req ) )
    return dam;       /* in the required state: full damage */

  /* Out of state: the mob shrugs it off.  Hint roughly 1 in 3. */
  if ( number_range( 0, 2 ) == 0 )
  {
    sprintf( buf, "Your attack glances off harmlessly.  %s\n\r", def->lesson );
    send_to_char( buf, ch );
  }
  return 0;
}

/*
 * trial_update -- the telegraph + punish clock.  Call once per real second
 * from second_update().
 *
 * A static beat counter advances once per call, so every beat fires exactly
 * once regardless of any drift between second_update() and current_time (the
 * two are NOT guaranteed to step in lockstep -- relying on current_time %% 5
 * silently skips beats).  The cycle is 5 beats, shared by all players in
 * trials (stateless -- no per-char timers, no save changes):
 *   beat 0  -> telegraph (the wind-up echo)
 *   beat 2  -> resolve (weather it, or take a scaled punish)
 * That gives the player ~2 real seconds to react -- the PvP-speed window from
 * the design doc.
 *
 * The punish hit is dealt via damage() with the tier mob as the attacker so
 * death credit / corpse-to-lobby is handled exactly like a normal kill.
 */
void trial_update( void )
{
  static int beat = 0;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ch;
  CHAR_DATA *tmob;
  CHAR_DATA *rch;
  const struct trial_def *def;
  int phase, tier, mvnum, dam;

  phase = beat;
  beat  = ( beat + 1 ) % 5;       /* 5-beat cycle: 0 telegraph, 2 resolve */
  if ( phase != 0 && phase != 2 )
    return;       /* nothing to do on the off-beats */

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected != CON_PLAYING || d->character == NULL )
      continue;

    ch = d->character;
    def = trial_def_for( ch, &tier );
    if ( def == NULL || def->req == TRIAL_REQ_NONE )
      continue;

    if ( ch->position == POS_DEAD )
      continue;

    /* ---- Telegraph beat: announce the wind-up. ---- */
    if ( phase == 0 )
    {
      send_to_char( def->telegraph, ch );
      send_to_char( "\n\r", ch );
      continue;
    }

    /* ---- Resolve beat (phase == 2). ---- */
    if ( trial_ready( ch, def->req ) )
    {
      send_to_char( "`GYou weather it -- your guard holds.`n\n\r", ch );
      continue;
    }

    /*
     * Failed the window: deal the corrective + a scaled punish hit.
     * Scaling is by tier, not by raw mob power, per the design's
     * "escalate by complexity, keep it fair" note.   (tunable)
     */
    dam = UMAX( 10, ch->max_hit * (5 + tier*3) / 100 );

    /* Find this tier's mob to credit the hit (so death is handled normally). */
    tmob  = NULL;
    mvnum = TRIAL_MOB_FIRST + (tier-1);
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
      if ( IS_NPC(rch) && rch->pIndexData != NULL
        && rch->pIndexData->vnum == mvnum )
      { tmob = rch;
        break;
      }
    }

    send_to_char( def->fail, ch );
    send_to_char( "\n\r", ch );

    if ( tmob != NULL )
      damage( tmob, ch, dam, TYPE_UNDEFINED );
    else
    {
      /* No mob present (shouldn't happen): guarded direct loss. */
      ch->hit -= dam;
      update_pos( ch );
    }
  }

  return;
}
