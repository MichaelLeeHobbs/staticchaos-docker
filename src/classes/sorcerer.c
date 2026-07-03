/* Original by Alathon */
/* Steal this and I'll assrape you with a baseball bat wrapped in barbed wire. */

#if defined (macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

bool	saves_chant	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int cn ) );

CHANT_DATA *chant_free;

#define SORC_OFFSPEC_CAP 44

/* Effective school rank for chanting: full in your specialty (and the elemental
   schools under shamanism), capped elsewhere.  Respeccing therefore makes
   off-spec ranks *inactive* rather than destroying them -- they come back at
   full strength if you specialize into that school again. */
static int sorc_rank( CHAR_DATA *ch, int school )
{
  int raw = ch->pcdata->powers[school];
  if ( ch->pcdata->powers[SORC_SPEC] == school
    || ( ch->pcdata->powers[SORC_SPEC] == SCHOOL_ASTRAL
         && school > SCHOOL_BLACK && school < SCHOOL_WHITE ) )
    return raw;
  return UMIN( raw, SORC_OFFSPEC_CAP );
}

static char *sorc_school_name( int school )
{
  switch ( school )
  { case SCHOOL_BLACK:  return "Black";
    case SCHOOL_EARTH:  return "Earth";
    case SCHOOL_WIND:   return "Wind";
    case SCHOOL_FIRE:   return "Fire";
    case SCHOOL_WATER:  return "Water";
    case SCHOOL_ASTRAL: return "Astral";
    case SCHOOL_WHITE:  return "White";
    default:            return "?";
  }
}

static char *sorc_target_name( int target )
{
  switch ( target )
  { case TAR_CHAR_OFFENSIVE: return "offensive (target may save)";
    case TAR_CHAR_DEFENSIVE: return "defensive";
    case TAR_CHAR_SELF:      return "self";
    case TAR_IGNORE:         return "special / area";
    default:                 return "?";
  }
}

/* squash a name into a match key: lower-case, no spaces or hyphens, so
   "rubyeye blade", "rubyeye-blade" and "rubyeyeblade" all compare equal. */
static void sorc_squash( char *dest, const char *src )
{
  while ( *src )
  { if ( *src != ' ' && *src != '-' )
      *dest++ = LOWER( *src );
    src++;
  }
  *dest = '\0';
}

/* display form of a chant name: spaces shown as hyphens (what you type). */
static char *sorc_hyphen( const char *name )
{
  static char buf[MAX_INPUT_LENGTH];
  char *d = buf;
  while ( *name ) { *d++ = ( *name == ' ' ) ? '-' : *name; name++; }
  *d = '\0';
  return buf;
}

/* Match player input against chant names, space/hyphen-insensitive and by
   prefix.  Returns a unique match index, or -1; on -1, the amb/namb outputs
   hold the prefix candidates (0 = none, >1 = ambiguous).  An exact squashed
   match beats longer names that merely share the prefix. */
static int sorc_match_chant( const char *raw, int amb[], int *namb )
{
  char key[MAX_INPUT_LENGTH], nk[MAX_INPUT_LENGTH];
  int i, exact = -1;

  *namb = 0;
  sorc_squash( key, raw );
  if ( key[0] == '\0' )
    return -1;
  for ( i = 0; i < MAX_CHANT; i++ )
  {
    sorc_squash( nk, chant_table[i].name );
    if ( !strcmp( nk, key ) )
      exact = i;
    if ( !str_prefix( key, nk ) )
      amb[ (*namb)++ ] = i;
  }
  if ( exact >= 0 ) { *namb = 0; return exact; }
  if ( *namb == 1 ) return amb[0];
  return -1;
}

void do_chant( CHAR_DATA *ch, char *argument )
{
  CHANT_DATA *cha;
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char tar_name[MAX_INPUT_LENGTH];
  int amb[MAX_CHANT];
  int namb = 0;
  int cn = -1;
  int i = 0;
  int rank = 0;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SORCERER )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' )
  { send_to_char( "Syntax: 'chant <spell> [target]'.  Also: 'chant list', 'chant info <spell>'.\n\r", ch );
    return;
  }

  /* 'chant list' -- chants you can currently attempt */
  if ( !str_cmp( arg1, "list" ) )
  {
    send_to_char( "`WChants you can attempt`n  (rank = required school rank = Mystic cost):\n\r", ch );
    send_to_char( "Spell                 School    Rank  Lines\n\r", ch );
    for ( i = 0; i < MAX_CHANT; i++ )
    { if ( sorc_rank( ch, chant_table[i].school ) < chant_table[i].rank )
        continue;
      sprintf( buf1, "%-20s  %-7s   %3d   %3d\n\r", sorc_hyphen( chant_table[i].name ),
          sorc_school_name( chant_table[i].school ), chant_table[i].rank, chant_table[i].lines );
      send_to_char( buf1, ch );
    }
    send_to_char( "Use 'chant info <spell>' for details.\n\r", ch );
    return;
  }

  /* 'chant info <spell>' -- details (space/hyphen-insensitive, prefix) */
  if ( !str_cmp( arg1, "info" ) )
  {
    while ( *argument == ' ' ) argument++;
    if ( *argument == '\0' )
    { send_to_char( "Usage: chant info <spell>.\n\r", ch );
      return;
    }
    cn = sorc_match_chant( argument, amb, &namb );
    if ( cn < 0 )
    { if ( namb == 0 )
        send_to_char( "No such chant.  Try 'chant list'.\n\r", ch );
      else
      { send_to_char( "Did you mean one of:\n\r", ch );
        for ( i = 0; i < namb; i++ )
        { sprintf( buf1, "  %s\n\r", sorc_hyphen( chant_table[amb[i]].name ) );
          send_to_char( buf1, ch );
        }
      }
      return;
    }
    sprintf( buf1, "`WChant:`n %s\n\r", sorc_hyphen( chant_table[cn].name ) );
    send_to_char( buf1, ch );
    sprintf( buf1, "School: %s    Required rank / Mystic cost: %d\n\r",
        sorc_school_name( chant_table[cn].school ), chant_table[cn].rank );
    send_to_char( buf1, ch );
    sprintf( buf1, "Target: %s\n\r", sorc_target_name( chant_table[cn].target ) );
    send_to_char( buf1, ch );
    sprintf( buf1, "Chant length: %d line(s)   Wait: %d   Lag: %d\n\r",
        chant_table[cn].lines, chant_table[cn].wait, chant_table[cn].lag );
    send_to_char( buf1, ch );
    sprintf( buf1, "Your effective %s rank: %d\n\r",
        sorc_school_name( chant_table[cn].school ), sorc_rank( ch, chant_table[cn].school ) );
    send_to_char( buf1, ch );
    return;
  }

  argument = one_argument( argument, arg2 );

  cn = sorc_match_chant( arg1, amb, &namb );

  /* arg1 ambiguous?  They may have typed a spaced multi-word name -- if arg1
     plus arg2 resolves to one spell, point them at the hyphenated form. */
  if ( cn < 0 && namb > 1 && arg2[0] != '\0' )
  {
    char joined[MAX_INPUT_LENGTH];
    int amb2[MAX_CHANT], namb2, cn2;
    snprintf( joined, sizeof( joined ), "%s%s", arg1, arg2 );
    cn2 = sorc_match_chant( joined, amb2, &namb2 );
    if ( cn2 >= 0 )
    {
      sprintf( buf1, "Spell names are one word -- type '%s' with a hyphen (or in quotes):\n\r   chant %s <target>\n\r",
          sorc_hyphen( chant_table[cn2].name ), sorc_hyphen( chant_table[cn2].name ) );
      send_to_char( buf1, ch );
      return;
    }
  }

  if ( cn < 0 )
  {
    if ( namb == 0 )
      send_to_char( "There is no such spell.  Try 'chant list'.\n\r", ch );
    else
    { send_to_char( "Did you mean one of:\n\r", ch );
      for ( i = 0; i < namb; i++ )
      { sprintf( buf1, "  %s\n\r", sorc_hyphen( chant_table[amb[i]].name ) );
        send_to_char( buf1, ch );
      }
    }
    return;
  }

  /* arg1 matched a (possibly multi-word) spell.  If arg2 simply continues that
     spell's name, the player typed it with a space -- guide them to the
     hyphenated form instead of mistaking the 2nd word for a target. */
  if ( arg2[0] != '\0' )
  {
    char k1[MAX_INPUT_LENGTH], k12[MAX_INPUT_LENGTH], nk[MAX_INPUT_LENGTH], joined[MAX_INPUT_LENGTH];
    sorc_squash( k1, arg1 );
    snprintf( joined, sizeof( joined ), "%s%s", arg1, arg2 );
    sorc_squash( k12, joined );
    sorc_squash( nk, chant_table[cn].name );
    if ( strlen( k12 ) > strlen( k1 ) && !str_prefix( k12, nk ) )
    {
      sprintf( buf1, "Spell names are one word -- type '%s' with a hyphen (or in quotes):\n\r   chant %s <target>\n\r",
          sorc_hyphen( chant_table[cn].name ), sorc_hyphen( chant_table[cn].name ) );
      send_to_char( buf1, ch );
      return;
    }
  }

  if ( sorc_rank( ch, chant_table[cn].school ) < chant_table[cn].rank )
  { send_to_char( "You are not learned enough to chant that.\n\r", ch );
    return;
  }

  /* Mystic is now an ammo pool: you must be able to afford the full cost, and
     the chant then lands at your *school* power (see below) -- not throttled by
     how much fuel is left.  This blocks casting when tapped out instead of the
     old behaviour of letting depleted chants resolve at feeble strength. */
  if ( ch->pcdata->powers[SORC_MYSTIC] < chant_table[cn].rank )
  { send_to_char( "Your Mystic is too depleted to weave that chant.\n\r", ch );
    return;
  }

  /* mana no longer used directly
  if ( ch->mana < chant_table[cn].cost )
  { send_to_char( "You do not have enough mana to invoke that spell.\n\r", ch );
    return;
  }
  */

  if ( chant_table[cn].lines > 1 && ch->pcdata->chant != NULL )
  {
    stc( "You cannot maintain two complex chants at once.\n\r", ch );
    return;
  }

  if ( chant_table[cn].lines > 1 && IS_SET(ch->pcdata->actnew,NEW_LAGUNABLADE) )
  {
    stc( "You cannot weave complex magic while the blade burns in your hands.\n\r", ch );
    return;
  }

  switch ( chant_table[cn].target )
  {
    case TAR_CHAR_OFFENSIVE:
      if ( arg2[0] == '\0' )
      { if ( ch->fighting == NULL )
        {
          send_to_char( "Chant at who?\n\r", ch );
          return;
        }
        else 
          victim = ch->fighting;
      }
      else
      { if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
        {
          send_to_char( "They aren't here.\n\r", ch );
          WAIT_STATE( ch, 4 );
          return;
        }
      }
      break;
    case TAR_CHAR_DEFENSIVE:
        if ( arg2[0] == '\0' )
          victim = ch;
        else if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
        { send_to_char( "They aren't here.\n\r", ch );
          return;
        }
      break;
    case TAR_CHAR_SELF:
      victim = ch;
      break;
    case TAR_IGNORE:
      victim = ch;
      break;
    default:
      victim = ch;
      break;

  }

  if ( chant_table[cn].target == TAR_CHAR_OFFENSIVE
    && !IS_NPC(victim) && victim->level < 2 )
  { send_to_char( "No chanting on mortals.\n\r", ch );
    return;
  }

  sprintf( buf1, "You chant '%s`n'", chant_table[cn].line1 );
  sprintf( buf2, "$n chants '%s`n'", chant_table[cn].line1 );

  act( buf1, ch, NULL, NULL, TO_CHAR );
  act( buf2, ch, NULL, NULL, TO_ROOM );

  if ( chant_table[cn].target == TAR_CHAR_OFFENSIVE && victim != NULL )
    timer_check( ch, victim );

  /* Chant power = your school rank (mastery), NOT clamped to remaining Mystic.
     The old 'rank = UMIN(SORC_MYSTIC, rank)' line made every cast weaker as the
     pool drained -- a death spiral.  Cost is still paid; affordability is gated
     above. */
  rank = sorc_rank( ch, chant_table[cn].school );
  ch->pcdata->powers[SORC_MYSTIC] -= chant_table[cn].rank;
  ch->pcdata->powers[SORC_MYSTIC] = UMAX(1,ch->pcdata->powers[SORC_MYSTIC]);

  if ( chant_free == NULL )
    cha             = alloc_perm( sizeof(*cha), PERM_AFF );
  else
  {
    cha             = chant_free;
    chant_free      = chant_free->next;
  }

  i = (chant_table[cn].lines > 1 ? chant_table[cn].lines - 1 : 1);

  if ( chant_table[cn].target == TAR_IGNORE )
  {
    cha->target = str_dup( arg2 );
  }
  else
  {
    one_argument( victim->name, tar_name );
    cha->target = str_dup( tar_name );
  }

  cha->cn = cn;
  cha->rank = rank;
  cha->line = 2;
  cha->wait = chant_table[cn].wait * i;

  cha->next = ch->pcdata->chant;
  ch->pcdata->chant = cha;

  WAIT_STATE( ch, chant_table[cn].lag );
  return;
}

void chant_cast( CHAR_DATA *ch, CHANT_DATA *cast )
{
  CHANT_DATA *cha;
  CHAR_DATA *victim;
  void *vo;
  int rank;

  /* Use the rank captured at chant start (Mystic was already paid then).  The
     old code recomputed rank from the now-reduced Mystic, double-counting the
     cost so expensive chants resolved weaker the more they cost. */
  rank = cast->rank;

  if ( chant_table[cast->cn].target != TAR_IGNORE )
  {
    if ( ch->fighting != NULL && is_name( cast->target, ch->fighting->name ) )
      vo = ch->fighting;  /* for fighting blind */
    else if ( (vo = get_char_room( ch, cast->target )) == NULL )
      vo = get_char_area( ch, cast->target );
  }
  else
    vo = cast->target;

  /* if we have a target, cast the chant */
  if ( vo != NULL )
  {
    (*chant_table[cast->cn].cast) ( cast->cn, rank, ch, vo );

    /* Check for casting offensive spells */
    if ( vo != NULL && vo != ch && chant_table[cast->cn].target == TAR_CHAR_OFFENSIVE )
    {
      victim = (CHAR_DATA *) vo;
      if ( victim->position > POS_STUNNED )
      {
        if ( ch->fighting == NULL )
          set_fighting( ch, victim );
        if ( victim->fighting == NULL &&
            (ch->in_room == victim->in_room || !IS_SET(victim->act,PLR_FIREBACK)) )
          set_fighting( victim, ch );
      }
    }
  }
  /* otherwise misfire -- refund half the Mystic this chant cost */
  else
  {
    ch->pcdata->powers[SORC_MYSTIC] += chant_table[cast->cn].rank / 2;
    stc( "Your chant misfired.\n\r", ch );
  }

  /* add the used up chant back to the list of available ones */
  free_string( cast->target );
  if ( ch->pcdata->chant == cast )
  {
    ch->pcdata->chant = cast->next;
    cast->next = chant_free;
    chant_free = cast;
  }
  else if ( ch->pcdata->chant != NULL )
  {
    for ( cha = ch->pcdata->chant; cha->next != cast; cha = cha->next )
      ;

    cha->next = cast->next;
    cast->next = chant_free;
    chant_free = cast;
  }

  return;
}

void lose_chant( CHAR_DATA *ch )
{
  CHANT_DATA *cha;

  if ( IS_NPC(ch) )
    return;
  if ( ch->pcdata->chant == NULL )
    return;

  cha = ch->pcdata->chant;
  ch->pcdata->chant = ch->pcdata->chant->next;
  cha->next = chant_free;
  chant_free = cha;

  return;
}


/* PvP balance: TRUE for chants whose whole purpose is breaking down a Ki Wall
   (diem wing / flow break / bomb di wind).  A Saiyan's Ki-Wall save bonus must
   NOT make those chants harder to land -- that defeats the counter-play. */
bool chant_breaks_kiwall( int cn )
{
  if ( cn < 0 || cn >= MAX_CHANT )
    return FALSE;
  return ( chant_table[cn].cast == chant_diem_wing
        || chant_table[cn].cast == chant_flow_break
        || chant_table[cn].cast == chant_bomb_di_wind );
}

