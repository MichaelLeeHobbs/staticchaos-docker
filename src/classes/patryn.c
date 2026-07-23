/* Original by Alathon */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

void	add_rune	args( ( CHAR_DATA *ch, int type, int part ) );
void	remove_rune	args( ( CHAR_DATA *ch, int type, int part ) );
void	runecast	args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *arg ) );

const	int	runemax[5] = { 15, 5, 5, 8, 8 };

/* Maps a RUNE_* bit value to its name; used by do_runes and runeweave list.
   Indexed by { bit, name }; primaries carry a trainable strength, secondaries
   do not.  RUNE_LIFE.. are also the result of rune1+rune2 sums. */
struct rune_name_type
{
  int		bit;
  char *	name;
};

const struct rune_name_type primary_runes[] =
{
  { RUNE_AIR,		"air"		},
  { RUNE_EARTH,		"earth"		},
  { RUNE_FIRE,		"fire"		},
  { RUNE_WATER,		"water"		},
  { RUNE_ENERGY,	"energy"	},
  { RUNE_NEGATIVE,	"negative"	},
  { 0,			NULL		}
};

const struct rune_name_type secondary_runes[] =
{
  { RUNE_LIFE,		"life"		},
  { RUNE_DEATH,		"death"		},
  { RUNE_CREATION,	"creation"	},
  { RUNE_DESTRUCTION,	"destruction"	},
  { RUNE_PROTECTION,	"protection"	},
  { RUNE_TRANSFORMATION, "transformation"},
  { RUNE_MOVEMENT,	"movement"	},
  { RUNE_ABJURATION,	"abjuration"	},
  { 0,			NULL		}
};

/* Power index (P_AIR..) for each primary rune, parallel to primary_runes. */
const int primary_power[] =
{ P_AIR, P_EARTH, P_FIRE, P_WATER, P_ENERGY, P_NEGATIVE };

/* Table-driven runeweave dispatch (#refactor): each rune combo's effect body
   lives in its own static rw_* handler below, and cost_table carries a pointer
   to it in the SAME fixed order as the {spell,cost,target,wait,effect} data.
   A handler returns TRUE to continue into the normal post-cast bookkeeping
   (fighting/wait/mana -- the old `break`) or FALSE to abort early with no mana
   or wait-state spent (the old bare `return`). */
typedef bool RW_FUN args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* Shared "default" casting messages (were local def1/def2 in do_runeweave). */
static char rw_def1[] = "You move your hands in mystick patterns, uttering runes under your breath.";
static char rw_def2[] = "$n moves $s hands in a complex pattern, while muttering broken phrases.";

static	bool	rw_air_life		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_earth_life		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_fire_life		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_water_life		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_energy_life		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_negative_life	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_air_death		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_fire_death		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_water_creation	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_air_creation		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_energy_creation	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_air_destruction	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_fire_destruction	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_energy_destruction	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_negative_destruction	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_air_protection	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_earth_protection	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_fire_protection	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_water_protection	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_negative_protection	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_air_transformation	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_fire_transformation	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_energy_transformation args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_negative_transformation args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_air_movement		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_negative_movement	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_air_abjuration	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_earth_abjuration	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_fire_abjuration	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_water_abjuration	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_energy_abjuration	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static	bool	rw_negative_abjuration	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

struct cost_type
{
  int spell;
  int cost;
  int target;
  int wait;
  char *effect;		/* one-line blurb for 'runeweave list' (#99) */
  RW_FUN *handler;	/* effect body for this combo (table-driven dispatch) */
};

/* effect blurbs describe what each combo ACTUALLY does (derived from the
   switch in do_runeweave below) and imply the target: "a foe" = offensive,
   "(self)" = self, "a target" = defensive, "the room" = room. */
const	struct	cost_type	cost_table	[]	=
{
  {	65,	300,	TAR_CHAR_OFFENSIVE,	9,	"hampers escape: harder to flee, +1 lag (mark)",	rw_air_life		},
  {	66,	300,	TAR_CHAR_OFFENSIVE,	9,	"+10% physical damage taken (mark)",	rw_earth_life		},
  {	68,	300,	TAR_CHAR_OFFENSIVE,	9,	"+12% fire damage taken (mark)",	rw_fire_life		},
  {	72,	300,	TAR_CHAR_OFFENSIVE,	9,	"-15% regen & healing received (mark)",	rw_water_life		},
  {	80,	750,	TAR_CHAR_DEFENSIVE,	10,	"heals HP (scales with energy rune)",		rw_energy_life		},
  {	96,	400,	TAR_CHAR_OFFENSIVE,	10,	"negative damage, drains HP to you",		rw_negative_life	},
  {	129,	500,	TAR_CHAR_OFFENSIVE,	10,	"single-target lightning damage",	rw_air_death		},
  {	132,	500,	TAR_CHAR_OFFENSIVE,	10,	"single-target fire damage",		rw_fire_death		},
  {	264,	250,	TAR_CHAR_DEFENSIVE,	10,	"douses fire burning on a target",	rw_water_creation	},
  {	257,	750,	TAR_CHAR_DEFENSIVE,	10,	"ball lightning: 18d65 shock to astral foes", rw_air_creation	},
  {	272,	100,	TAR_CHAR_DEFENSIVE,	10,	"cures wounds & status (scales with energy)",		rw_energy_creation	},
  {	513,	750,	TAR_CHAR_OFFENSIVE,	9,	"gust: interrupts foe chant, strips ki/kiwall", rw_air_destruction	},
  {	516,	400,	TAR_IGNORE,		10,	"fire damage to all enemies in the room", rw_fire_destruction	},
  {	528,	600,	TAR_CHAR_OFFENSIVE,	10,	"dispels magic on a foe",		rw_energy_destruction	},
  {	544,	500,	TAR_CHAR_OFFENSIVE,	10,	"strips a Mazoku foe's dark energy",	rw_negative_destruction	},
  {	1025,	200,	TAR_CHAR_SELF,		10,	"improves armor class (-20 AC, self)",		rw_air_protection	},
  {	1026,	250,	TAR_CHAR_SELF,		10,	"improves armor class (-40 AC, self)",		rw_earth_protection	},
  {	1028,	750,	TAR_CHAR_SELF,		8,	"fully absorbs the next attack (self)", rw_fire_protection	},
  {	1032,	100,	TAR_CHAR_SELF,		10,	"improves armor class (-20 AC, self)",			rw_water_protection	},
  {	1056,	250,	TAR_CHAR_DEFENSIVE,	10,	"blocks incoming negative drain (self/ally)",	rw_negative_protection	},
  {	2049,	350,	TAR_CHAR_DEFENSIVE,	10,	"reflects incoming lightning bolts (self/ally)", rw_air_transformation },
  {	2052,	350,	TAR_CHAR_DEFENSIVE,	10,	"reflects/absorbs incoming fire (self/ally)",	rw_fire_transformation	},
  {	2064,	400,	TAR_CHAR_SELF,		10,	"toggles truesight (self)",		rw_energy_transformation },
  {	2080,	100,	TAR_CHAR_OFFENSIVE,	10,	"drains a foe's mana, half to you",		rw_negative_transformation },
  {	4097,	100,	TAR_CHAR_DEFENSIVE,	10,	"grants flight (self)",			rw_air_movement		},
  {	4128,	400,	TAR_CHAR_OFFENSIVE,	8,	"drains 10-150 move to you",		rw_negative_movement	},
  {	16385,	750,	TAR_CHAR_SELF,		8,	"reduces incoming wind/lightning damage (self)",		rw_air_abjuration	},
  {     16386,  750,    TAR_CHAR_SELF,		8,	"reduces incoming earth/physical damage (self)",		rw_earth_abjuration	},
  {     16388,  750,    TAR_CHAR_SELF,          8,	"reduces incoming fire damage (self)",		rw_fire_abjuration	},
  {     16392,  750,    TAR_CHAR_SELF,          8,	"reduces incoming water damage (self)",		rw_water_abjuration	},
  {     16400,  750,    TAR_CHAR_SELF,	        8,	"-15% incoming chant damage (self)",		rw_energy_abjuration	},
  {     16416,  750,    TAR_CHAR_SELF,          8,	"reduces incoming negative/death damage (self)",	rw_negative_abjuration	}
};