bool saves_chant( CHAR_DATA *ch, CHAR_DATA *victim, int cn )
{
  int chance = 45;
  bool spec;
  if ( IS_NPC(victim) )
  {
    chance += victim->level / 3;
    if ( victim->level >= 102 )
      chance += victim->level + 50;
  }
  else
  {
    chance += victim->pcdata->will / 3;
    /* The Ki-Wall spirit bonus does not apply to wall-breaking chants. */
    if ( IS_CLASS(victim,CLASS_SAIYAN) && is_affected(victim,gsn_kiwall)
      && !chant_breaks_kiwall(cn) )
      chance += victim->pcdata->spirit;
    if ( victim->class == CLASS_PATRYN )
      chance += victim->pcdata->powers[P_AIR]/2;
    if ( victim->class == CLASS_FIST )
      chance += victim->pcdata->spirit / 5;
    if ( victim->class == CLASS_SORCERER )
    { if ( IS_AFFECTED(victim,AFF_HOLY_RESIST) )
        chance += sorc_rank( victim, SCHOOL_WHITE );
      if ( IS_AFFECTED( victim, AFF_VAS_GLUUDO) )
        chance += 10;
      if ( victim->pcdata->powers[SORC_SPEC] == chant_table[cn].school ||
          (victim->pcdata->powers[SORC_SPEC] == SCHOOL_ASTRAL &&
           chant_table[cn].school > SCHOOL_BLACK && chant_table[cn].school < SCHOOL_WHITE) )
        chance += victim->pcdata->powers[victim->pcdata->powers[SORC_SPEC]] / 2;
    }
  }

  chance -= ch->pcdata->will / 3;
  chance -= sorc_rank( ch, chant_table[cn].school );
  spec = ( ch->pcdata->powers[SORC_SPEC] == chant_table[cn].school ||
       (ch->pcdata->powers[SORC_SPEC] == SCHOOL_ASTRAL &&
        chant_table[cn].school > SCHOOL_BLACK && chant_table[cn].school < SCHOOL_WHITE) );
  if ( spec )
    chance /= 2;

  /* Floors/ceilings -- lowered from the old flat 15/85 so a specialist's
     chants land reliably, while real anti-magic bosses still resist hard. */
  {
    int lo, hi;
    if ( IS_NPC(victim) && victim->level >= 100 )
      lo = 15;            /* PvE bosses keep a higher save floor */
    else if ( spec )
      lo = 10;            /* specialized caster: spells land reliably */
    else
      lo = 12;            /* off-spec caster */
    hi = ( IS_NPC(victim) && victim->level >= 102 ) ? 85 : 75;
    chance = URANGE( lo, chance, hi );
  }

  if ( number_percent() < chance )
    return TRUE;

  return FALSE;
}

void do_research( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  int school, cost, spec, max, rank;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SORCERER )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }
  if ( argument[0] == '\0' )
  { send_to_char( "The following schools of magic may be researched:\n\r", ch );
    send_to_char( "Black, Earth, Wind, Fire, Water, Astral, White.\n\r", ch );
    send_to_char( "Use 'research list' to see your ranks, caps and next costs.\n\r", ch );
    return;
  }
  else if ( !str_cmp( argument, "list" ) )
  { static const int schools[7] =
      { SCHOOL_BLACK, SCHOOL_EARTH, SCHOOL_WIND, SCHOOL_FIRE, SCHOOL_WATER, SCHOOL_ASTRAL, SCHOOL_WHITE };
    int sc;
    send_to_char( "School    Rank   Cap   Next rank costs\n\r", ch );
    for ( sc = 0; sc < 7; sc++ )
    { int s = schools[sc];
      int specd = ( ch->pcdata->powers[SORC_SPEC] == s ||
          ( ch->pcdata->powers[SORC_SPEC] == SCHOOL_ASTRAL && s > SCHOOL_BLACK && s < SCHOOL_WHITE ) );
      int cap = specd ? 50 : SORC_OFFSPEC_CAP;
      int r   = ch->pcdata->powers[s];
      int nc  = specd ? UMIN( 75, (r+1)*5 ) : UMIN( 75, (r+1)*5 ) + (r+1);
      if ( r >= cap )
        sprintf( buf, "%-8s  %3d   %3d   (maxed)\n\r", sorc_school_name(s), r, cap );
      else
        sprintf( buf, "%-8s  %3d   %3d   %d primal\n\r", sorc_school_name(s), r, cap, nc );
      send_to_char( buf, ch );
    }
    sprintf( buf, "You have %d primal.  Specialty schools cap at 50, others at %d.\n\r",
        ch->pcdata->primal, SORC_OFFSPEC_CAP );
    send_to_char( buf, ch );
    return;
  }
  else if ( !str_cmp( argument, "black" ) )
    school = SCHOOL_BLACK;
  else if ( !str_cmp( argument, "earth" ) )
    school = SCHOOL_EARTH;
  else if ( !str_cmp( argument, "wind" ) )
    school = SCHOOL_WIND;
  else if ( !str_cmp( argument, "fire" ) )
    school = SCHOOL_FIRE;
  else if ( !str_cmp( argument, "water" ) )
    school = SCHOOL_WATER;
  else if ( !str_cmp( argument, "astral" ) )
    school = SCHOOL_ASTRAL;
  else if ( !str_cmp( argument, "white" ) )
    school = SCHOOL_WHITE;
  else
  { send_to_char( "Research what?\n\r", ch );
    return;
  }

  rank = ch->pcdata->powers[school] + 1;
  spec = ch->pcdata->powers[SORC_SPEC];

  if ( spec == school || ( spec == SCHOOL_ASTRAL && school > SCHOOL_BLACK
       && school < SCHOOL_WHITE ) )
  { max = 50;
    cost = UMIN( 75, rank*5 );
  }
  else
  { max = SORC_OFFSPEC_CAP;
    cost = UMIN( 75, rank*5 ) + rank;
  }

  if ( rank > max )
  { sprintf( buf, "You have already attained your maximum rank in %s magic.\n\r", argument );
    send_to_char( buf, ch );
    return;
  }

  if ( ch->pcdata->primal < cost )
  { sprintf( buf, "Researching rank %d of %s magic will cost you %d primal.\n\r",
	rank, argument, cost );
    send_to_char( buf, ch );
    return;
  }

  sprintf( buf, "You have achieved rank %d of %s magic.\n\r", rank, argument );
  send_to_char( buf, ch );
  ch->pcdata->primal -= cost;
  ch->pcdata->powers[school]++;
  return;
}


void do_specialize( CHAR_DATA *ch, char *argument )
{ char buf[MAX_STRING_LENGTH];
  int school = 0;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SORCERER )
  { send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { send_to_char( "You can specialize in black magic, white magic, or shamanism.\n\r", ch );
    if ( ch->pcdata->powers[SORC_SPEC] != 0 )
    { switch( ch->pcdata->powers[SORC_SPEC] )
      { case SCHOOL_BLACK:
	  send_to_char( "You are currently specialized in black magic.\n\r", ch );
	  break;
	case SCHOOL_WHITE:
	  send_to_char( "You are currently specialized in white magic.\n\r", ch );
	  break;
	case SCHOOL_ASTRAL:
	  send_to_char( "You are currently specialized in shamanism.\n\r", ch );
	  break;
	default:
	  send_to_char( "Your specialization is screwed up.\n\r", ch );
	  break;
      }
    }
    return;
  }
  else if ( !str_cmp( argument, "black magic" ) )
    school = SCHOOL_BLACK;
  else if ( !str_cmp( argument, "white magic" ) )
    school = SCHOOL_WHITE;
  else if ( !str_cmp( argument, "shamanism" ) )
    school = SCHOOL_ASTRAL;
  else
  { send_to_char( "That isn't a school of magic.\n\r", ch );
    return;
  }

  if ( ch->pcdata->powers[SORC_SPEC] == 0 )
  { sprintf( buf, "You are now specialized in %s.\n\r", argument );
    send_to_char( buf, ch );
    ch->pcdata->powers[SORC_SPEC] = school;
  }
  else
  { if ( ch->pcdata->primal < 250 )
    { send_to_char( "Changing your specialization costs 250 primal.\n\r", ch );
      return;
    }
    sprintf( buf, "You have respecialized to %s.\n\r", argument );
    send_to_char( buf, ch );
    ch->pcdata->primal -= 250;
    ch->pcdata->powers[SORC_SPEC] = school;
    /* Ranks above the off-spec cap are NOT destroyed -- sorc_rank() makes them
       inactive while out of spec, and they return at full if you spec back. */
  }

  return;
}

 
  

/*
 * Chant functions start here
 */
void chant_balus_rod( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo; 
  int dam = dice( 10, 20 + rank );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "You swing a whip of energy at $N..", ch, NULL, victim, TO_CHAR );
  act( "$n swings a whip of energy at you..", ch, NULL, victim, TO_VICT );
  act( "$n swings a whip of energy at $N..", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
}

void chant_disfang( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( 15, 20 + rank );
  if ( saves_chant( ch, victim, cn ) )
  { act( "$N skillfully dodges your shadow dragon.", ch, NULL, victim, TO_CHAR );
    act( "You skillfully dodge $n's shadow dragon.", ch, NULL, victim, TO_VICT );
    act( "$N skillfully dodges $n's shadow dragon.", ch, NULL, victim, TO_NOTVICT );
    return;
  }
  act( "You summon a dragon of shadow to attack $N.", ch, NULL, victim, TO_CHAR );
  act( "$n summons a dragon of shadow to attack you!", ch, NULL, victim, TO_VICT );
  act( "$n summons a dragon of shadow to attack $N.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
}

void chant_dolph_zork( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  OBJ_DATA *obj;

  act( "A blade of ice coalesces in your hands.", ch, NULL, NULL, TO_CHAR );
  act( "A blade of ice coalesces in $n's hands.", ch, NULL, NULL, TO_ROOM );
  obj = create_object( get_obj_index( DOLPH_ZORK ), 0 );
  obj->timer = rank + dice( 1, 4 );
  obj->value[1] = 10;
  obj->value[2] = 20;
  obj_to_char( obj, ch );
  return;
}

void chant_dynast_brass( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *vch;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch_next;
  int dam;

  act( "A lightning-wreathed pentegram erupts beneath $N!", ch, NULL, victim, TO_CHAR );
  act( "A lightning-wreathed pentegram erupts beneath you!", ch, NULL, victim, TO_VICT );
  act( "A lightning-wreathed pentegram erupts beneath $N!", ch, NULL, victim, TO_NOTVICT );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( (IS_NPC(victim) && IS_NPC(vch)) || is_same_group( victim, vch ) )
    { dam = dice( 15, 30 + rank );
      if ( saves_chant( ch, victim, cn ) )
        dam /= 2;
      chant_damage( ch, vch, dam, cn );
    }
  }
  return;
}

void chant_blast_ash( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  int chance = rank;

  chance += ch->pcdata->will / 2;

  if ( rank > 40 )
    chance += (rank - 40)*2;
  if ( IS_NPC(victim) )
  { if ( victim->level > 95 )
      chance = 0;
    else
      chance -= victim->level * 2 / 3;
  }
  else
    chance -= victim->hit / 150;

  if ( number_percent() > chance )
  { act( "$N resists your attempt to forcefully discorporate them.", ch, NULL, victim, TO_CHAR );
    act( "You resist $n's attempt to forcefully discorporate you!", ch, NULL, victim, TO_VICT );
    return;
  }

  act( "$N is engulfed by dark energy!", ch, NULL, victim, TO_CHAR );
  act( "You are engulfed by dark energy!", ch, NULL, victim, TO_VICT );
  act( "$N is engulfed by dark energy!", ch, NULL, victim, TO_NOTVICT );

  /* PvP safety cap (issue #5): the execute deals damage equal to the target's
   * full current HP -- a guaranteed kill from any HP, too swingy in PvP. Cap it
   * to non-lethal above 25% HP; true execute only once the target is at/below
   * 25%. PvE is unchanged.
   * Spec: docs/features/5-2026-06-27-blast-ash-pvp-cap.md */
  if ( IS_NPC( victim ) || victim->hit * 4 <= victim->max_hit )
  { /* PvE, or a PC already at/below 25% HP: full execute. */
    chant_damage( ch, victim, victim->hit + dice( 2, 5 ), cn );
  }
  else
  { /* PC above 25% HP: 20% current HP + rank*20, clamped so it can't kill.
     * Leave 10% headroom: chant_damage adds +10% Black-school damage to a
     * "scorched" victim (fight.c), which would otherwise push a hit-1 clamp
     * back over lethal and defeat this cap. (hit-1)*10/11 stays non-lethal
     * even after that amplification. */
    int dam = victim->hit / 5 + rank * 20;
    dam = UMIN( dam, ( victim->hit - 1 ) * 10 / 11 );
    chant_damage( ch, victim, dam, cn );
  }
  return;
}

void chant_ferrous_bleed( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int dam, count = 0;

  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if ( !saves_chant( ch, victim, cn ) )
  { act( "$N shivers and little bits of energy arc about them.", ch, NULL, victim, TO_CHAR );
    act( "Your body courses with twisted magic!", ch, NULL, victim, TO_VICT );
    act( "$N shivers and little bits of energy arc about them.", ch, NULL,victim, TO_NOTVICT);

    while ( victim->affected && count < 2 )
    { affect_remove( victim, victim->affected );
      dam = dice( 10, rank );
      chant_damage( ch, victim, dam, cn );
      count++;
    }

    /* PvP balance: if the target had nothing to strip the chant used to be a
       total no-op.  Deal a small backlash so a successful cast always bites. */
    if ( count == 0 )
    { dam = dice( 5, rank );
      chant_damage( ch, victim, dam, cn );
    }
  }

  return;
}

void chant_hell_blast( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int dam;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int oldhit;
  int heal;
  if ( number_percent() > (rank-10)*2 )
  { act( "$N dodges your bolt of negative energy!", ch, NULL, victim, TO_CHAR );
    act( "You dodge $n's bolt of negative energy!", ch, NULL, victim, TO_VICT );
    act( "$N dodges $n's bolt of negative energy.", ch, NULL, victim, TO_NOTVICT );
    return;
  }
  dam = dice( rank, rank );
  act( "Your bolt of negative energy slams into $N!", ch, NULL, victim, TO_CHAR );
  act( "$n's bolt of negative energy slams into you!", ch, NULL, victim, TO_VICT );
  act( "$n's bolt of negative energy slams into $N!", ch, NULL, victim, TO_NOTVICT );
  oldhit = victim->hit;
  chant_damage( ch, victim, dam, cn );
  if ( victim && victim->position > POS_STUNNED )
  { 
    do_say( ch, "Ahh, that's the stuff!" );
    heal = oldhit - victim->hit;
    ch->hit = UMIN( ch->max_hit, ch->hit + heal );
  }
  return;
}

void chant_zelas_brid( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int dam;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  act( "A thin beam of red energy extends toward $N.", ch, NULL, victim, TO_CHAR );
  act( "A thin beam of red energy extends toward you!", ch, NULL, victim, TO_VICT );
  act( "A thin beam of red energy extends toward $N.", ch, NULL, victim, TO_NOTVICT );
  dam = dice( rank*5/8, rank * 3 );
  chant_damage( ch, victim, dam, cn );
  return;
}

void chant_dolph_strash( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int dam;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  act( "A lance of ice streaks from your fingers to $N.", ch, NULL, victim, TO_CHAR );
  act( "A lance of ice streaks at you from $n!", ch, NULL, victim, TO_VICT );
  act( "A lance of ice streaks from $n's fingers at $N.", ch, NULL, victim, TO_NOTVICT );
  dam = dice( rank*1/2, rank * 3 );
  chant_damage( ch, victim, dam, cn );
  return;
}

void chant_ruby_eye_blade( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  OBJ_DATA *obj;
  act( "A blade of sullen red energy bursts into being!", ch, NULL, NULL, TO_CHAR );
  act( "A blade of sullen red energy bursts into being.", ch, NULL, NULL, TO_ROOM );
  obj = create_object( get_obj_index( RUBYEYE_BLADE ), 0 );
  obj->timer = (rank-25)*3 + dice( 1, 4 );
  obj->value[1] = rank;
  obj->value[2] = rank * 2;
  obj_to_char( obj, ch );
  return;
}

void chant_gaav_flare( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int dam;
  bool saved;
  bool balus_blocked = FALSE;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  act( "You launch a tremendous ball of dark flame at $N!", ch, NULL, victim, TO_CHAR );
  act( "$n launches a tremendous ball of dark flame at you!", ch, NULL, victim, TO_VICT );
  act( "$n launches a tremendous ball of dark flame at $N!", ch, NULL, victim, TO_NOTVICT );
  dam = dice( rank, rank * 3 );
  saved = saves_chant( ch, victim, cn );
  if ( saved )
    dam -= dam/4;

  /* PvP balance (#27): Balus Wall (Patryn fire+protection runeweave OR the
     Sorcerer chant -- same gsn_balus_wall affect) partly turns Gaav Flare.
     Gaav Flare is Black-school so it bypasses the fire-school full-absorb in
     fight.c; here the wall instead smothers HALF the dark flame, is consumed,
     and blocks the Scorched debuff -- it does NOT fully absorb. */
  if ( victim != NULL && is_affected( victim, gsn_balus_wall ) )
  { dam -= dam / 2;
    affect_strip( victim, gsn_balus_wall );
    balus_blocked = TRUE;
    act( "$N's Balus Wall flares and smothers half the dark flame, then shatters!", ch, NULL, victim, TO_CHAR );
    act( "Your Balus Wall flares and smothers half the dark flame, then shatters!", ch, NULL, victim, TO_VICT );
    act( "$N's Balus Wall flares and smothers half the dark flame, then shatters!", ch, NULL, victim, TO_NOTVICT );
  }

  chant_damage( ch, victim, dam, cn );

  /* PvP balance: on a FAILED save the flame leaves the target "Scorched",
     a short Black-school vulnerability (handled in chant_damage).  A Balus
     Wall block also prevents Scorched (#27). */
  if ( !saved && !balus_blocked && victim != NULL && victim->position > POS_DEAD )
  { int sn = skill_lookup( "scorched" );
    if ( sn > 0 && !is_affected( victim, sn ) )
    { AFFECT_DATA af;
      af.type      = sn;
      af.duration  = dice( 2, 4 );      /* ~2-8 ticks */
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = 0;
      affect_to_char( victim, &af );
      act( "$N's skin is left smoldering and scorched!", ch, NULL, victim, TO_CHAR );
      act( "Your skin is left smoldering and scorched!", ch, NULL, victim, TO_VICT );
    }
  }
  return;
}

void chant_dynast_breath( int cn, int rank, CHAR_DATA *ch, void *vo )
{ AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool pvp;
  bool saved;
  int  ac_pen;

  if ( is_affected( victim, skill_lookup( "dynast breath" ) ) )
  { send_to_char( "They are already encased in ice!\n\r", ch );
    return;
  }

  /* Reworked (issue #2 / docs/features/2-2026-06-27-dynast-breath-rework.md):
   * one clean effect -- AC exposure, plus no-flee only on a failed save -- with
   * a save and a short duration. Drops the old hitroll/damroll affects and the
   * ~105-tick lockdown that let Sorcerer win by default. AC value is set by
   * target type at cast (PvP = a player victim), capped in PvP. */
  pvp   = !IS_NPC( victim );
  saved = saves_chant( ch, victim, cn );

  act( "$N is encased in a pillar of ice!", ch, NULL, victim, TO_CHAR );
  act( "Ice erupts beneath your feet and encases you!", ch, NULL, victim, TO_VICT );
  act( "$N is encased in a pillar of ice!", ch, NULL, victim, TO_NOTVICT );

  if ( saved )
    ac_pen = pvp ? UMIN( rank * 2, 100 ) : rank * 3;
  else
    ac_pen = pvp ? UMIN( rank * 6, 300 ) : rank * 6;  /* #95: PvP cap 200->300 */

  af.type      = skill_lookup( "dynast breath" );
  /* #34: restore the long not-saved root (the #2 nerf to 3/5 was too harsh) --
   * but it now lands only on a FAILED save (and is shrugged by Holy Bless, below),
   * which is the counterplay the original ~105-tick lockdown lacked.  A save still
   * shortens it sharply. */
  if ( saved )
    af.duration = pvp ? dice( 2, 10 ) : dice( 3, 10 );
  else
    af.duration = dice( 10, 20 );
  af.location  = APPLY_AC;
  af.modifier  = ac_pen;
  af.bitvector = saved ? 0 : AFF_NO_FLEE;
  /* #23: Holy Bless (white-spec) ward -- 5% chance to keep the feet free of the
   * rooting ice.  Only the no-flee is shrugged off; the AC penalty still lands. */
  if ( ( af.bitvector & AFF_NO_FLEE ) != 0
    && holy_bless_wards( ch, victim,
         "Ceiphied's blessing keeps your feet free of the ice!\n\r",
         "Ceiphied's blessing frees $N's feet -- $E is not rooted!" ) )
    af.bitvector = 0;
  affect_to_char( victim, &af );

  /* #95: a failed save in PvP also carries a flat -15 hitroll / -15 damroll for
   * the same duration.  Re-adds the offensive pressure the #2 rework dropped, but
   * gated behind a FAILED save (which a save avoids entirely) rather than the old
   * unconditional lockdown. */
  if ( !saved && pvp )
  { AFFECT_DATA af2;
    af2.type      = skill_lookup( "dynast breath" );
    af2.duration  = af.duration;
    af2.location  = APPLY_HITROLL;
    af2.modifier  = -15;
    af2.bitvector = 0;
    affect_to_char( victim, &af2 );
    af2.location  = APPLY_DAMROLL;
    affect_to_char( victim, &af2 );
  }

  if ( saved )
  { act( "$N twists aside, resisting the worst of the ice!", ch, NULL, victim, TO_CHAR );
    send_to_char( "You twist aside as the ice closes in, resisting the worst of it!\n\r", victim );
  }
  return;
}

void chant_laguna_blast( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int dam;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  act( "A purple-black pentagram below $N spews forth dark energies!", ch, NULL, victim, TO_CHAR );
  act( "A purple-black pentagram below you spews forth dark energies!", ch, NULL, victim, TO_VICT );
  act( "A purple-black pentagram below $N spews forth dark energies!", ch, NULL, victim, TO_NOTVICT );
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( (IS_NPC(victim) && IS_NPC(vch)) || is_same_group( victim, vch ) )
    { dam = dice( rank, rank*2 );
      if ( saves_chant( ch, victim, cn ) )
        dam /= 2;
      chant_damage( ch, vch, dam, cn );
    }
  }
  return;
}

void chant_flame_breath( int cn, int rank, CHAR_DATA *ch, void *vo )
{ 
  int dam;
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  act( "Flames explode all around $N!", ch, NULL, victim, TO_CHAR );
  act( "Flames explode all around you!", ch, NULL, victim, TO_VICT );
  act( "Flames explode all around $N.", ch, NULL, victim, TO_NOTVICT );
  dam = dice( rank, rank * 4 );

  if ( saves_chant( ch, victim, cn ) )
    dam /= 2;
  else
  {
    act( "$N ignites and burns brightly!", ch, NULL, victim, TO_CHAR );
    act( "You feel your skin melt as you combust!", ch, NULL, victim, TO_VICT );
    act( "$N ignites and burns brightly.", ch, NULL, victim, TO_NOTVICT );
    af.type      = skill_lookup( "flame breath" );
    af.duration  = dice(7,5);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
  }

  chant_damage( ch, victim, dam, cn );
  return;
}

void chant_dragon_slave( int cn, int rank, CHAR_DATA *ch, void *vo )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  act( "Burning red energy explodes into a gumdrop of death!", ch, NULL, NULL, TO_CHAR );
  act( "Burning red energy explodes into a gumdrop of death!", ch, NULL, NULL, TO_ROOM );
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( !is_same_group( ch, vch ) )
    { dam = dice( rank, rank*4 );
      if ( saves_chant( ch, vch, cn ) )
        dam -= dam / 3;
      chant_damage( ch, vch, dam, cn );
    }
  }
  SET_BIT(ch->pcdata->actnew,NEW_DRAGON_SLAVE);
  return;
}

/* Black-magic finisher: conjures a short-lived blade of huge damage that
   drains Mystic every pulse while wielded (see char_update) and blocks other
   complex chants (see do_chant).  Counterplay: disarm, flee, Flow Break. */
void chant_laguna_blade( int cn, int rank, CHAR_DATA *ch, void *vo )
{ OBJ_DATA *obj;
  OBJ_DATA *wield;

  if ( IS_SET( ch->pcdata->actnew, NEW_LAGUNABLADE ) )
  { send_to_char( "The blade already burns in your hands.\n\r", ch );
    return;
  }

  /* put away whatever is wielded first */
  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
  { act( "You lower $p.", ch, wield, NULL, TO_CHAR );
    unequip_char( ch, wield );
  }

  act( "An awesome blade of oscillating black energy erupts in your hands!", ch, NULL, NULL, TO_CHAR );
  act( "An awesome blade of oscillating black energy erupts in $n's hands!", ch, NULL, NULL, TO_ROOM );

  SET_BIT( ch->pcdata->actnew, NEW_LAGUNABLADE );
  obj = create_object( get_obj_index( LAGUNA_BLADE ), 2 );
  obj->timer    = 2;                          /* short fuse even if you survive */
  obj->value[1] = UMAX( 20, rank );           /* damage dice scale with rank */
  obj->value[2] = 40 * UMAX( 20, rank );
  obj_to_char( obj, ch );
  equip_char( ch, obj, WEAR_WIELD );
  return;
}

/* white magic starts here */
void chant_lighting( int cn, int rank, CHAR_DATA *ch, void *vo )
{ AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  
  act( "A bright ball of light appears right in $N's face.", ch, NULL, victim, TO_CHAR );
  act( "A bright ball of light appears right in your face!", ch, NULL, victim, TO_VICT );
  act( "A bright ball of light appears right in $N's face.", ch, NULL, victim, TO_NOTVICT );
  if ( IS_SUIT(victim) || victim->level > 95 )
    return;

  /* #6: no longer all-or-nothing.  On a save, a brief "dazzled" hitroll
   * penalty; on a failed save, blind (PvP capped at 2 ticks, hitroll -8).
   * Uses the blindness type (a real skill index, not cn) and doesn't stack. */
  if ( saves_chant( ch, victim, cn ) )
  { int sn = skill_lookup( "blindness" );
    if ( !is_affected( victim, sn ) )
    { af.type      = sn;
      af.location  = APPLY_HITROLL;
      af.modifier  = -3;
      af.duration  = 1;
      af.bitvector = 0;
      affect_to_char( victim, &af );
    }
    act( "$N is dazzled by the flash!", ch, NULL, victim, TO_CHAR );
    send_to_char( "You are dazzled by the searing light!\n\r", victim );
    return;
  }

  /* #23: Holy Bless (white-spec) ward -- 5% chance to shrug off the blinding flash */
  if ( holy_bless_wards( ch, victim,
         "Ceiphied's blessing wards off the searing light!\n\r",
         "Ceiphied's blessing shields $N from the blinding flash!" ) )
    return;

  /* refresh existing blindness on the VICTIM (the old code stripped the caster) */
  if ( is_affected( victim, skill_lookup( "blindness" ) ) )
    affect_strip( victim, skill_lookup( "blindness" ) );
  af.type      = skill_lookup( "blindness" );
  af.location  = APPLY_HITROLL;
  af.modifier  = -8;
  af.duration  = ( !IS_NPC(victim) ? 2 : 1+dice(5,20) );
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );
  act( "$N is blinded!", ch, NULL, victim, TO_CHAR );
  send_to_char( "You are blinded!\n\r", victim );
  return;
}

void chant_dicleary( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int heal;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  heal = 100 + dice( 10, rank );
  if ( victim == ch )
    send_to_char( "You are refreshed and healed.\n\r", ch );
  else
  { act( "$N is refreshed and healed.", ch, NULL, victim, TO_CHAR );
    send_to_char( "You are refreshed and healed.\n\r", victim );
  }
  victim->hit = UMIN( victim->max_hit, victim->hit + heal );
  update_pos( victim );
  victim->move = UMIN( victim->max_move, victim->move + heal );

  /* #6: also cleanse ONE major debuff, gated by rank, highest priority first
   * (curse > root > blind > poison). Healing/move above is unchanged. */
  { int cured = 0;
    if ( rank >= 35 )
    { int cg = 0;
      if      ( is_affected( victim, gsn_wind_curse  ) ) cg = gsn_wind_curse;
      else if ( is_affected( victim, gsn_earth_curse ) ) cg = gsn_earth_curse;
      else if ( is_affected( victim, gsn_flame_curse ) ) cg = gsn_flame_curse;
      else if ( is_affected( victim, gsn_water_curse ) ) cg = gsn_water_curse;
      if ( cg != 0 )
      { affect_strip( victim, cg ); cured = 1;
        send_to_char( "A dark curse is lifted from you.\n\r", victim ); }
    }
    if ( !cured && rank >= 45 && IS_AFFECTED( victim, AFF_NO_FLEE ) )
    { AFFECT_DATA *paf;
      int rt = -1;
      for ( paf = victim->affected; paf != NULL; paf = paf->next )
        if ( paf->bitvector & AFF_NO_FLEE )
        { rt = paf->type; break; }
      /* Only cure (and message) if a backing affect actually exists; strip by
       * type so paired affects (e.g. the laphas-seed move drain) go too. */
      if ( rt != -1 )
      { affect_strip( victim, rt ); cured = 1;
        send_to_char( "You are freed from magical restraint.\n\r", victim ); }
    }
    if ( !cured && rank >= 25 && is_affected( victim, skill_lookup( "blindness" ) ) )
    { affect_strip( victim, skill_lookup( "blindness" ) ); cured = 1;
      send_to_char( "Your sight returns.\n\r", victim ); }
    if ( !cured && rank >= 15 && is_affected( victim, skill_lookup( "poison" ) ) )
    { affect_strip( victim, skill_lookup( "poison" ) ); cured = 1;
      send_to_char( "The poison is purged from you.\n\r", victim ); }
  }
  return;
}