void do_learn( CHAR_DATA *ch, char *argument )
{
  int type = 0;
  int cost = 0;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "\n\rYou simply don't understand rune magic.\n\r", ch );
    return;
  }
  cost = UMIN( 200, 50 + ch->pcdata->powers[P_LEARNED_NUM] * 10 );

  if ( argument[0] == '\0' )
  { send_to_char( "\n\rYou can learn the following runes:\n\r", ch );
    send_to_char( "Primary: earth air fire water energy negative\n\r", ch );
    send_to_char( "Secondary: life death creation destruction protection\n\r", ch );
    send_to_char( "           transformation movement abjuration\n\r", ch );
    return;
  }
  else if ( !str_cmp( argument, "air" ) )
    type = RUNE_AIR;
  else if ( !str_cmp( argument, "earth" ) )
    type = RUNE_EARTH;
  else if ( !str_cmp( argument, "fire" ) )
    type = RUNE_FIRE;
  else if ( !str_cmp( argument, "water" ) )
    type = RUNE_WATER;
  else if ( !str_cmp( argument, "energy" ) )
    type = RUNE_ENERGY;
  else if ( !str_cmp( argument, "negative" ) )
    type = RUNE_NEGATIVE;
  else if ( !str_cmp( argument, "life" ) )
    type = RUNE_LIFE;
  else if ( !str_cmp( argument, "death" ) )
    type = RUNE_DEATH;
  else if ( !str_cmp( argument, "creation" ) )
    type = RUNE_CREATION;
  else if ( !str_cmp( argument, "destruction" ) )
    type = RUNE_DESTRUCTION;
  else if ( !str_cmp( argument, "protection" ) )
    type = RUNE_PROTECTION;
  else if ( !str_cmp( argument, "transformation" ) )
    type = RUNE_TRANSFORMATION;
  else if ( !str_cmp( argument, "movement" ) )
    type = RUNE_MOVEMENT;
  else if ( !str_cmp( argument, "abjuration" ) )
    type = RUNE_ABJURATION;
  else if ( !str_cmp( argument, "cost" ) )
  { sprintf( buf, "\n\rIt will cost you %d primal to learn another rune.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }
  else if ( !str_cmp( argument, "learned" ) )
  {
    return;
  }
  else
  { send_to_char( "Thats not a rune.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->pcdata->powers[P_LEARNED],type) )
  { send_to_char( "But you already know that rune!\n\r", ch );
    return;
  }
  if ( ch->pcdata->primal < cost )
  { send_to_char( "You don't have enough primal to learn another rune.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= cost;
  SET_BIT(ch->pcdata->powers[P_LEARNED],type);
  ch->pcdata->powers[P_LEARNED_NUM]++;
  send_to_char( "You have learned how to use a new rune!\n\r", ch );
  return;
}

void do_runes( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  int learn_cost;
  int i, pidx, strength, train_cost;
  bool known;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "You simply don't understand rune magic.\n\r", ch );
    return;
  }

  learn_cost = UMIN( 200, 50 + ch->pcdata->powers[P_LEARNED_NUM] * 10 );

  send_to_char( "\n\rPrimary runes (strength can be raised with 'runetrain'):\n\r", ch );
  send_to_char( "Rune        Status   Strength  Cost\n\r", ch );
  send_to_char( "------------------------------------------------\n\r", ch );
  for ( i = 0; primary_runes[i].name != NULL; i++ )
  {
    known = IS_SET( ch->pcdata->powers[P_LEARNED], primary_runes[i].bit );
    pidx  = primary_power[i];
    strength = ch->pcdata->powers[pidx];
    if ( !known )
    { sprintf( buf, "%-10s  unknown    %3d/100  %d primal to learn\n\r",
        primary_runes[i].name, strength, learn_cost );
    }
    else if ( strength >= 100 )
    { sprintf( buf, "%-10s  known      %3d/100  (mastered)\n\r",
        primary_runes[i].name, strength );
    }
    else
    { train_cost = 150 * (strength+1) * (strength+1);
      sprintf( buf, "%-10s  known      %3d/100  %d exp to train\n\r",
        primary_runes[i].name, strength, train_cost );
    }
    send_to_char( buf, ch );
  }

  send_to_char( "\n\rSecondary runes (no strength; learned only):\n\r", ch );
  send_to_char( "Rune            Status   Cost\n\r", ch );
  send_to_char( "------------------------------------------------\n\r", ch );
  for ( i = 0; secondary_runes[i].name != NULL; i++ )
  {
    known = IS_SET( ch->pcdata->powers[P_LEARNED], secondary_runes[i].bit );
    if ( !known )
    { sprintf( buf, "%-14s  unknown    %d primal to learn\n\r",
        secondary_runes[i].name, learn_cost );
    }
    else
    { sprintf( buf, "%-14s  known      -\n\r", secondary_runes[i].name );
    }
    send_to_char( buf, ch );
  }
  send_to_char( "\n\r", ch );
  return;
}

static bool rw_air_life( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_LIFE */
  AFFECT_DATA af;

    	if ( is_affected(victim,gsn_wind_curse) )
    	{
    	  stc( "They are already so cursed.\n\r", ch );
    	  return TRUE;
    	}
    	/* #23: Holy Bless (white-spec) ward -- 5% chance to shrug off the curse */
    	if ( holy_bless_wards( ch, victim, "Ceiphied's blessing wards it off!\n\r",
    	       "Ceiphied's blessing wards off your runeweave!" ) )
    	  return TRUE;
    	/* only one elemental curse at a time: strip the other three */
    	affect_strip( victim, gsn_earth_curse );
    	affect_strip( victim, gsn_flame_curse );
    	affect_strip( victim, gsn_water_curse );
    	act( "A pale blue mist settles around $N.", ch, NULL, victim, TO_CHAR );
    	act( "A pale blue mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_wind_curse;
        af.location  = 0;
        af.modifier  = 0;
        /* PvP duration cap: 3 ticks vs players, Mind-scaled vs NPCs */
        af.duration  = !IS_NPC(victim) ? 3 : ch->pcdata->mind *2/3;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_earth_life( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_EARTH + RUNE_LIFE */
  AFFECT_DATA af;

        if ( is_affected(victim,gsn_earth_curse) )
        {
          stc( "They are already so cursed.\n\r", ch );
          return TRUE;
        }
        /* #23: Holy Bless (white-spec) ward -- 5% chance to shrug off the curse */
        if ( holy_bless_wards( ch, victim, "Ceiphied's blessing wards it off!\n\r",
               "Ceiphied's blessing wards off your runeweave!" ) )
          return TRUE;
        /* only one elemental curse at a time: strip the other three */
        affect_strip( victim, gsn_wind_curse );
        affect_strip( victim, gsn_flame_curse );
        affect_strip( victim, gsn_water_curse );
        act( "A dull yellow mist settles around $N.", ch, NULL, victim, TO_CHAR );
        act( "A dull yellow mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_earth_curse;
        af.location  = 0;
        af.modifier  = 0;
        /* PvP duration cap: 3 ticks vs players, Mind-scaled vs NPCs */
        af.duration  = !IS_NPC(victim) ? 3 : ch->pcdata->mind *2/3;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_fire_life( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_FIRE + RUNE_LIFE */
  AFFECT_DATA af;

        if ( is_affected(victim,gsn_flame_curse) )
        {
          stc( "They are already so cursed.\n\r", ch );
          return TRUE;
        }
        /* #23: Holy Bless (white-spec) ward -- 5% chance to shrug off the curse */
        if ( holy_bless_wards( ch, victim, "Ceiphied's blessing wards it off!\n\r",
               "Ceiphied's blessing wards off your runeweave!" ) )
          return TRUE;
        /* only one elemental curse at a time: strip the other three */
        affect_strip( victim, gsn_wind_curse );
        affect_strip( victim, gsn_earth_curse );
        affect_strip( victim, gsn_water_curse );
        act( "A blood red mist settles around $N.", ch, NULL, victim, TO_CHAR );
        act( "A blood red mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_flame_curse;
        af.location  = 0;
        af.modifier  = 0;
        /* PvP duration cap: 3 ticks vs players, Mind-scaled vs NPCs */
        af.duration  = !IS_NPC(victim) ? 3 : ch->pcdata->mind *2/3;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_water_life( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_WATER + RUNE_LIFE */
  AFFECT_DATA af;

        if ( is_affected(victim,gsn_water_curse) )
        {
          stc( "They are already so cursed.\n\r", ch );
          return TRUE;
        }
        /* #23: Holy Bless (white-spec) ward -- 5% chance to shrug off the curse */
        if ( holy_bless_wards( ch, victim, "Ceiphied's blessing wards it off!\n\r",
               "Ceiphied's blessing wards off your runeweave!" ) )
          return TRUE;
        /* only one elemental curse at a time: strip the other three */
        affect_strip( victim, gsn_wind_curse );
        affect_strip( victim, gsn_earth_curse );
        affect_strip( victim, gsn_flame_curse );
        act( "A murky mist settles around $N.", ch, NULL, victim, TO_CHAR );
        act( "A murky mist settles around $N.", ch, NULL, victim, TO_ROOM );
        af.type      = gsn_water_curse;
        af.location  = 0;
        af.modifier  = 0;
        /* PvP duration cap: 3 ticks vs players, Mind-scaled vs NPCs */
        af.duration  = !IS_NPC(victim) ? 3 : ch->pcdata->mind *2/3;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_energy_life( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_ENERGY + RUNE_LIFE */
  int i;

	act( rw_def1, ch, NULL, victim, TO_CHAR );
	act( rw_def2, ch, NULL, victim, TO_ROOM );
	for ( i = 0; i < ch->pcdata->powers[P_ENERGY]; i += 33 )
	  runecast( ch, victim, "heal" );
	return TRUE;
}

static bool rw_negative_life( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_NEGATIVE + RUNE_LIFE */
  int dam = 0;
  int i;

    	if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
    	{
    	  if ( victim == ch )
    	  {
    	    stc( "You failed.\n\r", ch );
    	    return FALSE;
    	  }
    	  act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_CHAR );
    	  act( "Dark energy streaks toward and is absorbed by you!", ch, NULL, victim, TO_VICT );
    	  act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_NOTVICT );
	  victim->hit += dice( ch->pcdata->mind/8, ch->pcdata->powers[P_NEGATIVE] );
	  victim->hit = UMIN( victim->hit, victim->max_hit );
    	}
    	else
    	{
    	  act( "Dark energy streaks from you to $N, and rebounds!", ch, NULL, victim,TO_CHAR);
    	  act( "Dark energy streaks from $n to you, and rebounds to $m!", ch, NULL, victim, TO_VICT );
    	  act( "Dark energy streaks from $n to $N, then rebounds.", ch, NULL, victim, TO_NOTVICT );
    	  dam = dice( ch->pcdata->mind/4, ch->pcdata->powers[P_NEGATIVE] );
    	  i = victim->hit;
    	  damage( ch, victim, dam, DAM_NEGATIVE );
    	  if ( victim != NULL && victim->hit > 0 )
    	    ch->hit = UMIN( ch->max_hit, ch->hit + (i - victim->hit) );
	}
    	return TRUE;
}

static bool rw_air_death( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_DEATH */
  int dam = 0;

    	act( "A bolt of lightning leaps from your fingers toward $N!", ch, NULL, victim, TO_CHAR );
    	act( "A bolt of lightning leaps from $n's fingers toward you!", ch, NULL, victim, TO_VICT );
    	act( "A bolt of lightning leaps from $n's fingers toward $N!", ch, NULL, victim, TO_NOTVICT );
        if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_AIR_BLOCK) )
        { act( "The lightning bolt rebounds!", ch, NULL, NULL, TO_CHAR );
          act( "The lightning bolt rebounds!", ch, NULL, NULL, TO_ROOM );
          dam = dice( ch->pcdata->mind / 4, ch->pcdata->powers[P_AIR] );
	  damage( ch, ch, dam, DAM_LIGHTNING );
          return TRUE;
        }
        else
        {
	  dam = dice( ch->pcdata->mind / 2, ch->pcdata->powers[P_AIR] );
	  damage( ch, victim, dam, DAM_LIGHTNING );
	}
	return TRUE;
}

static bool rw_fire_death( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_FIRE + RUNE_DEATH */
  int dam = 0;

	act( "A lance of flame streaks from your fingers toward $N!", ch, NULL, victim, TO_CHAR );
	act( "A lance of flame streaks from $n's fingers toward you!", ch, NULL, victim, TO_VICT );
	act( "A lance of flame streaks from $n's fingers toward $N!", ch, NULL, victim, TO_NOTVICT );
	if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_FIRE_BLOCK) )
	{ act( "The flame lance rebounds!", ch, NULL, NULL, TO_CHAR );
	  act( "The flame lance rebounds!", ch, NULL, NULL, TO_ROOM );
	  dam = dice( ch->pcdata->mind / 4, ch->pcdata->powers[P_FIRE] );
	  damage( ch, ch, dam, DAM_FIRE );
	  return TRUE;
	}
	else
	{ dam = dice( ch->pcdata->mind / 2, ch->pcdata->powers[P_FIRE] );
          if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_VAN_REHL) )
          { REMOVE_BIT(victim->pcdata->actnew,NEW_VAN_REHL);
            act( "Their icy barrier melts!", ch, NULL, victim, TO_CHAR );
            act( "Your icy barrier melts!", ch, NULL, victim, TO_VICT );
          }
	  damage( ch, victim, dam, DAM_FIRE );
	}
	return TRUE;
}

static bool rw_air_creation( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_CREATION */
    	if ( IS_NPC(victim) )
    	{
    	  stc( "Nothing happens.\n\r", ch );
    	  return FALSE;
    	}
    	act( "Ball lightning crackles around $N.", ch, NULL, victim, TO_CHAR );
    	act( "Ball lightning crackles around $N.", ch, NULL, victim, TO_ROOM );
    	if ( !IS_SET(victim->pcdata->actnew,NEW_BALL_LIGHTNING) )
    	  SET_BIT(victim->pcdata->actnew,NEW_BALL_LIGHTNING);
    	return TRUE;
}

static bool rw_water_creation( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_WATER + RUNE_CREATION */
        act( rw_def1, ch, NULL, victim, TO_CHAR );
        act( rw_def2, ch, NULL, victim, TO_ROOM );
        if ( is_affected(victim,skill_lookup("flame breath")) )
        {
          affect_strip(victim,skill_lookup("flame breath"));
          act( "The flames are doused.", ch, NULL, victim, TO_CHAR );
          act( "Your skin abruptly stops burning.", ch, NULL, victim, TO_VICT );
          act( "The flames burning $N are doused.", ch, NULL, victim, TO_ROOM );
        }
        return TRUE;
}

static bool rw_energy_creation( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_ENERGY + RUNE_CREATION */
	act( rw_def1, ch, NULL, victim, TO_CHAR );
	act( rw_def2, ch, NULL, victim, TO_ROOM );
	if ( ch->pcdata->powers[P_ENERGY] < 25 )
	  runecast( ch, victim, "cure light" );
	else if ( ch->pcdata->powers[P_ENERGY] < 50 )
	  runecast( ch, victim, "cure serious" );
	else if ( ch->pcdata->powers[P_ENERGY] < 75 )
	{
	  runecast( ch, victim, "cure critical" );
	  runecast( ch, victim, "refresh" );
	}
	else
	{
	  runecast( ch, victim, "heal" );
	  runecast( ch, victim, "refresh" );
	  runecast( ch, victim, "cure poison" );
	}
	return TRUE;
}

static bool rw_air_destruction( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_DESTRUCTION */
    	act( "You launch a gust of wind into $N.", ch, NULL, victim, TO_CHAR );
    	act( "$n's gust of wind slams into you!", ch, NULL, victim, TO_VICT );
    	act( "$n's gust of wind slams into $N.", ch, NULL, victim, TO_NOTVICT );
    	if ( !IS_NPC(victim) )
    	{ if ( victim->class == CLASS_FIST )
    	  { if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
	      REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
	    if ( victim->pcdata->powers[F_KI] > 0 )
	      victim->pcdata->powers[F_KI]--;
	  }
	  else if ( IS_CLASS(victim,CLASS_SORCERER ) && victim->pcdata->chant != NULL )
	  { /* Issue #3: only acts while a chant is actually in progress (the old
	     * lose_chant was a silent no-op otherwise -- so don't strip Defense or
	     * print "chant holds" against a Sorcerer who isn't chanting).
	     * Issue #122: Defense no longer holds the chant outright; it is torn
	     * away and counts as one more -10 on the interrupt roll.  The roll is
	     * clamped to [50,85], so avoidance never exceeds 50%. */
	    int chance = 75;
	    if ( is_affected( victim, gsn_defense ) )
	    { affect_strip( victim, gsn_defense );
	      act( "Your rune-wind tears away $N's Defense!", ch, NULL, victim, TO_CHAR );
	      act( "$n's rune-wind tears away your Defense!", ch, NULL, victim, TO_VICT );
	      chance -= 10;
	    }
	    if ( IS_AFFECTED( victim, AFF_HOLY_RESIST ) != 0 ) chance -= 10;
	    if ( IS_AFFECTED( victim, AFF_VAS_GLUUDO  ) != 0 ) chance -= 10;
	    if ( IS_AFFECTED( victim, AFF_RAYWING ) != 0
	      || IS_AFFECTED( victim, AFF_WINDY_SHIELD ) != 0 ) chance -= 10;
	    chance = URANGE( 50, chance, 85 );
	    if ( number_percent() <= chance )
	    { lose_chant( victim );
	      act( "Your rune-wind shatters $N's gathering chant!", ch, NULL, victim, TO_CHAR );
	      act( "$n's rune-wind shatters your gathering chant!", ch, NULL, victim, TO_VICT );
	    }
	    else
	    { act( "$N rides out the tearing wind, chant unbroken.", ch, NULL, victim, TO_CHAR );
	      act( "You ride out the tearing wind, your chant unbroken!", ch, NULL, victim, TO_VICT );
	    }
	  }

	  if ( is_affected(victim,gsn_kiwall) )
	    affect_strip(victim,gsn_kiwall);

        }
        return TRUE;
}

static bool rw_fire_destruction( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_FIRE + RUNE_DESTRUCTION */
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam = 0;

	act( "Fire sprays forth from your hands, engulfing the room in flame!", ch, NULL, victim, TO_CHAR );
	act( "The room is is engulfed in flame!", ch, NULL, victim, TO_ROOM );
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{ vch_next = vch->next;
	  if ( vch->in_room == NULL )
	    continue;
	  if ( vch->in_room == ch->in_room )
	  { if ( vch != ch && !is_same_group( ch, vch ) )
	    { if ( !IS_NPC(vch) && IS_SET(vch->pcdata->actnew,NEW_FIRE_BLOCK) )
	      { act( "The flames flow around $N.", ch, NULL, vch, TO_CHAR );
	        act( "The flames flow around you.", ch, NULL, vch, TO_VICT );
	        act( "The flames flow around $N.", ch, NULL, vch, TO_ROOM );
	      }
	      else
	      { dam = dice( ch->pcdata->mind / 4, ch->pcdata->powers[P_FIRE] );
	        if ( !IS_NPC(vch) && IS_SET(vch->pcdata->actnew,NEW_VAN_REHL) )
	        { REMOVE_BIT(vch->pcdata->actnew,NEW_VAN_REHL);
	          act( "Their icy barrier melts!", ch, NULL, vch, TO_CHAR );
	          act( "Your icy barrier melts!", ch, NULL, vch, TO_VICT );
	        }
	        damage( ch, vch, dam, DAM_FIRE );
	      }
	    }
	  }
	}
	return TRUE;
}

static bool rw_energy_destruction( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_ENERGY + RUNE_DESTRUCTION */
	runecast( ch, victim, "dispel magic" );
	if ( victim != ch && victim->fighting == NULL )
	  victim->fighting = ch;
	return TRUE;
}

static bool rw_negative_destruction( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_NEGATIVE + RUNE_DESTRUCTION */
    	act( "You launch a pearlescent ball of light at $N.", ch, NULL, victim, TO_CHAR );
    	act( "$n launches a pearlescent ball of light at you!", ch, NULL, victim, TO_VICT );
    	act( "$n launches a pearlescent ball of light at $N.", ch, NULL, victim, TO_NOTVICT );
    	if ( !IS_CLASS(victim,CLASS_MAZOKU) )
    	  return TRUE;
    	if ( victim->pcdata->powers[M_CTIME] <= 0 )
    	  return TRUE;
    	act( ".. the dark energies surrounding $N are dispelled!", ch, NULL, victim, TO_CHAR );
    	act( ".. you lose control of your dark energies!", ch, NULL, victim, TO_VICT );
    	act( ".. the dark energies surrounding $N are dispelled!", ch, NULL, victim, TO_NOTVICT );
    	victim->pcdata->powers[M_CTIME] = 0;
    	victim->pcdata->powers[M_CTYPE] = 0;
	return TRUE;
}

static bool rw_air_protection( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_PROTECTION */
	act( rw_def1, ch, NULL, victim, TO_CHAR );
	act( rw_def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, victim, "shield" );
	return TRUE;
}

static bool rw_earth_protection( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_EARTH + RUNE_PROTECTION */
	act( rw_def1, ch, NULL, victim, TO_CHAR );
	act( rw_def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, victim, "stone skin" );
	return TRUE;
}

static bool rw_fire_protection( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_FIRE + RUNE_PROTECTION */
  AFFECT_DATA af;

        if ( is_affected( ch, gsn_balus_wall ) )
        affect_strip( ch, gsn_balus_wall );
      
        af.type      = gsn_balus_wall;
        af.duration  = 30;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = 0;
        affect_to_char( ch, &af );
	act( "A reddish shield shimmers into existance in front of you.", ch, NULL, NULL, TO_CHAR );
	act( "A reddish shield shimmers into existance in front of $n.", ch, NULL, NULL, TO_ROOM );
	return TRUE;
}

static bool rw_water_protection( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_WATER + RUNE_PROTECTION */
	act( rw_def1, ch, NULL, victim, TO_CHAR );
	act( rw_def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, victim, "armor" );
	return TRUE;
}

static bool rw_negative_protection( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_NEGATIVE + RUNE_PROTECTION */
    /* NEGATIVE + PROTECTION */
    	if ( IS_NPC(victim) )
    	{ 
    	  stc( "All your base are belong to us.\n\r", ch );
    	  return FALSE;
    	}
    	act( "A black helix swirls up around you.", victim, NULL, NULL, TO_CHAR );
    	act( "A black helix swirls up around $n.", victim, NULL, NULL, TO_ROOM );
    	if ( !IS_SET(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
    	  SET_BIT(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK);
    	return TRUE;
}

static bool rw_air_transformation( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_TRANSFORMATION */
    	if ( IS_NPC(victim) )
    	{ act( "You failed.", ch, NULL, NULL, TO_CHAR );
    	  return TRUE;
    	}
    	act( "Inverted runes of air swirl up around you.", victim, NULL, NULL, TO_CHAR );
    	act( "Oddly disfigured runes swirl up around $n.", victim, NULL, NULL, TO_ROOM );
    	if ( IS_SET(victim->pcdata->actnew,NEW_FIRE_BLOCK) )
    	{ REMOVE_BIT(victim->pcdata->actnew,NEW_FIRE_BLOCK);
    	  act( "The inverted fire runes crumble.", ch, NULL, NULL, TO_CHAR );
    	}
    	SET_BIT(victim->pcdata->actnew,NEW_AIR_BLOCK);
    	return TRUE;
}

static bool rw_fire_transformation( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_FIRE + RUNE_TRANSFORMATION */
        if ( IS_NPC(victim) )
        { act( "You failed.", ch, NULL, NULL, TO_CHAR );
          return TRUE;
        }
        act( "Inverted runes of fire ignite all around you.", victim, NULL, NULL, TO_CHAR );
        act( "Oddly disfigured runes ignite all around $n.", victim, NULL, NULL, TO_ROOM );
        if ( IS_SET(victim->pcdata->actnew,NEW_AIR_BLOCK) )
        { REMOVE_BIT(victim->pcdata->actnew,NEW_AIR_BLOCK);
          act( "The inverted air runes fade away.", ch, NULL, NULL, TO_CHAR );
        }
        SET_BIT(victim->pcdata->actnew,NEW_FIRE_BLOCK);
        return TRUE;
}

static bool rw_energy_transformation( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_ENERGY + RUNE_TRANSFORMATION */
        if ( IS_SET(ch->act, PLR_TRUESIGHT) )
        { send_to_char( "Your vision returns to normal.\n\r", ch );
          REMOVE_BIT(ch->act,PLR_TRUESIGHT);
        }
        else
        { send_to_char( "Your vision increases beyond mortal standards.\n\r", ch );
          SET_BIT(ch->act,PLR_TRUESIGHT);
        }
        return TRUE;
}

static bool rw_negative_transformation( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_NEGATIVE + RUNE_TRANSFORMATION */
  int dam = 0;

        if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
        {
          if ( victim == ch )
          {
            stc( "You failed.\n\r", ch );
            return FALSE;
          }
          act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_CHAR );
          act( "Dark energy streaks toward and is absorbed by you!", ch, NULL, victim, TO_VICT );
          act( "Dark energy streaks toward and is absorbed by $N.", ch, NULL, victim, TO_NOTVICT
);
          victim->mana += dice( ch->pcdata->mind/4, ch->pcdata->powers[P_NEGATIVE] );
          victim->mana = UMIN( victim->mana, victim->max_mana );
        }
        else
        {
          act( "Dark energy streaks from you to $N, and rebounds!", ch, NULL, victim,TO_CHAR);
          act( "Dark energy streaks from $n to you, and rebounds to $m!", ch, NULL, victim, TO_VICT );
          act( "Dark energy streaks from $n to $N, then rebounds.", ch, NULL, victim, TO_NOTVICT );
          dam = dice( ch->pcdata->mind/4, ch->pcdata->powers[P_NEGATIVE] );
          if ( IS_AFFECTED(victim,AFF_HOLY_RESIST) )
            dam -= dam * number_percent() / 100;
          if ( victim->mana > dam )
            victim->mana -= dam;
          else
          { dam = victim->mana - 1;
            victim->mana = 1;
          }
          ch->mana = UMIN( ch->max_mana, ch->mana + dam/2 );
        }
        return TRUE;
}

static bool rw_air_movement( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_MOVEMENT */
	act( rw_def1, ch, NULL, victim, TO_CHAR );
	act( rw_def2, ch, NULL, victim, TO_ROOM );
	runecast( ch, ch, "fly" );
	return TRUE;
}

static bool rw_negative_movement( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_NEGATIVE + RUNE_MOVEMENT */
  int dam = 0;

    	act( "A black haze surrounds you, sapping your strength!", ch, NULL, victim, TO_VICT );
    	act( "A black haze descends upon $N", ch, NULL, victim, TO_CHAR );
    	act( "A black haze descends upon $N", ch, NULL, victim, TO_ROOM );
    	dam = dice( 10, 15 );
    	victim->move = UMAX( victim->move - dam, 1 );
    	ch->move = UMIN( ch->move + dam, ch->max_move );
    	if ( IS_CLASS(victim, CLASS_SAIYAN) )
    	{
    	  victim->pcdata->powers[S_STRENGTH] = UMAX(
    	    victim->pcdata->powers[S_STRENGTH] - dam,
    	    victim->pcdata->powers[S_STRENGTH_MAX]/2 );
    	  victim->pcdata->powers[S_SPEED] = UMAX(
    	    victim->pcdata->powers[S_SPEED] - dam,
    	    victim->pcdata->powers[S_SPEED_MAX]/2 );
    	  victim->pcdata->powers[S_AEGIS] = UMAX(
    	    victim->pcdata->powers[S_AEGIS] - dam,
    	    victim->pcdata->powers[S_AEGIS_MAX]/2 );
    	}
    	return TRUE;
}

static bool rw_air_abjuration( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_AIR + RUNE_ABJURATION */
  AFFECT_DATA af;

   	act( "A dome of azure energy encases you.", ch, NULL, victim, TO_CHAR );
   	act( "A dome of azure energy encases $n.", ch, NULL, victim, TO_ROOM );
	if ( is_affected( ch, gsn_earth_ward ) )
	  affect_strip( ch, gsn_earth_ward );
	if ( is_affected( ch, gsn_wind_ward ) )
	  affect_strip( ch, gsn_wind_ward ) ;
	if ( is_affected( ch, gsn_flame_ward ) )
	  affect_strip( ch, gsn_flame_ward );
	if ( is_affected( ch, gsn_water_ward ) )
	  affect_strip( ch, gsn_water_ward );
	if ( is_affected( ch, gsn_spirit_ward ) )
	  affect_strip( ch, gsn_spirit_ward );
	if ( is_affected( ch, gsn_negative_ward ) )
	  affect_strip( ch, gsn_negative_ward );
	af.type      = gsn_wind_ward;
   	af.location  = 0;
   	af.modifier  = 0;
   	af.duration  = ch->pcdata->mind*15;
   	af.bitvector = 0;
   	affect_to_char( ch, &af );
   	return TRUE;
}

static bool rw_earth_abjuration( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_EARTH + RUNE_ABJURATION */
  AFFECT_DATA af;

        act( "A dome of verdant energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of verdant energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_earth_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_fire_abjuration( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_FIRE + RUNE_ABJURATION */
  AFFECT_DATA af;

        act( "A dome of crimson energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of crimson energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_flame_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15; 
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_water_abjuration( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_WATER + RUNE_ABJURATION */
  AFFECT_DATA af;

        act( "A dome of sea green energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of sea green energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_water_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_energy_abjuration( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_ENERGY + RUNE_ABJURATION */
  AFFECT_DATA af;

        act( "A dome of pale white energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of pale white energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_spirit_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

static bool rw_negative_abjuration( CHAR_DATA *ch, CHAR_DATA *victim )
{ /* case RUNE_NEGATIVE + RUNE_ABJURATION */
  AFFECT_DATA af;

        act( "A dome of obsidian energy encases you.", ch, NULL, victim, TO_CHAR );
        act( "A dome of obsidian energy encases $n.", ch, NULL, victim, TO_ROOM );
        if ( is_affected( ch, gsn_earth_ward ) )
          affect_strip( ch, gsn_earth_ward );
        if ( is_affected( ch, gsn_flame_ward ) )
          affect_strip( ch, gsn_flame_ward );
        if ( is_affected( ch, gsn_water_ward ) )
          affect_strip( ch, gsn_water_ward );
        if ( is_affected( ch, gsn_spirit_ward ) )
          affect_strip( ch, gsn_spirit_ward );
        if ( is_affected( ch, gsn_wind_ward ) )
          affect_strip( ch, gsn_wind_ward );
        if ( is_affected( ch, gsn_negative_ward ) )
          affect_strip( ch, gsn_negative_ward );
        af.type      = gsn_negative_ward;
        af.location  = 0;
        af.modifier  = 0;
        af.duration  = ch->pcdata->mind*15;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        return TRUE;
}

void do_runeweave( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim = NULL;
  int rune1, rune2, spell, slot, cost, target = TAR_CHAR_SELF;
  bool match = FALSE;


  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN || ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  /* arguments out of order for kludgy targeting */
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg1 );

  if ( !str_cmp( arg2, "list" ) )
  {
    char rn1[MAX_STRING_LENGTH];
    char rn2[MAX_STRING_LENGTH];
    int  p, s, sp;

    send_to_char( "\n\rThe following runeweaves are known:\n\r", ch );
    send_to_char( "Rune 1 + Rune 2          Mana  Effect\n\r", ch );
    send_to_char( "------------------------------------------------------------------------\n\r", ch );
    for ( slot = 0; slot < MAX_RUNESPELLS; slot++ )
    {
      sp = cost_table[slot].spell;
      strcpy( rn1, "?" );
      strcpy( rn2, "?" );
      for ( p = 0; primary_runes[p].name != NULL; p++ )
        if ( IS_SET( sp, primary_runes[p].bit ) )
          strcpy( rn1, primary_runes[p].name );
      for ( s = 0; secondary_runes[s].name != NULL; s++ )
        if ( IS_SET( sp, secondary_runes[s].bit ) )
          strcpy( rn2, secondary_runes[s].name );
      snprintf( buf, sizeof( buf ), "%-8s + %-14s %5d  %s\n\r",
        rn1, rn2, cost_table[slot].cost, cost_table[slot].effect );
      send_to_char( buf, ch );
    }
    return;
  }

  if ( arg2[0] == '\0' || arg3[0] == '\0' )
  { send_to_char( "You must specify two runes to weave.\n\r", ch );
    return;
  }


  if ( !str_prefix( arg2, "air" ) )
    rune1 = RUNE_AIR;
  else if ( !str_prefix( arg2, "earth" ) )
    rune1 = RUNE_EARTH;
  else if ( !str_prefix( arg2, "fire" ) )
    rune1 = RUNE_FIRE;
  else if ( !str_prefix( arg2, "water" ) )
    rune1 = RUNE_WATER;
  else if ( !str_prefix( arg2, "energy" ) )
    rune1 = RUNE_ENERGY;
  else if ( !str_prefix( arg2, "negative" ) )
    rune1 = RUNE_NEGATIVE;
  else
  { sprintf( buf, "You know of no rune of %s.\n\r", arg2 );
    send_to_char( buf, ch );
    return;
  }

  if ( !str_prefix( arg3, "life" ) )
    rune2 = RUNE_LIFE;
  else if ( !str_prefix( arg3, "death" ) )
    rune2 = RUNE_DEATH;
  else if ( !str_prefix( arg3, "creation" ) )
    rune2 = RUNE_CREATION;
  else if ( !str_prefix( arg3, "destruction" ) )
    rune2 = RUNE_DESTRUCTION;
  else if ( !str_prefix( arg3, "protection" ) )
    rune2 = RUNE_PROTECTION;
  else if ( !str_prefix( arg3, "transformation" ) )
    rune2 = RUNE_TRANSFORMATION;
  else if ( !str_prefix( arg3, "movement" ) )
    rune2 = RUNE_MOVEMENT;
  else if ( !str_prefix( arg3, "abjuration" ) )
    rune2 = RUNE_ABJURATION;
  else
  { sprintf( buf, "You know of no rune of %s.\n\r", arg3 );
    send_to_char( buf, ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[P_LEARNED],rune1) || !IS_SET(ch->pcdata->powers[P_LEARNED],rune2) )
  { send_to_char( "You have not learned the uses of that rune.\n\r", ch );
    return;
  }

  spell = rune1 + rune2;
  slot = 0; cost = 30000;
  while ( slot < MAX_RUNESPELLS && !match )
  {
    if ( cost_table[slot].spell == spell )
    {
      match = TRUE;
      cost = cost_table[slot].cost;
      target = cost_table[slot].target;
    }
    else
     slot++;
  }

  if ( !match )
  { send_to_char( "There is no such spell..yet.\n\r", ch );
    return;
  }

  /* Patryn defenses raised: offensive runeweave mana +15% (round up). */
  if ( target == TAR_CHAR_OFFENSIVE
    && IS_SET(ch->pcdata->powers[P_BITS], P_DEFENSES) )
    cost = ( cost * 115 + 99 ) / 100;

  if ( ch->mana < cost )
  { send_to_char( "You don't have enough mana.\n\r", ch );
    return;
  }

  if ( cost_table[slot].target == TAR_CHAR_SELF || cost_table[slot].target == TAR_IGNORE )
    victim = ch;
  else if ( cost_table[slot].target == TAR_CHAR_OFFENSIVE )
  { if ( arg1[0] == '\0' )
    { if ( ch->fighting != NULL )
        victim = ch->fighting;
      else
      { send_to_char( "Weave at who?\n\r", ch );
        return;
      }
    }
    else if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      WAIT_STATE( ch, 4 );
      return;
    }
    timer_check( ch, victim );
  }
  else
  { if ( arg1[0] == '\0' )
      victim = ch;
    else if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    { send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  }

  if ( !IS_NPC(victim) && victim->level < 2 )
  { send_to_char( "No runeweaving on mortals.\n\r", ch );
    return;
  }

  if ( cost_table[slot].handler == NULL )
  { send_to_char( "Nothing happens.\n\r", ch );
    return;
  }
  if ( !cost_table[slot].handler( ch, victim ) )
    return;

  if ( victim != NULL && target == TAR_CHAR_OFFENSIVE && victim != ch
    && victim->position >= POS_STUNNED )
  { if ( ch->fighting == NULL )
      ch->fighting = victim;
    if ( victim->fighting == NULL )
      victim->fighting = ch;
  }
  WAIT_STATE( ch, cost_table[slot].wait );
  ch->mana -= cost;
  return;
}

void do_tattoo( CHAR_DATA *ch, char *argument )
{
  int cost, mcost, location, type;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "You know nothing of the ancient art of tattooing.\n\r", ch );
    return;
  }
  if ( ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  { send_to_char( "You must specify both a location, and a rune to tattoo.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "torso" ) )
    location = TORSO;
  else if ( !str_cmp( arg1, "leftarm" ) )
    location = LEFTARM;
  else if ( !str_cmp( arg1, "rightarm" ) )
    location = RIGHTARM;
  else if ( !str_cmp( arg1, "leftleg" ) )
    location = LEFTLEG;
  else if ( !str_cmp( arg1, "rightleg" ) )
    location = RIGHTLEG;
  else
  { send_to_char( "\n\rYou can tattoo runes on the following bodyparts:\n\r", ch );
    send_to_char( "Torso, leftarm, rightarm, leftleg, rightleg.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg2, "earth" ) )
    type = RUNE_EARTH;
  else if ( !str_cmp( arg2, "air" ) )
    type = RUNE_AIR;
  else if ( !str_cmp( arg2, "fire" ) )
    type = RUNE_FIRE;
  else if ( !str_cmp( arg2, "water" ) )
    type = RUNE_WATER;
  else
  { send_to_char( "\n\rYou can tattoo the following runes onto your body:\n\r", ch );
    send_to_char( "Earth, air, fire, water.\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->pcdata->powers[P_LEARNED],type) )
  { send_to_char( "\n\rBut you don't know that rune!\n\r", ch );
    return;
  }

  cost = 10 + get_runes( ch, RUNE_ALL, location ) * 10;
  if ( ch->pcdata->primal < cost )
  { send_to_char( "You don't have enough primal.\n\r", ch );
    return;
  }
  mcost = cost * 25;
  if ( ch->mana < mcost )
  { send_to_char( "You don't have enough mana.\n\r", ch );
    return;
  }

  if ( get_runes( ch, RUNE_ALL, location ) >= runemax[location] )
  { send_to_char( "There is no more space for another rune there.\n\r", ch );
    return;
  }

  add_rune( ch, type, location );
  ch->pcdata->primal -= cost;
  ch->mana -= mcost;
  sprintf( buf, "\n\rYou tattoo a rune of %s onto your %s.\n\r", arg2, arg1 );
  send_to_char( buf, ch );
  WAIT_STATE( ch, 12 );
  return;
}

int get_runes( CHAR_DATA *ch, int type, int part )
{
  int i;
  int num = 0;

  if ( IS_NPC(ch) )
    return 0;
  if ( ch->class != CLASS_PATRYN )
    return 0;
  if ( part > 4 || part < 0 )
    return 0;

  if ( type != RUNE_ALL )
  {
    for ( i = 0; i < runemax[part]; i++ )
      if ( ch->pcdata->runes[part][i] == type )
	num++;
  }
  else
  {
    for ( i = 0; i < runemax[part]; i++ )
      if ( ch->pcdata->runes[part][i] != 0 )
	num++;
  }

  return URANGE( 0, num, 15 );
}


/* Highest-priority active rune block (Air > Fire > Negative), or 0.  On a
   non-zero return *name is set to "air"/"fire"/"negative"; left untouched
   otherwise.  Single source of truth for the block-cascade order (#25). */
int patryn_active_block( CHAR_DATA *ch, char **name )
{
  if ( !IS_CLASS(ch,CLASS_PATRYN) )
    return 0;

  if ( IS_SET(ch->pcdata->actnew,NEW_AIR_BLOCK) )
  { *name = "air";      return NEW_AIR_BLOCK; }
  if ( IS_SET(ch->pcdata->actnew,NEW_FIRE_BLOCK) )
  { *name = "fire";     return NEW_FIRE_BLOCK; }
  if ( IS_SET(ch->pcdata->actnew,NEW_NEGATIVE_BLOCK) )
  { *name = "negative"; return NEW_NEGATIVE_BLOCK; }

  return 0;
}


/* Highest-priority active ward's gsn, or 0.  Priority order:
   Spirit > Earth > Flame > Wind > Water > Negative (#25). */
int patryn_active_ward( CHAR_DATA *ch )
{
  if ( !IS_CLASS(ch,CLASS_PATRYN) )
    return 0;

  if ( is_affected(ch,gsn_spirit_ward)   ) return gsn_spirit_ward;
  if ( is_affected(ch,gsn_earth_ward)    ) return gsn_earth_ward;
  if ( is_affected(ch,gsn_flame_ward)    ) return gsn_flame_ward;
  if ( is_affected(ch,gsn_wind_ward)     ) return gsn_wind_ward;
  if ( is_affected(ch,gsn_water_ward)    ) return gsn_water_ward;
  if ( is_affected(ch,gsn_negative_ward) ) return gsn_negative_ward;

  return 0;
}


void add_rune( CHAR_DATA *ch, int type, int part )
{
  int i = 0;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
    return;

  if ( get_runes( ch, RUNE_ALL, part ) >= runemax[part] )
    return;
  while ( ch->pcdata->runes[part][i] != 0 && i < runemax[part] - 1 )
    i++;
  ch->pcdata->runes[part][i] = type;
  return;
}

void remove_rune( CHAR_DATA *ch, int type, int part )
{
  int i = 0;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
    return;

  if ( get_runes( ch, type, part ) == 0 )
    return;

  while ( ch->pcdata->runes[part][i] != type )
    i++;

  ch->pcdata->runes[part][i] = 0;
  return;
}

/* #7: print an active ward and its remaining duration, if present. */
void runestats_ward( CHAR_DATA *ch, int gsn, char *name )
{
  AFFECT_DATA *paf;
  char buf[MAX_STRING_LENGTH];

  for ( paf = ch->affected; paf != NULL; paf = paf->next )
    if ( paf->type == gsn )
    { /* ward "duration" is an absorption pool (mind*15), drained in big chunks
       * per blocked hit -- it is strength, not time, so don't call it ticks. */
      sprintf( buf, "  Active ward: %-8s (strength ~%d)\n\r", name, paf->duration );
      send_to_char( buf, ch );
      return;
    }
}

void do_runestats( CHAR_DATA *ch, char *argument )
{
  int part, rune;
  char *message[5][15];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "What do you think you are, a Patryn?\n\r", ch );
    return;
  }

  for ( part = 0; part < 5; part++ )
    for ( rune = 0; rune < runemax[part]; rune++ )
    { if ( ch->pcdata->runes[part][rune] == RUNE_AIR )
	message[part][rune] = "air";
      else if ( ch->pcdata->runes[part][rune] == RUNE_EARTH )
	message[part][rune] = "earth";
      else if ( ch->pcdata->runes[part][rune] == RUNE_FIRE )
	message[part][rune] = "fire";
      else if ( ch->pcdata->runes[part][rune] == RUNE_WATER )
	message[part][rune] = "water";
      else if ( ch->pcdata->runes[part][rune] == RUNE_NONE )
	message[part][rune] = "none";
      else
	message[part][rune] = "?err?";
    }

  send_to_char( "\n\rYou have the following runes on you:\n\r", ch );

  sprintf( buf, "Torso: %s %s %s %s %s %s %s %s\n\r",
	message[0][0],message[0][1],message[0][2],message[0][3],
	message[0][4],message[0][5],message[0][6],message[0][7] );
  send_to_char( buf, ch );
  sprintf( buf, "       %s %s %s %s %s %s %s\n\r",
	message[0][8],message[0][9],message[0][10],message[0][11],
	message[0][12],message[0][13],message[0][14] );
  send_to_char( buf, ch );
  sprintf( buf, "Left Arm: %s %s %s %s %s\n\r",
	message[1][0],message[1][1],message[1][2],message[1][3],message[1][4] );
  send_to_char( buf, ch );
  sprintf( buf, "Right Arm: %s %s %s %s %s\n\r",
	message[2][0],message[2][1],message[2][2],message[2][3],message[2][4] );
  send_to_char( buf, ch );
  sprintf( buf, "Left Leg: %s %s %s %s %s %s %s %s\n\r",
	message[3][0],message[3][1],message[3][2],message[3][3],
	message[3][4],message[3][5],message[3][6],message[3][7] );
  send_to_char( buf, ch );
  sprintf( buf, "Right Leg: %s %s %s %s %s %s %s %s\n\r\n\r",
	message[4][0],message[4][1],message[4][2],message[4][3],
	message[4][4],message[4][5],message[4][6],message[4][7] );
  send_to_char( buf, ch );

  /* #7: derived defensive summary -- defenses state, rune layout (and what each
   * drives), active blocks, and active wards with remaining duration.  The exact
   * derived magnitudes (mitigation %, parry/melee/leg bonuses) are formula-driven
   * in fight.c and shown here as their rune inputs; exact numbers are a follow-up. */
  { int t_earth = get_runes( ch, RUNE_EARTH, TORSO );
    int t_fire  = get_runes( ch, RUNE_FIRE,  TORSO );
    int t_air   = get_runes( ch, RUNE_AIR,   TORSO );
    int t_water = get_runes( ch, RUNE_WATER, TORSO );
    int a_fire  = get_runes( ch, RUNE_FIRE,  LEFTARM ) + get_runes( ch, RUNE_FIRE,  RIGHTARM );
    int a_earth = get_runes( ch, RUNE_EARTH, LEFTARM ) + get_runes( ch, RUNE_EARTH, RIGHTARM );
    int l_earth = get_runes( ch, RUNE_EARTH, LEFTLEG ) + get_runes( ch, RUNE_EARTH, RIGHTLEG );

    send_to_char( "Defensive summary:\n\r", ch );
    sprintf( buf, "  Defenses: %s\n\r",
      IS_SET( ch->pcdata->powers[P_BITS], P_DEFENSES ) ? "raised" : "lowered" );
    send_to_char( buf, ch );
    sprintf( buf, "  Torso runes: Earth %d / Fire %d / Air %d / Water %d  (earth+fire drive damage mitigation)\n\r",
      t_earth, t_fire, t_air, t_water );
    send_to_char( buf, ch );
    sprintf( buf, "  Arm runes:   Fire %d (melee damage) / Earth %d (parry)\n\r", a_fire, a_earth );
    send_to_char( buf, ch );
    sprintf( buf, "  Leg runes:   Earth %d (reduces enemy parry/dodge/block)\n\r", l_earth );
    send_to_char( buf, ch );

    if ( IS_SET( ch->pcdata->actnew, NEW_AIR_BLOCK )
      || IS_SET( ch->pcdata->actnew, NEW_FIRE_BLOCK )
      || IS_SET( ch->pcdata->actnew, NEW_NEGATIVE_BLOCK ) )
    { sprintf( buf, "  Active block:%s%s%s\n\r",
        IS_SET( ch->pcdata->actnew, NEW_AIR_BLOCK )      ? " air"      : "",
        IS_SET( ch->pcdata->actnew, NEW_FIRE_BLOCK )     ? " fire"     : "",
        IS_SET( ch->pcdata->actnew, NEW_NEGATIVE_BLOCK ) ? " negative" : "" );
      send_to_char( buf, ch );
    }

    runestats_ward( ch, gsn_spirit_ward,   "spirit"   );
    runestats_ward( ch, gsn_earth_ward,    "earth"    );
    runestats_ward( ch, gsn_flame_ward,    "flame"    );
    runestats_ward( ch, gsn_wind_ward,     "wind"     );
    runestats_ward( ch, gsn_water_ward,    "water"    );
    runestats_ward( ch, gsn_negative_ward, "negative" );
    send_to_char( "\n\r", ch );
  }

  return;

}

void do_erase( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int rune, part;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  { send_to_char( "You must specify a body part and a rune to erase.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "torso" ) )
    part = TORSO;
  else if ( !str_cmp( arg1, "leftarm" ) )
    part = LEFTARM;
  else if ( !str_cmp( arg1, "rightarm" ) )
    part = RIGHTARM;
  else if ( !str_cmp( arg1, "leftleg" ) )
    part = LEFTLEG;
  else if ( !str_cmp( arg1, "rightleg" ) )
    part = RIGHTLEG;
  else
  { send_to_char( "That is not an acceptable body part.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg2, "earth" ) )
    rune = RUNE_EARTH;
  else if ( !str_cmp( arg2, "air" ) )
    rune = RUNE_AIR;
  else if ( !str_cmp( arg2, "fire" ) )
    rune = RUNE_FIRE;
  else if ( !str_cmp( arg2, "water" ) )
    rune = RUNE_WATER;
  else
  { send_to_char( "That is not an acceptable rune.\n\r", ch );
    return;
  }

  if ( get_runes(ch,rune,part) == 0 )
  { send_to_char( "But you have none of those runes there!\n\r", ch );
    return;
  }

  if ( ch->pcdata->primal < 25 || ch->mana < 1000 )
  { send_to_char( "It costs 25 primal and 1000 mana to erase a rune.\n\r", ch );
    return;
  }

  ch->pcdata->primal -= 25;
  ch->mana -= 1000;
  remove_rune(ch,rune,part);
  sprintf( buf, "You painstakingly remove the rune of %s from your %s.\n\r", arg2, arg1 );
  send_to_char( buf, ch );
  return;
}

void runecast( CHAR_DATA *ch, CHAR_DATA *victim, char *arg )
{
  int sn = skill_lookup( arg );
  int lev = ch->pcdata->will;

  (*skill_table[sn].spell_fun) ( sn, lev, ch, victim );

  if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE && ch != victim )
  { CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    for ( vch = char_list; vch != NULL; vch = vch_next )
    { vch_next = vch->next_in_room;
      if ( victim == vch && victim->fighting == NULL )
      { multi_hit( victim, ch, TYPE_UNDEFINED );
	break;
      }
    }
  }

  return;
}

void do_runetrain( CHAR_DATA *ch, char *argument )
{
  int rune, cost;
  char buf[MAX_STRING_LENGTH];
  char * const runenames[] = { "motherfucker", "shitface", "air", "earth", "fire", "water", "energy", "negative" };

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_PATRYN || ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  
  if ( argument[0] == '\0' )
  { send_to_char( "Your understanding of the following runes can be trained:\n\r", ch );
    sprintf( buf, "Air    %5d  Earth  %5d  Fire     %5d\n\r",
	150 * (ch->pcdata->powers[P_AIR]+1) * (ch->pcdata->powers[P_AIR]+1),
	150 * (ch->pcdata->powers[P_EARTH]+1) * (ch->pcdata->powers[P_EARTH]+1),
	150 * (ch->pcdata->powers[P_FIRE]+1) * (ch->pcdata->powers[P_FIRE]+1) );
    send_to_char( buf, ch );
    sprintf( buf, "Water  %5d  Energy %5d  Negative %5d\n\r", 
   	150 * (ch->pcdata->powers[P_WATER]+1) * (ch->pcdata->powers[P_WATER]+1),
   	150 * (ch->pcdata->powers[P_ENERGY]+1) * (ch->pcdata->powers[P_ENERGY]+1),
   	150 * (ch->pcdata->powers[P_NEGATIVE]+1) * (ch->pcdata->powers[P_NEGATIVE]+1) );
    send_to_char( buf, ch );
    return;
  }

  if ( !str_cmp( argument, "air" ) )
    rune = P_AIR;
  else if ( !str_cmp( argument, "earth" ) )
    rune = P_EARTH;
  else if ( !str_cmp( argument, "fire" ) )
    rune = P_FIRE;
  else if ( !str_cmp( argument, "water" ) )
    rune = P_WATER;
  else if ( !str_cmp( argument, "energy" ) )
    rune = P_ENERGY;
  else if ( !str_cmp( argument, "negative" ) )
    rune = P_NEGATIVE;
  else
  { send_to_char( "You can't runetrain that.\n\r", ch );
    return;
  }

  cost = 150 * (ch->pcdata->powers[rune]+1) * ( ch->pcdata->powers[rune]+1);
  if ( ch->pcdata->powers[rune] >= 100 )
  { send_to_char( "You already understand all the uses of that rune.\n\r", ch );
    return;
  }
  if ( ch->exp < cost )
  { sprintf( buf, "You need %d experience to train that rune.\n\r", cost );
    send_to_char( buf, ch );
    return;
  }

  ch->exp -= cost;
  ch->pcdata->powers[rune]++;
  sprintf( buf, "Your understanding of the rune of %s improves.\n\r", runenames[rune] );
  send_to_char( buf, ch );
  return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
  if ( !IS_CLASS( ch, CLASS_PATRYN ) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( IS_SUIT(ch) )
  { send_to_char( "Not in a mobile suit.\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_CIRCLE) )
  { send_to_char( "Your circle is already closed!\n\r", ch );
    return;
  }
  if ( ch->position < POS_RESTING || ch->position > POS_RESTING )
  { send_to_char( "You must be at rest to close the circle of your being.\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "You close the circle of your being.\n\r", ch );
    SET_BIT(ch->pcdata->actnew,NEW_CIRCLE);
    return;
  }
  else
    send_to_char( "Sorry, multi-person circling is still in the works.\n\r", ch );

  return;
}

void do_defenses( CHAR_DATA *ch, char *argument )
{ 
  char arg[MAX_INPUT_LENGTH];
  argument = one_argument( argument, arg );
  
  if ( !IS_CLASS(ch,CLASS_PATRYN) )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( ch->level < 2 )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' )
  {
    if ( IS_SET(ch->pcdata->powers[P_BITS],P_DEFENSES) )
      send_to_char( "Your defenses are raised.\n\r", ch );
    else
      send_to_char( "Your defenses are lowered.\n\r", ch );
  }
  else if ( !str_prefix( arg, "raise" ) )
  {
    if ( IS_SET(ch->pcdata->powers[P_BITS],P_DEFENSES) )
      send_to_char( "Your defenses are already raised.\n\r", ch );
    else
    { SET_BIT(ch->pcdata->powers[P_BITS],P_DEFENSES);
      send_to_char( "You raise your rune defenses, and the tattoos across your body harden with light.\n\r", ch );
      WAIT_STATE( ch, 4 );   /* #7 */
    }
  }
  else if ( !str_prefix( arg, "lower" ) )
  {
    if ( !IS_SET(ch->pcdata->powers[P_BITS],P_DEFENSES) )
      send_to_char( "Your defenses are already lowered.\n\r", ch );
    else
    { REMOVE_BIT(ch->pcdata->powers[P_BITS],P_DEFENSES);
      send_to_char( "You lower your rune defenses, letting the power flow back into your hands.\n\r", ch );
      WAIT_STATE( ch, 2 );   /* #7 */
    }
  }
  else
   send_to_char( "Acceptable arguments are raise, lower, or nothing.\n\r", ch );

  return;
}