void chant_holy_bless( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int sn =  skill_lookup( "bless" );

  if ( is_affected( victim, sn ) )
    affect_strip( victim, sn );

  af.type      = sn;
  af.duration  = 30 * rank + dice(1,6);
  af.location  = APPLY_HITROLL;
  /* #6: better EARLY scaling (rank/5 -> 10->2, 25->5, 50->10) without nerfing
   * high rank -- take the max of the new curve and the old (rank-30)/2 curve. */
  af.modifier  = UMAX( 2, UMAX( rank / 5, (rank - 30) / 2 ) );
  /* #23: a White-specialist caster's bless also confers Ceiphied's ward -- a
   * 5% chance to shrug off curse / blind / sleep / magical root.  Marked on the
   * affect so it shows in affected_by and is cleared when the bless ends. */
  af.bitvector = ( !IS_NPC(ch) && ch->pcdata->powers[SORC_SPEC] == SCHOOL_WHITE )
                 ? AFF_HOLY_BLESS : 0;
  affect_to_char( victim, &af );

  /* Carry the ward bit on exactly ONE node (the hitroll affect above).  If both
   * bless nodes carried it, affect_remove's per-node REMOVE_BIT would clear the
   * aggregate flag the moment either node is stripped individually (flow_break,
   * dispel), killing the ward while the other node still lives. */
  af.bitvector = 0;
  af.location  = APPLY_DAMROLL;
  affect_to_char( victim, &af );
  send_to_char( "You feel Ceiphied's protection enfold you.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

/* #23: White-specialist Holy Bless ward.  Returns TRUE (and emits the two
 * messages) when the victim shrugs off an incoming effect, so callers can skip
 * applying it.  Centralizes the 5% predicate + messaging shared by every
 * curse/blind/sleep/root application site. */
bool holy_bless_wards( CHAR_DATA *ch, CHAR_DATA *victim, char *vict_msg, char *caster_msg )
{
  if ( IS_AFFECTED( victim, AFF_HOLY_BLESS ) != 0 && number_percent() <= HOLY_BLESS_WARD_PCT )
  { send_to_char( vict_msg, victim );
    act( caster_msg, ch, NULL, victim, TO_CHAR );
    return TRUE;
  }
  return FALSE;
}

/* White/Astral counter to magic stacking.  Strips 1-3 temporary affects
   (never everything), collapses a holy-resist barrier, poofs an active Laguna
   Blade or Recovery, and -- on a failed save -- disrupts a gathering chant. */
void chant_flow_break( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA *paf, *paf_next;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *wield;
  int hr      = skill_lookup( "holy resist" );
  int strips  = URANGE( 1, 1 + rank / 18, 3 );   /* 1-3 by rank; never all */
  int removed = 0;

  act( "You sever the flow of power around $N!", ch, NULL, victim, TO_CHAR );
  act( "$n severs the flow of power around you!", ch, NULL, victim, TO_VICT );
  act( "$n severs the flow of power around $N.", ch, NULL, victim, TO_NOTVICT );

  /* Extra effect vs a holy-resist barrier: collapse or weaken it first. */
  if ( is_affected( victim, hr ) )
  {
    for ( paf = victim->affected; paf != NULL && paf->type != hr; paf = paf->next )
      ;
    if ( paf != NULL )
    {
      paf->duration = UMAX( 0, paf->duration - 200 - rank * 4 );
      if ( paf->duration == 0 )
      { affect_strip( victim, hr );
        send_to_char( "Your protective barrier collapses!\n\r", victim );
      }
      else
        send_to_char( "Your barrier shudders but holds.\n\r", victim );
    }
  }

  /* PvP balance: explicitly tear down a Vas Gluudo shield (the generic strip
     below only reaches the first few affects, so guarantee this one falls). */
  { int vg = skill_lookup( "vas gluudo" );
    if ( vg > 0 && is_affected( victim, vg ) )
    { affect_strip( victim, vg );
      send_to_char( "Your shield of energy unravels!\n\r", victim );
    }
  }

  /* Shatter an active Laguna Blade. */
  if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew, NEW_LAGUNABLADE) )
  {
    REMOVE_BIT( victim->pcdata->actnew, NEW_LAGUNABLADE );
    if ( ( wield = get_eq_char( victim, WEAR_WIELD ) ) != NULL
      && wield->pIndexData->vnum == LAGUNA_BLADE )
      extract_obj( wield );
    send_to_char( "Your blade of black energy shatters!\n\r", victim );
  }

  /* Disrupt an active Recovery. */
  if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew, NEW_RECOVERY) )
  {
    REMOVE_BIT( victim->pcdata->actnew, NEW_RECOVERY );
    send_to_char( "Your recovery is disrupted.\n\r", victim );
  }

  /* #8: explicit, save-scaled interaction with Patryn defenses.  Blocks are
   * actnew bits (the generic strip below can't reach them); wards are gsn_*_ward
   * affects, handled here (drained or removed) -- so the generic loop skips them
   * to avoid double-removing a ward in one cast. */
  if ( IS_CLASS( victim, CLASS_PATRYN ) )
  {
    bool  saved     = saves_chant( ch, victim, cn );
    int   blockbit  = 0;
    int   wardgsn   = 0;
    bool  popblock  = FALSE;
    char *blockname = NULL;
    char  buf[MAX_STRING_LENGTH];
    AFFECT_DATA *wpaf;

    /* Priority cascades live in patryn.c (patryn_active_block/ward) -- #25. */
    blockbit = patryn_active_block( victim, &blockname );
    wardgsn  = patryn_active_ward( victim );

    /* A failed save is strictly worse than a save: it removes a ward outright
     * (and a block); a save only drains the ward -- never fully, floor 1 -- and
     * has a 25% chance to also pop a block.  Block removal is hoisted below so
     * both paths share one stanza. */
    if ( !saved )
    { popblock = ( blockbit != 0 );
      if ( wardgsn != 0 )
      { affect_strip( victim, wardgsn );
        send_to_char( "Your ward unravels!\n\r", victim );
        act( "Your Flow Break unravels $N's ward!", ch, NULL, victim, TO_CHAR );
      }
    }
    else
    { popblock = ( blockbit != 0 && number_percent() <= 25 );
      if ( wardgsn != 0 )
      { for ( wpaf = victim->affected; wpaf != NULL; wpaf = wpaf->next )
          if ( wpaf->type == wardgsn )
          { wpaf->duration = UMAX( 1, wpaf->duration - 300 ); break; }
        act( "Your Flow Break scrapes power from $N's ward.", ch, NULL, victim, TO_CHAR );
        send_to_char( "Your ward loses power under the Flow Break.\n\r", victim );
      }
    }

    if ( popblock )
    { REMOVE_BIT( victim->pcdata->actnew, blockbit );
      sprintf( buf, "Your %s block is torn apart!\n\r", blockname );
      send_to_char( buf, victim );
      act( "Your Flow Break tears one of $N's rune blocks apart!", ch, NULL, victim, TO_CHAR );
    }
  }

  /* Strip up to `strips` temporary affects (permanent/innate ones are left).
   * Patryn wards are handled explicitly above, so skip them here (#8). */
  for ( paf = victim->affected; paf != NULL && removed < strips; paf = paf_next )
  {
    paf_next = paf->next;
    if ( paf->type == hr || paf->duration < 0 )
      continue;
    /* Patryn wards are handled explicitly above -- skip them here only for
     * Patryns, so a ward lingering on a non-Patryn is still strippable. */
    if ( IS_CLASS(victim,CLASS_PATRYN)
      && ( paf->type == gsn_spirit_ward || paf->type == gsn_earth_ward
        || paf->type == gsn_flame_ward  || paf->type == gsn_wind_ward
        || paf->type == gsn_water_ward  || paf->type == gsn_negative_ward ) )
      continue;
    if ( paf->type > 0 && skill_table[paf->type].msg_off )
    { send_to_char( skill_table[paf->type].msg_off, victim );
      send_to_char( "\n\r", victim );
    }
    affect_remove( victim, paf );
    removed++;
  }

  /* On a failed save, also disrupt one gathering chant/prep. */
  if ( !IS_NPC(victim) && victim->class == CLASS_SORCERER
    && victim->pcdata->chant != NULL && !saves_chant( ch, victim, cn ) )
  {
    lose_chant( victim );
    act( "You disrupt $N's gathering chant!", ch, NULL, victim, TO_CHAR );
    send_to_char( "Your gathering chant is disrupted!\n\r", victim );
  }

  if ( removed == 0 )
    act( "...but there was little magic left to unravel.", ch, NULL, victim, TO_CHAR );
  return;
}


/* PvP tunables for Laphas Seed (the bind/restraint chant). */
#define LAPHAS_PVP_DUR_CAP  20   /* max ticks of AFF_NO_FLEE vs a player    */
#define LAPHAS_MOVE_DRAIN   50   /* movement points sapped while bound      */

void chant_laphas_seed( int cn, int rank, CHAR_DATA *ch, void *vo )
{ AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if ( IS_AFFECTED(victim,AFF_NO_FLEE) )
  { send_to_char( "They're already affected by a restraining spell.\n\r", ch );
    return;
  }
  if ( saves_chant( ch, victim, cn ) )
  { act( "You launch bands of energy at $N, but $E dodges!", ch, NULL, victim, TO_CHAR );
    act( "Bands of energy streak toward you, but you dodge!", ch, NULL, victim, TO_VICT );
    act( "$N dodges $n's energy bands.", ch, NULL, victim, TO_NOTVICT );
    return;
  }

  /* #23: Holy Bless (white-spec) ward -- 5% chance to snap the binding energy */
  if ( holy_bless_wards( ch, victim,
         "Ceiphied's blessing snaps the binding energy!\n\r",
         "Ceiphied's blessing keeps $N free of your bands!" ) )
    return;

  act( "$N is wrapped in bands of energy!", ch, NULL, victim, TO_CHAR );
  act( "You are wrapped in bands of energy!", ch, NULL, victim, TO_VICT );
  act( "$N is wrapped in bands of energy.", ch, NULL, victim, TO_NOTVICT );
  af.type      = skill_lookup( "laphas seed" );
  af.duration  = dice(5,20);
  /* PvP balance: cap the restraint duration on players so it can't lock a PC
     down for an absurdly long time (PvE mobs keep the full roll).          */
  if ( !IS_NPC(victim) )
    af.duration = UMIN( af.duration, LAPHAS_PVP_DUR_CAP );
  af.location  = APPLY_AC;
  af.modifier  = rank * 4;
  af.bitvector = AFF_NO_FLEE;
  affect_to_char( victim, &af );

  /* PvP balance: also sap movement so the bound target can't immediately
     burn moves on escape skills.  Second affect carries the -move modifier. */
  af.location  = APPLY_MOVE;
  af.modifier  = -1 * LAPHAS_MOVE_DRAIN;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  return;
}

void chant_recovery( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;
  if ( IS_NPC(victim) )
    return;
  if ( IS_SET(victim->pcdata->actnew,NEW_RECOVERY) )
  { send_to_char( "They already have a recovery spell active.\n\r", ch );
    return;
  }
  SET_BIT(victim->pcdata->actnew,NEW_RECOVERY);   /* starts the heal-over-time (char_update) */

  /* immediate partial heal so the cast isn't a no-op */
  heal = 200 + dice( rank, 30 );
  victim->hit = UMIN( victim->max_hit, victim->hit + heal );
  update_pos( victim );

  /* higher rank also cleanses poison / blindness / light stun */
  if ( rank >= 30 )
  { if ( is_affected( victim, skill_lookup( "poison" ) ) )
      affect_strip( victim, skill_lookup( "poison" ) );
    if ( is_affected( victim, skill_lookup( "blindness" ) ) )
      affect_strip( victim, skill_lookup( "blindness" ) );
    if ( victim->position == POS_STUNNED )
      victim->position = POS_RESTING;
  }

  act( "You place a hand over a wound, and it begins to knit closed.", ch, NULL, NULL, TO_CHAR );
  act( "$n places $s hand against $N's body, and it begins to glow.",ch,NULL,victim,TO_ROOM );
  send_to_char( "A soothing warmth spreads through you; your wounds begin to mend.\n\r", victim );
  return;
}

void chant_sleeping( int cn, int rank, CHAR_DATA *ch, void *vo )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA  af;
  af.type      = skill_lookup( "sleep" );
  af.duration  = dice( 3, rank );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SLEEP;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( !is_same_group( ch, vch ) && ch != vch )
    { if ( ( IS_NPC(vch) && (saves_chant(ch, vch, cn) || vch->level > 75) ) ||
           (!IS_NPC(vch) && (saves_chant(ch,vch,cn) || vch->max_hit>10000 || vch->level<2)) )
      { act( "$N looks drowsy for a moment, but shakes it off.", ch, NULL, vch, TO_CHAR );
        act( "You feel drowsy for a moment, but shake it off.", ch, NULL, vch, TO_VICT );
        act( "$N looks drowsy for a moment, but shakes it off.", ch,NULL,vch,TO_NOTVICT );
      }
      /* #23: Holy Bless (white-spec) ward -- 5% chance to shrug off the sleep */
      else if ( holy_bless_wards( ch, vch,
                  "Ceiphied's blessing wards off the drowsiness!\n\r",
                  "Ceiphied's blessing keeps $N awake!" ) )
      { /* warded -- messages emitted by helper */ }
      else
      { act( "$N dozes off.", ch, NULL, vch, TO_CHAR );
        act( "You feel drowsy.. zzzz...", ch, NULL, vch, TO_VICT );
        act( "$N dozes off.", ch, NULL, vch, TO_NOTVICT );
        vch->position = POS_SLEEPING;
        if ( !IS_AFFECTED(vch,AFF_SLEEP) )
          affect_to_char( vch, &af );
      }
    }
  }
  return;

}

void chant_mos_varim( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;
  if ( is_affected( ch, gsn_mos_varim ) )
    affect_strip( ch, gsn_mos_varim );

  af.type      = gsn_mos_varim;
  af.duration  = 30 * rank + dice(1,6);
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = 0;
  affect_to_char( ch, &af );
  act( "A flourescent white ball pops into existance above you.",ch,NULL,NULL,TO_CHAR );
  act( "A flourescent white ball pops into existance above $n.",ch,NULL,NULL,TO_ROOM );
  return;
}

void chant_visfarank( int cn, int rank, CHAR_DATA *ch, void *vo )
{ AFFECT_DATA af;
  int sn = skill_lookup( "visfarank" );
  if ( is_affected( ch, sn ) )
    affect_strip( ch, sn );

  act( "You focus magical energy into your fists.", ch, NULL, NULL, TO_CHAR );
  act( "$n's fists begin to glow.", ch, NULL, NULL, TO_ROOM );
  af.type      = sn;
  af.duration  = 30 * rank + dice(1,6);
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_VISFARANK;
  affect_to_char( ch, &af );
  return;
}

void chant_vas_gluudo( int cn, int rank, CHAR_DATA *ch, void *vo )
{  AFFECT_DATA af;
  int sn = skill_lookup( "vas gluudo" );
  if ( is_affected( ch, sn ) )
    affect_strip( ch, sn );
  act( "A translucent shield of energy forms around you.", ch, NULL, NULL, TO_CHAR );
  act( "A translucent shield of energy forms around $n.", ch, NULL, NULL, TO_ROOM );
  af.type      = sn;
  af.duration  = 30 * rank + dice(1,6);
  af.location  = APPLY_AC;
  af.modifier  = -1 * rank;
  af.bitvector = AFF_VAS_GLUUDO;
  affect_to_char( ch, &af );
  return;
}

void chant_chaos_strings( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  int i=0, dam, max;
  AFFECT_DATA af;
  bool pvp, saved;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_NPC(victim) )
    max = 6;
  else
    max = 4;

  act( "You lash $N with enchanted strings!", ch, NULL, victim, TO_CHAR );
  act( "You are lashed with enchanted strings!", ch, NULL, victim, TO_VICT );
  act( "$N is lashed with enchanted strings.", ch, NULL, victim, TO_NOTVICT );

  /* #6: peel -- apply the melee-suppression debuff the fight code already reads
   * (halves Fist melee, cuts attacks ~1/3).  Applied before damage so it lands
   * even if the victim then dies.  Damage is intentionally unchanged.  Skip in
   * a safe room so it can't be used to debuff players where no fight can happen
   * (chant_damage's is_safe runs later, so guard the affect ourselves). */
  pvp   = !IS_NPC(victim);
  saved = saves_chant( ch, victim, cn );
  if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE )
    && !is_affected( victim, skill_lookup( "chaos strings" ) ) )
  { af.type      = skill_lookup( "chaos strings" );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CHAOS_STRING;
    af.duration  = saved ? ( pvp ? 1 : 2 ) : ( pvp ? 2 : 4 );
    affect_to_char( victim, &af );
  }

  do
  { dam = dice( 15, rank );
    dam += dam/2 * i;
    chant_damage( ch, victim, dam, cn );
    i++;
  } while ( victim != NULL && number_percent() > (30-rank/2) && i < max );

  return;
}

void chant_holy_resist( int cn, int rank, CHAR_DATA *ch, void *vo )
{ AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int sn = skill_lookup( "holy resist" );
  if ( is_affected( ch, sn ) )
    affect_strip( ch, sn );
  act( "Shimmering motes of light descend onto you.", ch, NULL, victim, TO_CHAR );
  act( "Shimmering motes of light descend onto $n.", ch, NULL, victim, TO_ROOM );
  af.type      = sn;
  af.duration  = 30 * rank + dice(1,6);
  af.location  = APPLY_AC;
  af.modifier  = -3 * (rank-15);   /* sturdier barrier: white = the sustain spec */
  af.bitvector = AFF_HOLY_RESIST;
  affect_to_char( ch, &af );
  return;
}

void chant_defense( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;
  if ( is_affected( ch, gsn_defense ) )
  {
    stc( "You already have a defensive barrier active.\n\r", ch );
    return;
  }

  af.type      = gsn_defense;
  /* PvP balance: old rank/40 gave ~0-2 ticks (useless at most ranks).
     Now a guaranteed short emergency window that scales: 2 + rank/20. */
  af.duration  = 2 + rank / 20;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = 0;
  affect_to_char( ch, &af );

  act( "You bring your hands up and form a defensive barrier.",ch,NULL,NULL,TO_CHAR );
  act( "$n brings $m hands up and forms a defensive barrier.", ch, NULL, NULL, TO_ROOM );
  return;
}

void chant_chaotic_disintegrate( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int i=0, dam, max, min;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  
  if ( IS_NPC(victim) )
  { if ( victim->level > 95 )
    {
      min = 3;
      max = 5;
    }
    else
    {
      min = 5;
      max = 10;
    }
  }
  else
  { min = 2;
    max = 4;
  }
  act( "$N is engulfed in swirling white flame!", ch, NULL, victim, TO_CHAR );
  act( "You are engulfed in swirling white flame!", ch, NULL, victim, TO_VICT );
  act( "$N is engulfed in swirling white flame!", ch, NULL, victim, TO_NOTVICT );
  do 
  { dam = dice( rank, rank );
    dam += dam/2 * i;
    chant_damage( ch, victim, dam, cn );
    i++;
  } while ( victim != NULL && (number_percent() > 33 || i < min) && i < max );
  return;
}

void chant_ressurection( int cn, int rank, CHAR_DATA *ch, void *vo )
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal = 1;

  if ( !IS_NPC(victim) && !is_same_group( ch, victim ) )
  { send_to_char( "You cannot ressurection players not in your group.\n\r", ch );
    return;
  }

  act( "You place your hands on $N and call upon Ceified's power..",ch,NULL,victim,TO_CHAR);
  act( "$n places $s hands on you and invokes Ceified's name.",ch,NULL,victim,TO_VICT);
  act( "$n places $s hands on $N and invokes Ceified's name.", ch, NULL, victim, TO_NOTVICT);
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( vch != victim )
      heal += URANGE( 1, vch->max_hit / 30, 1000 );
  }
  if ( victim->position < POS_STUNNED )
    heal += heal / 2;
  heal = UMIN( heal, 25000 );
  victim->hit = UMIN( victim->max_hit, victim->hit + heal);
  update_pos( victim );
  return;
}


void chant_levitation( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  int sn = skill_lookup( "fly" );
  (*skill_table[sn].spell_fun) ( sn, rank, ch, vo );
  return;
}


void chant_scatter_brid( int cn, int rank, CHAR_DATA *ch, void *vo )
{ int i=0, dam, max;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_NPC(victim) )
    max = 10;
  else
    max = 5;
  act( "You spray $N with tiny energy bolts!", ch, NULL, victim, TO_CHAR );
  act( "You are sprayed with tiny energy bolts!", ch, NULL, victim, TO_VICT );
  act( "Tiny energy bolts stream towards $N.", ch, NULL, victim, TO_NOTVICT );
  do
  { dam = dice( 10, 10 );
    dam += dam/2 * i;
    chant_damage( ch, victim, dam, cn );
    i++;
  } while ( victim != NULL && number_percent() > (30-rank/2) && i < max );
  return;
} 

void chant_digger_bolt( int cn, int rank, CHAR_DATA *ch, void *vo )
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( 15, 20 + rank );
  if ( saves_chant( ch, victim, cn ) )
    dam /= 2;
  act( "A bolt of lightning arcs from your fingers to $N.", ch, NULL, victim, TO_CHAR );
  act( "Lightning arcs from $n's fingers to you!", ch, NULL, victim, TO_VICT );
  act( "Lightning arcs from $n's fingers to $N.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
}


void chant_diem_wing( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA*) vo;

  act( "You hurl a gust of wind at $N!", ch, NULL, victim, TO_CHAR );
  act( "$n hurls a gust of wind at you!", ch, NULL, victim, TO_VICT );
  act( "$n hurls a gust of wind at $N.", ch, NULL, victim, TO_NOTVICT );

  if ( IS_NPC(victim) )
    return;
  if ( IS_CLASS(victim,CLASS_FIST) && victim->pcdata->powers[F_KI] > 1 )
      victim->pcdata->powers[F_KI]--;
  if ( IS_CLASS(victim,CLASS_SORCERER) && victim != ch  )
    lose_chant( victim );
  if ( IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
    REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
  /* PvP feedback: this chant already stripped these barriers silently, so
     players "saw nothing happen".  Announce each strip so it reads as working. */
  if ( is_affected(victim,gsn_kiwall) )
  { affect_strip(victim,gsn_kiwall);
    act( "Your gust shears away $N's wall of Ki!", ch, NULL, victim, TO_CHAR );
    act( "$n's gust shears away your wall of Ki!", ch, NULL, victim, TO_VICT );
    act( "$n's gust shears away $N's wall of Ki!", ch, NULL, victim, TO_NOTVICT );
  }
  if ( is_affected(victim,gsn_defense) )
  { affect_strip(victim,gsn_defense);
    act( "Your gust tears apart $N's defensive barrier!", ch, NULL, victim, TO_CHAR );
    act( "$n's gust tears apart your defensive barrier!", ch, NULL, victim, TO_VICT );
    act( "$n's gust tears apart $N's defensive barrier!", ch, NULL, victim, TO_NOTVICT );
  }
  if ( is_affected(victim,gsn_balus_wall) )
  { affect_strip(victim,gsn_balus_wall);
    act( "Your gust scatters $N's Balus Wall!", ch, NULL, victim, TO_CHAR );
    act( "$n's gust scatters your Balus Wall!", ch, NULL, victim, TO_VICT );
    act( "$n's gust scatters $N's Balus Wall!", ch, NULL, victim, TO_NOTVICT );
  }
  return;
}

void chant_damu_brass( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( rank, rank*5/4 );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "You launch a ball compressed air at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n launches a ball of compressed air at you.", ch, NULL, victim, TO_VICT );
  act( "$n launches a ball of compressed air at $n.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
} 

void chant_raywing( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ AFFECT_DATA af;
  int sn = skill_lookup( "fly" );
  if ( is_affected( ch, sn ) )
      affect_strip( ch, sn );
  af.type      = sn;
  af.duration  = 30 * rank+dice(1,6);
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char( ch, &af );
  af.bitvector = AFF_RAYWING;
  affect_to_char( ch, &af );
  act( "You lift off and soar like a bird!", ch, NULL, NULL, TO_CHAR );
  act( "$n takes off like a bird.", ch, NULL, NULL, TO_ROOM );
  return;
} 
void chant_mono_volt( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ int dam;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  act( "You place your hand on $N and electrocute $M!", ch, NULL, victim, TO_CHAR );
  act( "$n places $s hand on you.. voltage flows through you!", ch, NULL, victim, TO_VICT );
  act( "$n places $s hand on $N and electrocutes $M.", ch, NULL, victim, TO_NOTVICT );
  dam = dice( rank, rank * 7 / 4 );
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_dimilar_ai( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( rank, rank * 3 / 4 );
  act( "You loose a blast of wind at $N!", ch, NULL, victim, TO_CHAR );
  act( "$n looses a blast of wind at you!", ch, NULL, victim, TO_VICT );
  act( "$n looses a blast of wind at $N.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  if ( !IS_NPC(victim) && number_percent() > (victim->pcdata->mind - 30) )
    unstance( victim );
  return;
} 
void chant_windy_shield( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ AFFECT_DATA af;
  int sn = skill_lookup( "windy shield" );
  if ( is_affected( ch, sn ) )
      affect_strip( ch, sn );
  af.type      = sn;
  af.duration  = 30 * rank+dice(1,6);
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_WINDY_SHIELD;
  affect_to_char( ch, &af );
  act( "Wind swirls around you, into a shield.", ch, NULL, NULL, TO_CHAR );
  act( "Wind swirls around $n, into a shield.", ch, NULL, NULL, TO_ROOM );
  return;
} 
void chant_bomb_di_wind( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( !IS_NPC(vch) && !is_same_group( ch, vch ) )
    { if ( IS_CLASS(vch,CLASS_FIST) && vch->pcdata->powers[F_KI] > 0 )
        vch->pcdata->powers[F_KI] = 0;
      if ( IS_SET(vch->pcdata->actnew,NEW_FIGUREEIGHT) )
        REMOVE_BIT(vch->pcdata->actnew,NEW_FIGUREEIGHT);
      if ( is_affected(vch,gsn_kiwall) )
        affect_strip(vch,gsn_kiwall);
      if ( is_affected(vch,gsn_defense) )
        affect_strip(vch,gsn_defense);
      if ( is_affected(vch,gsn_balus_wall) )
        affect_strip(vch,gsn_balus_wall);
      if ( IS_SET(vch->pcdata->actnew,NEW_FIRE_BLOCK) )
	REMOVE_BIT(vch->pcdata->actnew,NEW_FIRE_BLOCK);
      if ( IS_SET(vch->pcdata->actnew,NEW_AIR_BLOCK) )
	REMOVE_BIT(vch->pcdata->actnew,NEW_AIR_BLOCK);
      unstance( vch );
    }
  }
} 
 
void chant_gray_bomb( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ int dam;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  act( "Dust explodes around $N!", ch, NULL, victim, TO_CHAR );
  act( "Dust explodes around you!", ch, NULL, victim, TO_VICT );
  act( "Dust explodes around $N!", ch, NULL, victim, TO_NOTVICT );
  dam = dice( 10, 20+rank );
  if ( saves_chant( ch, victim, cn ) )
    dam /= 2;
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_earth_5( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{
} 
void chant_dill_brand( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_NPC(victim) )
  { send_to_char( "No effect on NPCs.\n\r", ch );
    return;
  }

  if ( !saves_chant( ch, victim, cn ) )
  { act( "$N is lifted into the air by an explosion!", ch, NULL, victim, TO_CHAR );
    act( "You are lifted into the air by an explosion!", ch, NULL, victim, TO_VICT );
    act( "$N is lifted into the air by an explosion!", ch, NULL, victim, TO_NOTVICT );
    unstance( victim );
  }
  else
  { act( "$N nimbly dodges the explosion of earth.", ch, NULL, victim, TO_CHAR );
    act( "You nimbly dodge the explosion of earth.", ch, NULL, victim, TO_VICT );
    act( "$N nimbly dodges the explosion of earth.", ch, NULL, victim, TO_NOTVICT );
  }
  return;
}

void chant_dug_wave( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( rank, rank*5/4 );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "$N is slammed back by a wave of earth!", ch, NULL, victim, TO_CHAR );
  act( "You are slammed back by a wave of earth!", ch, NULL, victim, TO_VICT );
  act( "$N is slammed back by a wave of earth!", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return; 
} 

void chant_blade_haut( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *wield;
  int dam;

  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL 
    || (wield->value[3] != 1 && wield->value[3] != 3) )
  { send_to_char( "You need an edged weapon to use the Blade Haut.\n\r", ch );
    return;
  }

  dam = dice( rank, rank*3/2);
  act( "You slam $p into the ground!", ch, wield, victim, TO_CHAR );
  act( "$n slams $p into the ground!", ch, wield, victim, TO_ROOM );
  if ( IS_CLASS(victim,CLASS_SAIYAN) && is_affected(victim,gsn_kiwall) )
  { act( "$n's wall of flame collapses!", ch, NULL, victim, TO_CHAR );
    act( "Your Ki Wall collapses from the shock!", ch, NULL, victim, TO_VICT );
    act( "$n's wall of flame collapses!", ch, NULL, victim, TO_NOTVICT );
    affect_strip(victim,gsn_kiwall);
    dam += dice( 4, dam );
  }
  else if ( IS_CLASS(victim,CLASS_FIST) && IS_SET(victim->pcdata->actnew,NEW_FIGUREEIGHT) )
  {
    act( "$n's arms twist inward!", ch, NULL, victim, TO_CHAR );
    act( "Your arms are snapped inward painfully!", ch, NULL, victim, TO_VICT );
    act( "$n's arms twist inward.", ch, NULL, victim, TO_NOTVICT );
    REMOVE_BIT(victim->pcdata->actnew,NEW_FIGUREEIGHT);
    dam *= 5;
  }
  chant_damage( ch, victim, dam, cn );
  return;  
} 
void chant_bephis_bring( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ 
} 
void chant_vigarthagaia( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{
} 
void chant_mega_brand( int cn, int rank, CHAR_DATA *ch, void *vo )
{ CHAR_DATA *vch;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch_next;
  int dam;

  act( "The earth explodes all around $N!", ch, NULL, victim, TO_CHAR );
  act( "The earth explodes all around you!", ch, NULL, victim, TO_VICT );
  act( "The earth explodes all around $N!", ch, NULL, victim, TO_NOTVICT );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( (IS_NPC(victim) && IS_NPC(vch)) || is_same_group( victim, vch ) )
    { dam = dice( rank, rank * 1/2 );
      chant_damage( ch, vch, dam, cn );
      if ( number_percent() < rank && vch != NULL && !IS_NPC(vch) )
        unstance( vch );
    }
  }
  return; 
}

void chant_dug_haut( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  act( "Huge stone spikes erupt everywhere!", ch, NULL, NULL, TO_CHAR );
  act( "Huge stone spikes erupt everywhere!", ch, NULL, NULL, TO_ROOM );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( vch != ch )
    { dam = dice( rank, rank );
      if ( saves_chant( ch, vch, cn ) )
        dam /= 2;
      chant_damage( ch, vch, dam, cn );
      if ( vch != NULL && !IS_NPC(vch) 
        && vch->position < POS_STUNNED && !IS_TIED(vch) )
      { act( "$N is knocked unconscious!", ch, NULL, vch, TO_CHAR );
        act( "You are knocked unconsciou!", ch, NULL, vch, TO_VICT );
        act( "$N is knocked unconscious!", ch, NULL, vch, TO_NOTVICT );
        vch->pcdata->extras[TIE] = dice(1,3);
      }
    }
  }
  return;
} 

void chant_vlave_howl( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ int dam, i;
  CHAR_DATA *victim = (CHAR_DATA*) vo;

  act( "You slam your fist into the ground, and summon forth geysers of magma!",
    ch, NULL, victim, TO_CHAR );
  act( "$n slams $s fist into the ground, and geysers of magma erupt from the earth!",
    ch, NULL, victim, TO_ROOM );

  for ( i = 0; i < 5 && victim != NULL && victim->position >= POS_STUNNED; i++ )
  { dam = dice( rank, rank );
    if ( saves_chant( ch, victim, cn ) )
      dam /= 3;
    chant_damage( ch, victim, dam, cn );
  }
  return;
} 


void chant_flare_bit( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( 10, 20 + rank );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "You toss several small balls of fire at $N", ch, NULL, victim, TO_CHAR );
  act( "$n tosses several small balls of fire at you.", ch, NULL, victim, TO_VICT );
  act( "$n tosses several small balls of fire at $n.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_flare_arrow( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( 10, 30 + rank );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "You shoot an arrow of flame at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n shoots an arrow of flame at you!" , ch, NULL, victim, TO_VICT );
  act( "$n shoots an arrow of flame at $n.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
}  
void chant_val_flare( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( 10, 40 + rank*2 );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "You launch a bolt of fire at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n launches a bolt of fire at you.", ch, NULL, victim, TO_VICT );
  act( "$n launches a bolt of fire at $n.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_burst_rondo( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ int i=0, dam, max;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_NPC(victim) )
    max = 7;
  else
    max = 4;
  act( "$N is pummeled by tiny fireballs!", ch, NULL, victim, TO_CHAR );
  act( "You are pummeled by tiny fireballs!", ch, NULL, victim, TO_VICT );
  act( "$N is pummeled by tiny fireballs!", ch, NULL, victim, TO_NOTVICT );
  do
  { dam = dice( 8, 20+rank );
    dam += dam/5 * i;
    chant_damage( ch, victim, dam, cn );
    i++;
  } while ( victim != NULL && number_percent() > (30-rank/2) && i < max );
  return;
} 
void chant_fireball( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *vch; 
  CHAR_DATA *vch_next;
  int dam;

  act( "You charge up a fireball and drop it!", ch, NULL, NULL, TO_CHAR );
  act( "$n charges up a fireball and drops it!", ch, NULL, NULL, TO_ROOM );
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( !is_same_group( ch, vch ) )
    { dam = dice( rank, rank );
      if ( saves_chant( ch, vch, cn ) )
        dam -= dam / 3;
      chant_damage( ch, vch, dam, cn );
    }
  }

  return;
} 

void chant_flare_lance( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( rank, rank*3/2 );
  if ( saves_chant( ch, victim, cn ) )
  dam = dam / 2;
  act( "You launch a lance of flame at $N!", ch, NULL, victim, TO_CHAR );
  act( "$n launches a lance of flame at you!", ch, NULL, victim, TO_VICT );
  act( "$n launches a lance of flame at $n.", ch, NULL, victim, TO_NOTVICT );
  if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_VAN_REHL) )
  { REMOVE_BIT(victim->pcdata->actnew,NEW_VAN_REHL);
    act( ".. and their icy barrier melts!", ch, NULL, victim, TO_CHAR );
    act( ".. and your icy barrier melts!", ch, NULL, victim, TO_VICT );
    act( ".. and their icy barrier melts!", ch, NULL, victim, TO_NOTVICT );
  }
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_rune_flare( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ int i=0, dam, max;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_NPC(victim) )
    max = 8;
  else
    max = 4;
  act( "Ribbons of fire streak towards $N!", ch, NULL, victim, TO_CHAR );
  act( "Ribbons of fire streak towards you!", ch, NULL, victim, TO_VICT );
  act( "Ribbons of fire streak towards $N.", ch, NULL, victim, TO_NOTVICT );
  do
  { dam = dice( rank, rank/3 );
    dam += dam/5 * i;
    chant_damage( ch, victim, dam, cn );
    i++;
  } while ( victim != NULL && number_percent() > (30-rank/2) && i < max );
  return;
}

void chant_balus_wall( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{
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
  return;
} 
void chant_burst_flare( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int dam = dice( rank, rank*4 );
  if ( saves_chant( ch, victim, cn ) )
  dam = dam / 2;
  act( "Blue-white flame bursts from your hands at $N!", ch, NULL, victim, TO_CHAR );
  act( "Blue-white flame bursts from $n's hands at you!", ch, NULL, victim, TO_VICT );
  act( "Blue-white flame bursts from $n's hands at $N!", ch, NULL, victim, TO_NOTVICT );
  if ( !IS_NPC(victim) && IS_SET(victim->pcdata->actnew,NEW_VAN_REHL) )
  { REMOVE_BIT(victim->pcdata->actnew,NEW_VAN_REHL);
    act( ".. and their icy barrier melts!", ch, NULL, victim, TO_CHAR );
    act( ".. and your icy barrier melts!", ch, NULL, victim, TO_VICT );
    act( ".. and their icy barrier melts!", ch, NULL, victim, TO_NOTVICT );
  }
  af.type      = skill_lookup( "flame breath" );
  af.duration  = dice(7,5);
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  chant_damage( ch, victim, dam, cn );
  return;
}

void chant_blast_bomb( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  act( "Dozens of high energy fireballs detonate simultaneously!", ch,NULL,NULL, TO_CHAR );
  act( "Dozens of high energy fireballs detonate simultaneously!", ch,NULL,NULL, TO_ROOM );
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( !is_same_group( ch, vch ) )
    { dam = dice( rank, rank * 5 );
      if ( saves_chant( ch, vch, cn ) )
        dam -= dam / 3;
        if ( !IS_NPC(vch) && IS_SET(vch->pcdata->actnew,NEW_VAN_REHL) )
        { REMOVE_BIT(vch->pcdata->actnew,NEW_VAN_REHL);
          act( ".. and their icy barrier melts!", ch, NULL, vch, TO_CHAR );
          act( ".. and your icy barrier melts!", ch, NULL, vch, TO_VICT );
	  act( ".. and their icy barrier melts!", ch, NULL, vch, TO_NOTVICT );
        }
      chant_damage( ch, vch, dam, cn );
    }
  }
} 
 

void chant_aqua_create( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ int sn = skill_lookup( "create spring" );

  (*skill_table[sn].spell_fun) ( sn, rank, ch, vo );
  return;
} 
void chant_freeze_arrow( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( 10, 40 + rank );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "You fire an arrow of ice at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n fires an arrow of ice at you.", ch, NULL, victim, TO_VICT );
  act( "$n fires an arrow of ice at $n.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
} 
/* --------------------------------------------------------------------------
 *  Dark Mist (#22) -- active-mist room-field registry.
 *
 *  The engine has no room-affect list (ROOM_INDEX_DATA carries no affects), so
 *  a persistent room effect is tracked here in a small fixed-size table.  Each
 *  live entry pins a room, the caster who owns the field, and a tick countdown.
 *  Effects look the room up via is_dark_misted():
 *    - non-allies take -10 hitroll        (fight.c one_hit)
 *    - grouped allies get +15% flee        (fight.c do_flee)
 *    - fire damage burns it away           (fight.c damage)
 *    - it otherwise expires on its timer   (update.c dark_mist_update)
 *  Owner safety: the caster pointer is dropped in extract_char (handler.c) via
 *  dark_mist_owner_gone, so the registry never dereferences a freed CHAR_DATA.
 *  ------------------------------------------------------------------------ */
#define MAX_DARK_MIST 16

typedef struct dark_mist_entry DARK_MIST;
struct dark_mist_entry
{
  ROOM_INDEX_DATA *room;
  CHAR_DATA       *owner;
  int              timer;
};

static DARK_MIST dark_mist_table[MAX_DARK_MIST];

/* Send a line to everyone in a room (no actor needed -- this is a room effect). */
static void dark_mist_echo( ROOM_INDEX_DATA *room, const char *msg )
{
  CHAR_DATA *rch;
  if ( room == NULL )
    return;
  for ( rch = room->people; rch != NULL; rch = rch->next_in_room )
    send_to_char( msg, rch );
}

/* Tear down the entry at slot i, optionally echoing a fade line to its room. */
static void dark_mist_teardown( int i, const char *msg )
{
  ROOM_INDEX_DATA *room = dark_mist_table[i].room;
  dark_mist_table[i].room  = NULL;
  dark_mist_table[i].owner = NULL;
  dark_mist_table[i].timer = 0;
  if ( msg != NULL )
    dark_mist_echo( room, msg );
}

/* Owner of the mist in this room, or NULL if the room is not misted. */
CHAR_DATA *is_dark_misted( ROOM_INDEX_DATA *room )
{
  int i;
  if ( room == NULL )
    return NULL;
  for ( i = 0; i < MAX_DARK_MIST; i++ )
    if ( dark_mist_table[i].room == room && dark_mist_table[i].timer > 0 )
      return dark_mist_table[i].owner;
  return NULL;
}

/* Add or refresh the mist on a room.  Returns FALSE (cast fails) if the room is
 * already misted by someone who is NOT the caster and NOT in the caster's group
 * -- an enemy can't hijack or extend the field -- or if the table is full. */
bool dark_mist_add( ROOM_INDEX_DATA *room, CHAR_DATA *owner, int timer )
{
  int i, slot = -1;
  if ( room == NULL )
    return FALSE;
  for ( i = 0; i < MAX_DARK_MIST; i++ )
  {
    if ( dark_mist_table[i].room == room && dark_mist_table[i].timer > 0 )
    { /* already misted: only the owner or an ally may refresh it (keep owner) */
      if ( dark_mist_table[i].owner != owner
        && !is_same_group( owner, dark_mist_table[i].owner ) )
        return FALSE;
      dark_mist_table[i].timer = timer;
      return TRUE;
    }
    if ( slot < 0 && dark_mist_table[i].timer <= 0 )
      slot = i;
  }
  if ( slot < 0 )
    return FALSE;  /* table full */
  dark_mist_table[slot].room  = room;
  dark_mist_table[slot].owner = owner;
  dark_mist_table[slot].timer = timer;
  return TRUE;
}

/* Fire burns the mist away with a message. */
void dark_mist_clear( ROOM_INDEX_DATA *room )
{
  int i;
  if ( room == NULL )
    return;
  for ( i = 0; i < MAX_DARK_MIST; i++ )
    if ( dark_mist_table[i].room == room && dark_mist_table[i].timer > 0 )
    {
      dark_mist_teardown( i, "The dark mist burns away.\n\r" );
      return;
    }
}

/* Caster extracted (quit/death): drop any field they own (silently). */
void dark_mist_owner_gone( CHAR_DATA *ch )
{
  int i;
  for ( i = 0; i < MAX_DARK_MIST; i++ )
    if ( dark_mist_table[i].owner == ch )
      dark_mist_teardown( i, NULL );
}

/* Tick all live fields; expire (with a message) on 0.  Once per PULSE_TICK. */
void dark_mist_update( void )
{
  int i;
  for ( i = 0; i < MAX_DARK_MIST; i++ )
  {
    if ( dark_mist_table[i].timer <= 0 )
      continue;
    if ( --dark_mist_table[i].timer <= 0 )
      dark_mist_teardown( i, "The dark mist thins and clears.\n\r" );
  }
}

void chant_dark_mist( int cn, int rank, CHAR_DATA *ch, void *vo )
{
  /* #22: Dark Mist is a persistent room field, not a one-shot debuff.  It marks
   * ch->in_room in the active-mist registry with ch as owner; while it lasts,
   * non-allies in the room take -10 hitroll (fight.c) and grouped allies get
   * +15% flee (do_flee).  Fire burns it away, else it expires on its timer.
   * People who enter after the cast are affected -- it's a field, not a
   * one-shot.  vo may be a target or NULL -- ignored; this is room-wide.
   * Scan/vision/ranged penalties from the full design are deferred (phase 1). */
  if ( !dark_mist_add( ch->in_room, ch, 5 ) )
  { /* room already owned by an enemy, or the registry is full */
    send_to_char( "The mists refuse to gather.\n\r", ch );
    return;
  }
  act( "A dark mist spills across the room, swallowing the edges of sight.", ch, NULL, NULL, TO_CHAR );
  act( "A dark mist spills across the room, swallowing the edges of sight.", ch, NULL, NULL, TO_ROOM );
  return;
}
void chant_ly_briem( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int chance = rank;
  chance += ch->pcdata->will / 2;
  if ( IS_NPC(victim) )
  { if ( victim->level > 95 )
      chance = 0;
    else
    chance -= victim->level;
  }
  else
    chance -= victim->max_hit / 100;
  if ( number_percent() > chance )
  { act( "You fail to deep freeze $N.", ch, NULL, victim, TO_CHAR );
    act( "You shiver as your blood runs cold.", ch, NULL, victim, TO_ROOM );
    return;
  }

  act( "$N is frozen solid!", ch, NULL, victim, TO_CHAR );
  act( "Your entire body freeze solid!", ch, NULL, victim, TO_VICT );
  act( "$N is frozen solid!", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, victim->hit + dice( 2, 5 ), cn );
  return;
} 
void chant_freeze_brid( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( rank, rank*3/2 );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "You launch a ball of ice at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n launches a ball of ice at you.", ch, NULL, victim, TO_VICT );
  act( "$n launches a ball of ice at $n.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_icicle_lance( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( rank, rank*3/2 );
  if ( saves_chant( ch, victim, cn ) )
  dam = dam / 2;
  act( "You launch a lance of ice at $N!", ch, NULL, victim, TO_CHAR );
  act( "$n launches a lance of ice at you!", ch, NULL, victim, TO_VICT );
  act( "$n launches a lance of ice at $n.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_sea_cucumber_spin( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *obj;
  char buf[MAX_INPUT_LENGTH];
  int i;

  if ( IS_NPC(victim) )
  {
    stc( "Why bother?  Just kill them.\n\r", ch );
    return;
  }
  if ( victim->pcdata->extras2[EVAL] < 10 )
  {
    stc( "You wouldn't hurt such a youngling, would you?\n\r", ch );
    return;
  }
  if ( victim->pcdata->extras2[EVAL] >= 50 )
  {
    act( "$N skillfully avoids the ravenous sea cucumbers.", ch, NULL, victim, TO_CHAR );
    act( "You skillfully avoid the ravenous sea cucumbers.", ch, NULL, victim, TO_VICT );
    act( "$N skillfully avoids the ravenous sea cucumbers.", ch, NULL, victim, TO_NOTVICT );
    return;
  }
  if ( ( obj = get_obj_list( ch, "spring", ch->in_room->contents ) ) == NULL )
  {
    stc( "There is no water here from which to summon them.\n\r", ch );
    return;
  }

  if ( victim->position > POS_MORTAL )
  {
    act( "$N is buried under a pile of sea cucumbers.\n\rHmmmmm, they taste good picked don't they?", ch, NULL, victim, TO_CHAR );
    act( "You are buried under a pile of sea cucumbers.. ugh!", ch, NULL, victim, TO_VICT );
    act( "$N is buried under a pile of sea cucumbers.\n\rHmmmmm, they taste good picked don't they?", ch, NULL, victim, TO_NOTVICT );
    return;
  }

  act( "A horde of sea cucumbers rise up and consume $N!", ch, NULL, victim, TO_CHAR );
  act( "A horde of sea cucumbers rises up and consumes you!", ch, NULL, victim, TO_VICT );
  act( "A horde of sea cucumbers rise up and consume $N!", ch, NULL, victim, TO_NOTVICT );

  sprintf( buf, "%s has been consumed by angry sea cucumbers.", victim->name );
  do_fatality( ch, buf );
  
  victim->pcdata->extras[TIMER] = 20;
  victim->pcdata->extras[TIE] = 0;
  victim->pcdata->extras2[PKCOUNT] += 5;
  victim->wait = 0;
  victim->level = 1;
  for ( i = 0; i < 10; i++ )
    victim->pcdata->suit[i] = 0;
  raw_kill( victim, TRUE );
  do_save( ch, "auto" );
  return;
} 
void chant_vice_freeze( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ int dam;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  act( "You hurl a freezing ball of ice at $N!", ch, NULL, victim, TO_CHAR );
  act( "$n hurls a freezing ball of ice at you!", ch, NULL, victim, TO_VICT );
  act( "$n hurls a freezing ball of ice at $N!", ch, NULL, victim, TO_NOTVICT );
  dam = dice( rank, rank * 5 / 2 );
  if ( saves_chant( ch, victim, cn ) )
    dam /= 2;
  chant_damage( ch, victim, dam, cn );
  return;
} 
void chant_demona_crystal( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *vch;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch_next;
  int dam;

  act( "Ice crawls toward and erupts beneath $N!", ch, NULL, victim, TO_CHAR );
  act( "Ice crawls toward and erupts beneath you!", ch, NULL, victim, TO_VICT );
  act( "Ice crawls toward and erupts beneath $N!", ch, NULL, victim, TO_NOTVICT );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( (IS_NPC(victim) && IS_NPC(vch)) || is_same_group( victim, vch ) )
    { dam = dice( rank, rank*4 );
      if ( saves_chant( ch, victim, cn ) )
        dam /= 2;
      chant_damage( ch, vch, dam, cn );
    }
  }
  return;
} 
void chant_van_rehl( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  act( "A web of ice spreads out above you.", ch, NULL, NULL, TO_CHAR );
  act( "A web of ice spreads out above $N.", ch, NULL, NULL, TO_ROOM );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  { vch_next = vch->next_in_room;
    if ( vch == ch || is_same_group( ch, vch ) )
    { if ( !IS_SET(vch->pcdata->actnew,NEW_VAN_REHL) )
        SET_BIT(vch->pcdata->actnew,NEW_VAN_REHL);
    }
  }
  return;
} 

void chant_bram_blazer( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int oldhit = victim->hit;
  int dam = dice( 15, 20 + rank );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "A thin bolt of blue energy lances toward $N.", ch, NULL, victim, TO_CHAR );
  act( "A thin bolt of blue energy lances toward you.", ch, NULL, victim, TO_VICT );
  act( "A thin bolt of blue energy lances toward $N.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  if ( victim != NULL && victim->position > POS_STUNNED && dam < victim->mana )
    victim->mana = UMAX( 1, victim->mana - ((oldhit - victim->hit)/(6-(rank/10))) );
  return;
}

void chant_elmekia_lance( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int oldhit = victim->hit;
  int dam = dice( 15, 30 + rank );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "A glowing green arrow streaks at $N.", ch, NULL, victim, TO_CHAR );
  act( "A glowing green arrow streaks at you.", ch, NULL, victim, TO_VICT );
  act( "A glowing green arrow streaks at $N.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  if ( victim != NULL && victim->position > POS_STUNNED && dam < victim->mana )
    victim->mana = UMAX( 1, victim->mana - ((oldhit - victim->hit)/(6-(rank/10))) );
  return;
} 
void chant_assha_dist( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( 30, 30 + rank );
  if ( saves_chant( ch, victim, cn ) )
  { act( "$N dodges the energy waves!", ch, NULL, victim, TO_CHAR );
    act( "You dodge waves of energy!", ch, NULL, victim, TO_CHAR );
    act( "$N dodges the waves of energy.", ch, NULL, victim, TO_NOTVICT );
  }
  else
  { act( "Waves of energy slam into $N!", ch, NULL, victim, TO_CHAR );
    act( "Waves of energy slam into you!", ch, NULL, victim, TO_VICT );
    act( "Waves of energy slam into $N.", ch, NULL, victim, TO_NOTVICT );
    chant_damage( ch, victim, dam, cn );
  }
  return;
} 
void chant_astral_detect( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ char buf[MAX_INPUT_LENGTH];
  char *target = (char *) vo;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
 
  found = FALSE;
  for ( obj = object_list; obj != NULL; obj = obj->next )
  {
    if ( !can_see_obj( ch, obj ) || !is_name( target, obj->name ) )
      continue;
    if ( IS_SET(obj->extra_flags,ITEM_NO_LOCATE) )
      continue;
    found = TRUE;
 
    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
      ;
 
 	if ( in_obj->carried_by != NULL )
 	{
 	    CHAR_DATA *holder = in_obj->carried_by;

 	    /* graded anti-scry vs a defenses-raised Patryn holding the object,
 	       mirroring chant_vision's Air-torso hook */
 	    if ( IS_CLASS(holder,CLASS_PATRYN) && holder->level >= 2
 		&& IS_SET(holder->pcdata->powers[P_BITS],P_DEFENSES)
 		&& get_runes(holder,RUNE_AIR,TORSO) > 0 )
 	    {
 		int acount = get_runes(holder,RUNE_AIR,TORSO);

 		if ( acount == 1 )
 		    act( "Your tattoos glow with a frosty blue aura.", holder, NULL, NULL, TO_CHAR );
 		else
 		    act( "Your tattoos glow frosty blue, as if probed from afar.", holder, NULL, NULL, TO_CHAR );

 		act( "$n's tattoos glow with a frosty blue aura.", holder, NULL, NULL, TO_ROOM );

 		/* 5+ Air runes or spirit ward: the detect fails for this item */
 		if ( acount >= 5 || is_affected( holder, gsn_spirit_ward ) )
 		    continue;
 		/* 3-4 Air runes: clouded, suppress holder and location */
 		if ( acount >= 3 )
 		{
 		    send_to_char( "The vision clouds...\n\r", ch );
 		    continue;
 		}
 		/* 1-2 Air runes: warn only, reveal still succeeds below */
 	    }

 	    sprintf( buf, "%s carried by %s.\n\r",
 		obj->short_descr, PERS(in_obj->carried_by, ch) );
 	}
 	else
 	{
 	    sprintf( buf, "%s in %s.\n\r",
 		obj->short_descr, in_obj->in_room == NULL
 		    ? "somewhere" : in_obj->in_room->name );
 	}
 
 	buf[0] = UPPER(buf[0]);
 	send_to_char( buf, ch );
     }
 
     if ( !found )
 	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
  return;
} 
void chant_astral_break( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam = dice( rank, rank*3 );
  if ( saves_chant( ch, victim, cn ) )
  { act( "$N dodges the blast of energy!", ch, NULL, victim, TO_CHAR );
    act( "You dodge a blast of energy!", ch, NULL, victim, TO_CHAR );
    act( "$N dodges the blast of energy.", ch, NULL, victim, TO_NOTVICT );
  }
  else
  { act( "A blast of energy slams into $N!", ch, NULL, victim, TO_CHAR );
    act( "A blast of energy slams into you!", ch, NULL, victim, TO_VICT );
    act( "A blast of energy slams into $N.", ch, NULL, victim, TO_NOTVICT );
    chant_damage( ch, victim, dam, cn );
  }
  return;
} 
void chant_shadow_snap( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if ( IS_AFFECTED( victim, AFF_NO_MOVE ) )
  { send_to_char( "They're already stuck, why bother?", ch );
    return;
  }
  if ( saves_chant( ch, victim, cn ) || saves_chant( ch, victim, cn ) )
  { act( "$N dodges your shadow snap!", ch, NULL, victim, TO_CHAR );
    act( "You dodge $n's shadow snap!", ch, NULL, victim, TO_VICT );
    act( "$N dodges $n's shadow snap.", ch, NULL, victim, TO_NOTVICT );
  }
  else
  { act( "Your shadow dagger pins $N's shadow to the ground!", ch, NULL, victim, TO_CHAR );
    act( "$n's shadow dagger pins your shadow to the ground!", ch, NULL, victim, TO_VICT );
    act( "$n's shadow dagger pins $N's shadow to the ground.", ch, NULL, victim, TO_NOTVICT );
    af.type      = 0;
    af.duration  = dice( 2, rank ) - 1;
    af.location  = APPLY_AC;
    af.modifier  = dice( 2, rank );
    af.bitvector = AFF_NO_MOVE;
    affect_to_char( victim, &af );    
  }
  return;
} 
void chant_elmekia_flame( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int oldhit = victim->hit;
  int dam = dice( rank, rank*2 );
  if ( saves_chant( ch, victim, cn ) )
    dam = dam / 2;
  act( "A tremendous blast of green flame slams into $N!", ch, NULL, victim, TO_CHAR );
  act( "A tremendous blast of green flame slams into you!", ch, NULL, victim, TO_VICT );
  act( "A tremendous blast of green flame slams into $N.", ch, NULL, victim, TO_NOTVICT );
  chant_damage( ch, victim, dam, cn );
  if ( victim != NULL && victim->position > POS_STUNNED && dam < victim->mana )
    victim->mana = UMAX( 1, victim->mana - ((oldhit - victim->hit)/(6-(rank/10))) );
  return;
} 
void chant_astral_vine( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  OBJ_DATA *obj;
  char *arg = (char *) vo;
  int hit = 0, dam = 0, chance;


  if ( arg[0] == '\0' )
  { send_to_char( "Enchant what?\n\r", ch );
    return;
  }
  if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
  { send_to_char( "You don't have that.\n\r", ch );
    return;
  }
  if ( IS_OBJ_STAT( obj, ITEM_UNIQUE ) )
  { send_to_char( "Don't fuck with uniques.\n\r", ch );
    return;
  }
  if ( IS_OBJ_STAT( obj, ITEM_HARDENED) )
  {
    stc( "It's too hard.\n\r", ch );
    return;
  }
  if ( obj->item_type != ITEM_WEAPON )
  { send_to_char( "Only on weapons.\n\r", ch );
    return;
  }

  chance = number_percent() - (50 - ch->pcdata->powers[SCHOOL_ASTRAL]);

  for ( paf = obj->affected; paf != NULL; paf = paf_next ) 
  { paf_next = paf->next;
    if ( paf->location == APPLY_HITROLL )
    { hit += paf->modifier;
      obj_affect_remove( obj, paf );
    }
    if ( paf->location == APPLY_DAMROLL )
    { dam += paf->modifier;
      obj_affect_remove( obj, paf );
    }
  }

  chance -= (hit + dam)*3/2;

  /* AC bonuses make it harder to enchant */
  for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf_next )
  {
    paf_next = paf->next;
    if ( paf->location == APPLY_AC )
      chance += URANGE( -10, paf->modifier / 3, 10 );
  }

  if ( chance < 15 )
  { act( "$p shivers, then disintegrates.", ch, obj, NULL, TO_CHAR );
    act( "$p shivers, then disintegrates.", ch, obj, NULL, TO_ROOM );
    extract_obj( obj );
    return;
  }
  else if ( chance < 45 )
  { act( "Nothing happens.", ch, NULL, NULL, TO_CHAR );
  }
  else if ( chance < 85 )
  { act( "$p shimmers with crimson energy.", ch, obj, NULL, TO_CHAR );
    act( "$p shimmers with crimson energy.", ch, obj, NULL, TO_ROOM );
    hit++;
    dam++;
  }
  else
  { act( "$p glows brightly with crimson flame!", ch, obj, NULL, TO_CHAR );
    act( "$p glows brightly with crimson flame!", ch, obj, NULL, TO_ROOM );
    hit += 2;
    dam += 2;
  }

  if ( affect_free == NULL )
    paf             = alloc_perm( sizeof(*paf), PERM_AFF );
  else
  { paf             = affect_free;
    affect_free     = affect_free->next;
  }

  paf->type           = skill_lookup( "enchant weapon" );
  paf->duration       = -1;
  paf->location       = APPLY_HITROLL;
  paf->modifier       = hit;
  paf->bitvector      = 0;
  paf->next           = obj->affected;
  obj->affected       = paf;

  if ( affect_free == NULL )
    paf             = alloc_perm( sizeof(*paf), PERM_AFF );
  else
  { paf             = affect_free;
    affect_free     = affect_free->next;
  }

  paf->type           = skill_lookup( "enchant weapon" );
  paf->duration       = -1;
  paf->location       = APPLY_DAMROLL;
  paf->modifier       = dam;
  paf->bitvector      = 0;
  paf->next           = obj->affected;
  obj->affected       = paf;

  return;
} 
void chant_vision( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ char *arg = (char *) vo;
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *original;

  if ( ch->fighting != NULL )
  { send_to_char( "You can't concentrate enough.\n\r", ch ); 
    return;
  }
  if ( arg == NULL || arg[0] == '\0' )
  { send_to_char( "Attempt to scry who?\n\r", ch );
    return;
  }
  if ( ( victim = get_char_world( ch, arg ) ) == NULL || saves_chant( ch, victim, cn ) )
  { send_to_char( "You fail to locate them.\n\r", ch ); 
    return;
  }

  if ( victim == ch )
  { send_to_char( "Go find a mirror.\n\r", ch );
    return;
  }
  if ( victim->in_room == NULL )
    return;

  if ( IS_CLASS(victim,CLASS_PATRYN) && get_runes(victim,RUNE_AIR,TORSO) > 0
    && victim->level >= 2 && IS_SET(victim->pcdata->powers[P_BITS],P_DEFENSES) )
  {
    int acount = get_runes(victim,RUNE_AIR,TORSO);

    if ( acount == 1 )
      act( "Your tattoos glow with a frosty blue aura.", victim, NULL, NULL, TO_CHAR );
    else
      act( "Your tattoos glow frosty blue, and a vision of $N flashes before your eyes.", victim, NULL, ch, TO_CHAR );

    act( "$n's tattoos glow with a frosty blue aura.", victim, NULL, NULL, TO_ROOM );

    /* graded anti-scry: 5+ Air runes or spirit ward fully blocks the vision */
    if ( acount >= 5 || is_affected( victim, gsn_spirit_ward ) )
    {
      send_to_char( "You fail to locate them.\n\r", ch );
      return;
    }
    /* 3-4 Air runes: vision dissolves, no room snapshot */
    if ( acount >= 3 )
    {
      send_to_char( "The vision dissolves into static.\n\r", ch );
      return;
    }
    /* 1-2 Air runes: warn only, vision succeeds below */
  }

  if ( victim->in_room->vnum == 1 ||
      (victim->in_room->vnum >= 8400 && victim->in_room->vnum <= 8599) ||
      (victim->in_room->vnum >= 8100 && victim->in_room->vnum <= 8299) )
  {
    act( "Electricity streaks through the astral and into your body!", ch, NULL, NULL, TO_CHAR );
    act( "$n writhes in agony as their body courses with electricity!", ch, NULL, NULL, TO_ROOM );
    damage( ch, ch, 29999, DAM_SHOCKSHIELD );
    WAIT_STATE(ch,40);
    return;
  }

  original = ch->in_room;
  char_from_room( ch ); 
  char_to_room( ch, victim->in_room );
  do_look( ch, "auto" );
  char_from_room( ch );
  char_to_room( ch, original );
  return;
} 

void chant_ra_tilt( int cn, int rank, CHAR_DATA *ch, void *vo ) 
{ CHAR_DATA *victim = (CHAR_DATA *) vo;
  int chance = number_percent();
  int counter = 0;
  int dam;

  act( "An awesome column of spiritual energy erupts beneath $N!",ch,NULL,victim,TO_CHAR );
  act( "An awesome column of spiritual energy erupts beneath you!",ch,NULL,victim,TO_VICT );
  act( "An awesome column of spiritual energy erupts beneath $N!",ch,NULL,victim,TO_NOTVICT);

  if ( IS_NPC(victim) )
  { chance += 100 - victim->level;
    dam = dice( rank, rank*8 );
    if ( victim->level > 95 )
      chance -= 20;
  }
  else
  { chance -= (victim->hit + victim->max_hit) / 1500;
    dam = dice( rank, 400 - (victim->pcdata->extras2[EVAL]*3) );
    if ( victim->level >= 17 )
      chance = 0;
  }

  /* fuck this, I've had it with pricks like him */
  if ( !str_cmp( ch->name, "Tilt" ) )
    chance /= 2;

  if ( chance <= 20 )
  { act( "... but $N manages to resist the attack!", ch, NULL, victim, TO_CHAR );
    act( "... but you manage to resist the attack!", ch, NULL, victim, TO_VICT );
    act( "... but $N manages to resist the attack.", ch, NULL, victim, TO_NOTVICT );
  }
  else if ( chance <= 80 )
  { chant_damage( ch, victim, dam , cn );
  }
  else
  { act( "$N is completely consumed!", ch, NULL, victim, TO_CHAR );
    do_yell( victim, "AAAAAAAAAAAAAaaaaaaaarrrrggggggggggggghhhhhhhhh!!!" );
    while ( victim != NULL && counter < 10 )
    { counter++;
      chant_damage( ch, victim, 10000, cn );
    }
  }
  return;
} 

void do_prepare( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int school = 0;
  int i = 0;

  if ( IS_NPC(ch) )
    return;
  if ( ch->class != CLASS_SORCERER )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Syntax is 'prepare <spell school>'.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg, "none" ) )
  {
    ch->pcdata->powers[SORC_PREP] = 0;
    stc( "You stop preparing spells.\n\r", ch );
    return;
  }
  else if ( !str_prefix( arg, "black" ) )
    school = SCHOOL_BLACK;
  else if ( !str_prefix( arg, "earth" ) )
    school = SCHOOL_EARTH;
  else if ( !str_prefix( arg, "wind" ) )
    school = SCHOOL_WIND;
  else if ( !str_prefix( arg, "fire" ) )
    school = SCHOOL_FIRE;
  else if ( !str_prefix( arg, "water" ) )
    school = SCHOOL_WATER;
  else if ( !str_prefix( arg, "astral" ) )
    school = SCHOOL_ASTRAL;
  else
  {
    stc( "You can't prepare attacks from that school.\n\r", ch );
    return;
  }

  for ( i = MAX_CHANT - 1; i > 0; i-- )	/* issue #47: was i = MAX_CHANT -> OOB read of chant_table[MAX_CHANT] on the first iteration. Only the OOB read is fixed; index-0 handling is deliberately unchanged (see issue #47 discussion) */
  {
    if ( chant_table[i].rank <= ch->pcdata->powers[school] &&
         chant_table[i].prep == TRUE &&
         chant_table[i].school == school )
    {
      break;
    }
  }

  if ( i <= 0 )
  {
    stc( "You know no prepareable spells from that school.\n\r", ch );
    return;
  }

  ch->pcdata->powers[SORC_PREP] = school;
  sprintf( buf, "You prepare to cast %s.\n\r", chant_table[i].name );
  stc( buf, ch );
  WAIT_STATE( ch, 4 );
  return;
}

void do_concentrate( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  if ( !IS_CLASS(ch,CLASS_SORCERER) )
  {
    stc( "Huh?\n\r", ch );
    return;
  }
  if ( IS_SET(ch->pcdata->actnew,NEW_CONCENTRATE) )
  {
    REMOVE_BIT(ch->pcdata->actnew,NEW_CONCENTRATE);
    stc( "You cease gathering mystic energies.\n\r", ch );
    return;
  }
  if ( ch->mana < 100 )
  {
    stc( "You simply don't have the power.\n\r", ch );
    return;
  }

  SET_BIT(ch->pcdata->actnew,NEW_CONCENTRATE);
  stc( "You begin concentrating.\n\r", ch );
  WAIT_STATE(ch,4);
  return;
}



const	struct	chant_type	chant_table	[MAX_CHANT]	=
{
    {
	"balus rod",	chant_balus_rod,	SCHOOL_BLACK, TRUE,
	1,	200,	1,	4,	TAR_CHAR_OFFENSIVE,
	"`rBalus Rod!`n", "", "", "", "", 2, 1
    },

    {
    	"disfang",	chant_disfang,		SCHOOL_BLACK, FALSE,
    	4,	300,	1,	8,	TAR_CHAR_OFFENSIVE,
    	"`dDisfang!`n", "", "", "", "", 2, 1
    },

    {
    	"dolph zork",	chant_dolph_zork,	SCHOOL_BLACK, FALSE,
    	7,	1000,	1,	8,	TAR_IGNORE,
    	"`bDolph Zork!`n", "", "", "", "", 8, 1
    },

    {
    	"dynast brass",	chant_dynast_brass,	SCHOOL_BLACK, TRUE,
    	10,	500,	1,	8,	TAR_CHAR_OFFENSIVE,
    	"`rDynast Brass!`n", "", "", "", "", 3, 1
    },

    {
    	"blast ash",	chant_blast_ash,	SCHOOL_BLACK, FALSE,
    	13,	1000,	1,	12,	TAR_CHAR_OFFENSIVE,
    	"`dBlast Ash!`n", "", "", "", "", 4, 1
    },

    {
    	"ferrous bleed",chant_ferrous_bleed,	SCHOOL_BLACK, FALSE,
    	16,	1500,	1,	8,	TAR_CHAR_OFFENSIVE,
    	"`rFerrous BLeed!`n", "", "", "", "", 4, 1
    },

    {
    	"hell blast",	chant_hell_blast,	SCHOOL_BLACK, FALSE,
    	19,     1200,	1,	10,	TAR_CHAR_OFFENSIVE,
    	"`dHell Blast!`n", "", "", "", "", 6, 1
    },

    {
    	"zelas brid",	chant_zelas_brid,	SCHOOL_BLACK, TRUE,
    	22,	800,	1,	4,	TAR_CHAR_OFFENSIVE,
    	"`dZelas Brid!`n", "", "", "", "", 2, 6
    },

    {
    	"dolph strash",	chant_dolph_strash,	SCHOOL_BLACK, FALSE,
    	25,	1000,	1,	6,	TAR_CHAR_OFFENSIVE,
    	"`bDolph Strash!`n", "", "", "", "", 2, 1
    },

    {
    	"rubyeye blade",chant_ruby_eye_blade,	SCHOOL_BLACK, FALSE,
    	28,	3000,	3,	8,	TAR_IGNORE,
    	"`rSword of the Demon King, burning with your rage..`n",
    	"`rCome to me now so they may feel our wrath!`n",
    	"`rRuby-eye Blade!`n", "", "", 10, 1
    },

    {
    	"dynast breath",chant_dynast_breath,	SCHOOL_BLACK, FALSE,
    	34,	3000,	2,	8,	TAR_CHAR_OFFENSIVE,
    	"`dSupreme king with the frozen soul, grant us the power of your icy rage!`n",
        "`dDynast Breath!`n", "", "", "", 5, 1
    },

    {
        "gaav flare",   chant_gaav_flare,       SCHOOL_BLACK, FALSE,
        37,     2000,    1,     9,     TAR_CHAR_OFFENSIVE,
        "`rGaav Flare!`n", "", "", "", "", 3, 1
    },

    {
    	"laguna blast",	chant_laguna_blast,	SCHOOL_BLACK, FALSE,
    	40,	2000,	2,	8,	TAR_CHAR_OFFENSIVE,
    	"`dUnbounded chaos, engulf those before me!`n",
    	"`dLaguna Blast!`n", "", "", "", 10, 1
    },

    {
    	"flame breath",	chant_flame_breath,	SCHOOL_BLACK, FALSE,
    	45,	1200,	2,	8,	TAR_CHAR_OFFENSIVE,
    	"`rAteku.. Ra-zu-omu.. Ranaku-tsuraku.. Izu peku naomu..`n",
    	"`rFlame Breath!`n", "", "", "", 10, 1
    },

    {
    	"dragon slave",	chant_dragon_slave,	SCHOOL_BLACK, FALSE,
    	50,	3000,	5,	8,	TAR_CHAR_OFFENSIVE,
    	"`rDarkness beyond twilight, crimson beyond blood that flows.`n",
    	"`rBuried in the stream of time is where your power grows.`n",
    	"`rI pledge myself to conquer all those who stand before us!`n",
    	"`rLet the fools be destroyed by the power you and I possess!`n",
    	"`rDRAGON SLAVE!!`n", 10, 1
    },

    {
    	"laguna blade",	chant_laguna_blade,	SCHOOL_BLACK, FALSE,
    	50,	6000,	5,	8,	TAR_IGNORE,
    	"`dLord of the dreams that terrify..`n",
    	"`dSword of cold and darkness, free yourself from the heavens bond.`n",
    	"`dBecome one with my power one with my body..`n",
    	"`dand let us walk the path of destruction together!`n",
    	"`dPower that can smash even the souls of the Gods -- LAGUNA BLADE!!`n",
    	12, 1
    },

    {
    	"lighting",	chant_lighting,		SCHOOL_WHITE, FALSE,
    	1,	350,	1,	8,	TAR_CHAR_OFFENSIVE,
    	"`WLighting!`n", "", "", "", "", 3, 1
    },

    {
    	"dicleary",	chant_dicleary,		SCHOOL_WHITE, FALSE,
    	4,	400,	1,	8,	TAR_CHAR_DEFENSIVE,
    	"`WDicleary!`n", "", "", "", "", 4, 1
    },

    {
    	"holy bless",	chant_holy_bless,	SCHOOL_WHITE, FALSE,
    	7,	750,	2,	8,	TAR_CHAR_SELF,
    	"`WGuide my strike that it may be true!`n",
    	"`WHoly Bless!`n", "", "", "", 2, 1
    },

    {
    	"flow break",	chant_flow_break,	SCHOOL_WHITE, FALSE,
    	25,	1500,	1,	8,	TAR_CHAR_OFFENSIVE,
    	"`WFlow Break!`n", "", "", "", "", 8, 1
    },

    {
        "laphas seed",  chant_laphas_seed,      SCHOOL_WHITE, FALSE,
        13,     1200,   1,      8,      TAR_CHAR_OFFENSIVE,
        "`YLaphas Seed!`n", "", "", "", "", 6, 1
    },

    {
    	"recovery",	chant_recovery,		SCHOOL_WHITE, FALSE,
    	16,	1000,	1,	8,	TAR_CHAR_DEFENSIVE,
    	"`WRecovery!`n", "", "", "", "", 3, 1
    },

    {
    	"sleeping",	chant_sleeping,		SCHOOL_WHITE, FALSE,
    	19,	2500,	1,	12,	TAR_IGNORE,
    	"`YSleeping!`n", "", "", "", "", 8, 1
    },

    {
    	"mos varim",	chant_mos_varim,	SCHOOL_WHITE, FALSE,
    	22,	500,	1,	8,	TAR_CHAR_SELF,
    	"`WMos Varim!`n", "", "", "", "", 4, 1
    },

    {
    	"visfarank",	chant_visfarank,	SCHOOL_WHITE, FALSE,
    	10,	1500,	1,	12,	TAR_IGNORE,
    	"`YVisfarank!`n", "", "", "", "", 4, 1
    },

    {
    	"vas gluudo",	chant_vas_gluudo,	SCHOOL_WHITE, FALSE,
    	28,	1500,	1,	8,	TAR_CHAR_SELF,
    	"`WVas Gluudo!`n", "", "", "", "", 4, 1
    },

    {
        "chaos strings",        chant_chaos_strings,    SCHOOL_WHITE, FALSE,
        31,     1000,   1,      10,      TAR_CHAR_OFFENSIVE,
        "`YChaos String!`n", "", "", "", "", 8, 1
    },

    {
    	"holy resist",	chant_holy_resist,	SCHOOL_WHITE, FALSE,
    	34,	800,	2,	8,	TAR_CHAR_SELF,
    	"`WHoly spirits become my shield!`n",
    	"`WHoly Resist!`n", "", "", "", 8, 1
    },

    {
    	"defense",	chant_defense,		SCHOOL_WHITE, FALSE,
    	37,	3000,	1,	10,	TAR_CHAR_SELF,
    	"`WBarrier against destruction come forth! Defense!`n",
    	"`WDefense!`n", "", "", "", 8, 1
    },

    {
    	"chaotic disintegrate",	chant_chaotic_disintegrate,	SCHOOL_WHITE, FALSE,
    	45,	1500,	3,	8,	TAR_CHAR_OFFENSIVE,
    	"`YAnakusolum..`n",
    	"`YNagakusacreen..`n",
    	"`YChaotic Disintegrate!!`n", "", "", 8, 1
    },

    {
    	"ressurection",	chant_ressurection,	SCHOOL_WHITE, FALSE,
    	50,	2500,	4,	8,	TAR_CHAR_DEFENSIVE,
    	"`WOh blessed and humble hand of God..`n",
    	"`WLife and breath of Mother Earth..`n",
    	"`WCome before me and show your great compassion and deliver us!`n",
    	"`WRessurection!!`n", "", 10, 1
    },

    {
	"levitation",	chant_levitation,	SCHOOL_WIND, FALSE,
	1,	200,	1,	10,	TAR_CHAR_SELF,
	"`CLevitation!`n", "", "", "", "", 8, 1
    },

    {
	"scatter brid",	chant_scatter_brid,	SCHOOL_WIND, TRUE,
	5,	200,	1,	8,	TAR_CHAR_OFFENSIVE,
	"`CScatter Brid`n", "", "", "", "", 3, 1
    },

    {
	"digger bolt",	chant_digger_bolt,	SCHOOL_WIND, FALSE,
	10,	300,	1,	6,	TAR_CHAR_OFFENSIVE,
	"`CDigger Bolt!`n", "", "", "", "", 2, 1
    },

    {
	"diem wing",	chant_diem_wing,	SCHOOL_WIND, FALSE,
	15,	500,	 1,	6,	TAR_CHAR_OFFENSIVE,
	"`CDiem Wing!`n", "", "", "", "", 2, 1
    },

    {
	"damu brass",	chant_damu_brass,	SCHOOL_WIND, TRUE,
	20,	400,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`CDamu Brass!`n", "", "", "", "", 4, 1
    },

    {
	"raywing",	chant_raywing,		SCHOOL_WIND, FALSE,
	25,	1000,	1,	12,	TAR_CHAR_SELF,
	"`CRaywing!`n", "", "", "", "", 8, 1
    },

    {
	"mono volt",	chant_mono_volt,	SCHOOL_WIND, FALSE,
	30,	1200,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`CMono Volt!`n", "", "", "", "", 4, 1
    },

    {
	"dimilar ai",	chant_dimilar_ai,	SCHOOL_WIND, TRUE,
	35,	1250,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`CDimilar Ai!`n", "", "", "", "", 8, 1
    },

    {
	"windy shield",	chant_windy_shield,	SCHOOL_WIND, FALSE,
	40,	1000,	1,	12,	TAR_CHAR_SELF,
	"`CWindy Shield!`n", "", "", "", "", 8, 1
    },

    {
	"bomb di wind",	chant_bomb_di_wind,	SCHOOL_WIND, FALSE,
	50,	1500,	4,	8,	TAR_CHAR_OFFENSIVE,
	"`CAround us, above us, ever flowing breath of the north..`n",
	"`CI beseech you, come forth from the peaks..`n",
	"`CAnd surround us with your icy fury!`n",
	"`CBomb di Wind!`n", "", 8, 1
    },

    {
	"gray bomb",	chant_gray_bomb,	SCHOOL_EARTH, FALSE,
	1,	150,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`yGray Bomb!`n", "", "", "", "", 8, 1
    },

    {
	"earth 5",	chant_earth_5,		SCHOOL_EARTH, FALSE,
	5,	1,	1,	12,	TAR_CHAR_SELF,
	"nothing", "", "", "", "", 8, 1
    },

    {
	"dill brand",	chant_dill_brand,	SCHOOL_EARTH, FALSE,
	10,	750,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`yDill Brand!`n", "", "", "", "", 4, 1
    },
    
    {
	"dug wave",	chant_dug_wave,		SCHOOL_EARTH, TRUE, 
	15,	250,	1,	8,	TAR_CHAR_OFFENSIVE,
	"`yDug Break!`n", "", "", "", "", 8, 1
    },

    {
	"blade haut",	chant_blade_haut,	SCHOOL_EARTH, FALSE,
	20,	900,	2,	9,	TAR_CHAR_OFFENSIVE,
	"`yEarth below me, take edge and strike!`n",
	"`yBlade Haut!`n", "", "", "", 12, 1
    },

    {
	"bephis bring",	chant_bephis_bring,	SCHOOL_EARTH, FALSE,
	25,	500,	1,	12,	TAR_CHAR_OFFENSIVE,
	"`yBephis Bring!`n", "", "", "", "", 8, 1
    },

    {
	"vigarthagaia",	chant_vigarthagaia,	SCHOOL_EARTH, FALSE,
	30,	500,	1,	12,	TAR_CHAR_SELF,
	"`yVigarthagaia`n", "", "", "", "", 12, 1
    },

    {
	"mega brand",	chant_mega_brand,	SCHOOL_EARTH, FALSE,
	35,	1500,	1,	14,	TAR_CHAR_OFFENSIVE,
	"`yMega Brand!`n", "", "", "", "", 12, 1
    },

    {
	"dug haut",	chant_dug_haut,		SCHOOL_EARTH, FALSE,
	40,	1200,	2,	10,	TAR_IGNORE,
	"`yEarth below me, submit to my will!`n",
	"`yDug Haut!`n", "", "", "", 9, 1
    },

    {
	"vlave howl",	chant_vlave_howl,	SCHOOL_EARTH, FALSE,
	50,	1400,	3,	12,	TAR_CHAR_OFFENSIVE,
	"`yInfinite earth, mother who nurtures all life`n",
	"`yLet thy power gather in my hand!`n",
	"`yVlave Howl!`n", "", "", 12, 1
    },

    {
	"flare bit",	chant_flare_bit,	SCHOOL_FIRE, TRUE,
	1,	125,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`RFlare Bit!`n", "", "", "", "", 5, 1
    },

    {
	"flare arrow",	chant_flare_arrow,	SCHOOL_FIRE, TRUE,
	5,	150,	1,	7,	TAR_CHAR_OFFENSIVE,
	"`RFlare Arrow!`n", "", "", "", "", 3, 1
    },

    {
	"val flare",	chant_val_flare,	SCHOOL_FIRE, FALSE,
	10,	330,	2,	7,	TAR_CHAR_OFFENSIVE,
	"`RSource of all power, crimson fire burning bright.`n",
	"`RVal Flare!`n", "", "", "", 3, 1
    },

    {
	"burst rondo",	chant_burst_rondo,	SCHOOL_FIRE, TRUE,
	15,	455,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`RBurst Rondo!`n", "", "", "", "", 6, 1
    },

    {
	"fireball",	chant_fireball,		SCHOOL_FIRE, TRUE,
	20,	750,	2,	6,	TAR_IGNORE,
	"`RSource of all power, crimson fire burning bright.`n",
	"`RFireball!`n", "", "", "", 8, 1
    },

    {
	"flare lance",	chant_flare_lance,	SCHOOL_FIRE, TRUE,
	25,	725,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`RFlare Lance!`n", "", "", "", "", 4, 1
    },

    {
	"rune flare",	chant_rune_flare,	SCHOOL_FIRE, TRUE,
	30,	950,	1,	12,	TAR_CHAR_OFFENSIVE,
	"`RRune Flare!`n", "", "", "", "", 8, 1
    },

    {
	"balus wall",	chant_balus_wall,	SCHOOL_FIRE, FALSE,
	35,	900,	1,	9,	TAR_CHAR_SELF,
	"`RBalus Wall!`n", "", "", "", "", 6, 1
    },

    {
	"burst flare",	chant_burst_flare,	SCHOOL_FIRE, FALSE,
	40,	1200,	3,	10,	TAR_CHAR_OFFENSIVE,
	"`RSource of all power, crimson fire burning bright.`n",
	"`RGather in my hands and become an inferno!`n",
	"`RBurst Flare!`n", "", "", 8, 1
    },

    {
	"blast bomb",	chant_blast_bomb,	SCHOOL_FIRE, FALSE,
	50,	4000,	3,	12,	TAR_IGNORE,
	"`RSource of all power, crimson fire burning bright.`n",
	"`RLet thy true nature come forth in glorious flame!`n",
	"`RBlast Bomb!!`n", "", "", 12, 1
    },

    {
	"aqua create",	chant_aqua_create,	SCHOOL_WATER, FALSE,
	1,	125,	1,	12,	TAR_IGNORE,
	"`BAqua Create!`n", "", "", "", "", 10, 1
    },

    {
	"freeze arrow",	chant_freeze_arrow,	SCHOOL_WATER, TRUE,
	5,	150,	1,	7,	TAR_CHAR_OFFENSIVE,
	"`BFreeze Arrow!`n", "", "", "", "", 4, 1
    },

    {
	"dark mist",	chant_dark_mist,	SCHOOL_WATER, FALSE,
	10,	100,	1,	15,	TAR_IGNORE,
	"`BDark Mist!`n", "", "", "", "", 12, 1
    },

    {
	"ly briem",	chant_ly_briem,		SCHOOL_WATER, FALSE,
	15,	1000,	1,	13,	TAR_CHAR_OFFENSIVE,
	"`BLy Briem!`n", "", "", "", "", 12, 1
    },

    {
	"freeze brid",	chant_freeze_brid,	SCHOOL_WATER, TRUE,
	20,	450,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`BFreeze Brid!`n", "", "", "", "", 8, 1
    },

    {
	"icicle lance",	chant_icicle_lance,	SCHOOL_WATER, TRUE,
	25,	625,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`BIcicle Lance!`n", "", "", "", "", 6, 1
    },

    {
	"sea cucumber spin", chant_sea_cucumber_spin, SCHOOL_WATER, FALSE,
	30,	750,	1,	12,	TAR_CHAR_OFFENSIVE,
	"`BSea Cucumber Spin!`n", "", "", "", "", 10, 1
    },

    {
	"vice freeze",	chant_vice_freeze,	SCHOOL_WATER, FALSE,
	35,	2500,	1,	14,	TAR_CHAR_OFFENSIVE,
	"`BVice Freeze!`n", "", "", "", "", 12, 1
    },

    {
	"demona crystal",chant_demona_crystal,	SCHOOL_WATER, FALSE,
	40,	1500,	3,	10,	TAR_CHAR_OFFENSIVE,
	"`BYou who crosses between sky and earth, gently flowing water`n",
	"`BGather in my hand and give me power!`n",
	"`BDemona Crystal!`n", "", "", 10, 1
    },

    {
	"van rehl",	chant_van_rehl,		SCHOOL_WATER, FALSE,
	50,	1500,	3,	10,	TAR_CHAR_OFFENSIVE,
	"`BHoly wind, wind which flows gently across the land`n",
	"`BLet all things be filled with your pure breath!`n",
	"`BVan Rehl!`n", "", "", 10, 1
    },

    {
	"bram blazer",	chant_bram_blazer,	SCHOOL_ASTRAL, TRUE,
	1,	130,	1,	4,	TAR_CHAR_OFFENSIVE,
	"`GBram Blazer!`n", "", "", "", "", 4, 1
    },

    {
	"elmekia lance",chant_elmekia_lance,	SCHOOL_ASTRAL, TRUE,
	5,	150,	1,	6,	TAR_CHAR_OFFENSIVE,
	"`GElmekia Lance!`n", "", "", "", "", 5, 1
    },

    {
	"assha dist",	chant_assha_dist,	SCHOOL_ASTRAL, FALSE,
	10,	200,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`GAssha Dist!`n", "", "", "", "", 8, 1
    },

    {
	"astral detect",chant_astral_detect,	SCHOOL_ASTRAL, FALSE,
	15,	350,	1,	12,	TAR_IGNORE,
	"`GAstral Detect!`n", "", "", "", "", 10, 1
    },

    {
	"astral break",	chant_astral_break,	SCHOOL_ASTRAL, FALSE,
	20,	600,	1,	10,	TAR_CHAR_OFFENSIVE,
	"`GAstral Break!`n", "", "", "", "", 9, 1
    },

    {
	"shadow snap",	chant_shadow_snap,	SCHOOL_ASTRAL, FALSE,
	25,	500,	1,	12,	TAR_CHAR_OFFENSIVE,
	"`GShadow Snap!`n", "", "", "", "", 10, 1
    },

    {
	"elmekia flame",chant_elmekia_flame,	SCHOOL_ASTRAL, FALSE,
	30,	2250,	1,	12,	TAR_CHAR_OFFENSIVE,
	"`GElmekia Flame!`n", "", "", "", "", 10, 1
    },

    {
	"astral vine",	chant_astral_vine,	SCHOOL_ASTRAL, FALSE,
	35,	3000,	1,	12,	TAR_IGNORE,
	"`GAstral Vine!`n", "", "", "", "", 10, 1
    },

    {
	"vision",	chant_vision,		SCHOOL_ASTRAL, FALSE,
	40,	1500,	1,	12,	TAR_IGNORE,
	"`GVision!`n", "", "", "", "", 10, 1
    },

    {
	"ra-tilt",	chant_ra_tilt,		SCHOOL_ASTRAL, FALSE,
	50,	4000,	4,	12,	TAR_CHAR_OFFENSIVE,
	"`GSource of all souls which dwell in the eternal and infinite.`n",
	"`GEverlasting flame of blue.`n",
	"`GLet the power hidden within my soul be called forth from the infinite!`n",
	"`GRa-Tilt!!`n", "", 10, 1
    }

};



