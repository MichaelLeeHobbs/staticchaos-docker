/*
 * gmcp.c -- Generic Mud Communication Protocol (telnet option 201).
 *
 * Phase 0: telnet negotiation, an IAC filter that pulls GMCP (and other telnet)
 * sequences out of the raw input stream before the line parser sees them, and a
 * send helper.  Opt-in: nothing is sent unless the client negotiated GMCP, so
 * non-GMCP clients (raw telnet, etc.) are completely unaffected.
 *
 * Phase 1: Char.Vitals -- pushes hp/mana/move once per second (heartbeat from
 * second_update) so a Mudlet client can draw status gauges.
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/telnet.h>
#include "merc.h"

/* Mudlet auto-install (Client.GUI): tell the client to download+install our
 * package bundle. Mudlet only re-installs when GUI_VERSION is higher than what
 * it has stored for the profile, so BUMP this whenever the hosted package
 * changes. */
#define GMCP_GUI_VERSION "1"
#define GMCP_GUI_URL     "http://rustycatz.fortiddns.com/gmcp/StaticChaos.xml"

/* Offer GMCP to a freshly-connected client:  IAC WILL GMCP. */
void gmcp_offer( DESCRIPTOR_DATA *d )
{
    char buf[4];
    buf[0] = (char) IAC;
    buf[1] = (char) WILL;
    buf[2] = (char) TELOPT_GMCP;
    write_to_descriptor( d->descriptor, buf, 3 );
}

/* Send one GMCP message:  IAC SB GMCP "<msg> <json>" IAC SE.
 * No-op unless the client enabled GMCP, so callers needn't check. */
void send_gmcp( DESCRIPTOR_DATA *d, const char *msg, const char *json )
{
    char buf[MAX_STRING_LENGTH];
    char *p = buf;

    if ( d == NULL || !d->gmcp )
        return;

    *p++ = (char) IAC;
    *p++ = (char) SB;
    *p++ = (char) TELOPT_GMCP;
    p += sprintf( p, "%s", msg );
    if ( json != NULL && json[0] != '\0' )
    {
        *p++ = ' ';
        p += sprintf( p, "%s", json );
    }
    *p++ = (char) IAC;
    *p++ = (char) SE;

    write_to_descriptor( d->descriptor, buf, (int) ( p - buf ) );
}

/* Handle an incoming GMCP payload (NOT NUL-terminated; length is plen).
 * We send every package regardless of Core.Supports, so we only need to be
 * lenient here -- log Core.Hello once, ignore the rest. */
static void gmcp_in( DESCRIPTOR_DATA *d, const char *payload, int plen )
{
    char msg[MAX_INPUT_LENGTH];
    int i;

    for ( i = 0; i < plen && i < (int) sizeof( msg ) - 1
              && payload[i] != ' ' && payload[i] != '\0'; i++ )
        msg[i] = payload[i];
    msg[i] = '\0';

    if ( !str_cmp( msg, "Core.Hello" ) )
    {
        char line[MAX_INPUT_LENGTH];
        int n = UMIN( plen, (int) sizeof( line ) - 1 );
        memcpy( line, payload, n );
        line[n] = '\0';
        sprintf( log_buf, "GMCP %s: %s", d->host, line );
        log_string( log_buf );

        /* Mudlet: auto-download + install our package bundle from the web host.
         * Mudlet stores the version per profile and only re-installs on a bump. */
        send_gmcp( d, "Client.GUI",
            "{\"version\":\"" GMCP_GUI_VERSION "\",\"url\":\"" GMCP_GUI_URL "\"}" );
    }
    return;
}

/* Pull telnet / GMCP out of d->inbuf in place, leaving only clean text for
 * read_from_buffer.  An incomplete IAC sequence (split across reads) is left in
 * the buffer to be completed on a later pass. */
void gmcp_telnet_filter( DESCRIPTOR_DATA *d )
{
    unsigned char *in = (unsigned char *) d->inbuf;
    int len = strlen( d->inbuf );
    int i = 0, j = 0;

    while ( i < len )
    {
        unsigned char cmd;

        if ( in[i] != IAC )
        { d->inbuf[j++] = d->inbuf[i++]; continue; }

        if ( i + 1 >= len )                 /* lone IAC at end: wait for more */
            break;

        cmd = in[i+1];

        if ( cmd == IAC )                   /* escaped 0xFF -> one literal byte */
        { d->inbuf[j++] = (char) IAC; i += 2; continue; }

        if ( cmd == WILL || cmd == WONT || cmd == DO || cmd == DONT )
        {
            if ( i + 2 >= len )             /* incomplete */
                break;
            if ( in[i+2] == TELOPT_GMCP )
                d->gmcp = ( cmd == DO || cmd == WILL ) ? TRUE : FALSE;
            i += 3;
            continue;
        }

        if ( cmd == SB )                    /* subnegotiation: scan for IAC SE */
        {
            int k = i + 2, se = -1;
            while ( k + 1 < len )
            {
                if ( in[k] == IAC && in[k+1] == SE ) { se = k; break; }
                k++;
            }
            if ( se < 0 )                   /* SB without SE yet: wait */
                break;
            if ( in[i+2] == TELOPT_GMCP )
                gmcp_in( d, d->inbuf + i + 3, se - ( i + 3 ) );
            i = se + 2;
            continue;
        }

        i += 2;                             /* other 2-byte IAC command */
    }

    /* keep any unprocessed (incomplete) tail, right after the clean text */
    if ( i < len )
    {
        memmove( d->inbuf + j, d->inbuf + i, len - i );
        j += len - i;
    }
    d->inbuf[j] = '\0';
}

/* Phase 1: push one player's current vitals (for status gauges).  Includes the
 * class-specific resource (res/maxres/resname) so a client can draw a 4th gauge
 * -- Sorcerer Mystic, Saiyan Power, Fist Ki, Mazoku Essence.  Classes with no
 * extra pool (Patryn, None) send an empty resname so the gauge hides. */
void gmcp_update_char( CHAR_DATA *ch )
{
    char json[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int res = 0, maxres = 0;
    const char *resname = "";

    if ( ch == NULL || IS_NPC( ch ) )
        return;
    if ( ( d = ch->desc ) == NULL || !d->gmcp )
        return;

    switch ( ch->class )
    {
        case CLASS_SORCERER:
            resname = "Mystic"; res = ch->pcdata->powers[SORC_MYSTIC];
            maxres = ch->pcdata->will; break;
        case CLASS_SAIYAN:
            resname = "Power"; res = ch->pcdata->powers[S_POWER];
            maxres = ch->pcdata->powers[S_POWER_MAX]; break;
        case CLASS_FIST:
            resname = "Ki"; res = ch->pcdata->powers[F_KI];
            maxres = ch->pcdata->powers[F_KI_MAX]; break;
        case CLASS_MAZOKU:
            resname = "Essence"; res = ch->pcdata->powers[M_ESSENSE];
            maxres = UMAX( 10000, res ); break;  /* no hard cap; 10000 = regen ceiling */
        default: break;                          /* Patryn / None: no extra pool */
    }

    sprintf( json,
        "{\"hp\":%d,\"maxhp\":%d,\"mana\":%d,\"maxmana\":%d,\"move\":%d,\"maxmove\":%d,"
        "\"res\":%d,\"maxres\":%d,\"resname\":\"%s\"}",
        ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move,
        res, maxres, resname );
    send_gmcp( d, "Char.Vitals", json );
}

/* Copy src into dest, stripping colour codes and escaping JSON specials. */
static void gmcp_json_str( char *dest, const char *src )
{
    if ( src == NULL ) { dest[0] = '\0'; return; }
    while ( *src != '\0' )
    {
        if ( *src == '`' && src[1] != '\0' )      /* backtick colour code: `X */
        { src += 2; continue; }
        if ( *src == '{' )                        /* brace colour code: {..} */
        { while ( *src != '\0' && *src != '}' ) src++;
          if ( *src != '\0' ) src++;
          continue; }
        if ( *src == '"' || *src == '\\' )
        { *dest++ = '\\'; *dest++ = *src++; continue; }
        if ( (unsigned char) *src < 32 )
        { *dest++ = ' '; src++; continue; }
        *dest++ = *src++;
    }
    *dest = '\0';
}

/* Phase 2: push the player's current room + exits (for the auto-mapper). */
void gmcp_update_room( CHAR_DATA *ch )
{
    static const char *dname[6] = { "n", "e", "s", "w", "u", "d" };
    char json[MAX_STRING_LENGTH];
    char ename[MAX_INPUT_LENGTH], earea[MAX_INPUT_LENGTH], exits[MAX_INPUT_LENGTH];
    char *ep = exits;
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *room;
    EXIT_DATA *pexit;
    int door, first = 1;

    if ( ch == NULL || IS_NPC( ch ) )
        return;
    if ( ( d = ch->desc ) == NULL || !d->gmcp || d->connected != CON_PLAYING )
        return;
    if ( ( room = ch->in_room ) == NULL )
        return;

    *ep++ = '{';
    for ( door = 0; door < 6; door++ )
    {
        if ( ( pexit = room->exit[door] ) == NULL )
            continue;
        if ( !first )
            *ep++ = ',';
        first = 0;
        ep += sprintf( ep, "\"%s\":%d", dname[door],
            pexit->to_room != NULL ? pexit->to_room->vnum : pexit->vnum );
    }
    *ep++ = '}';
    *ep = '\0';

    gmcp_json_str( ename, room->name );
    gmcp_json_str( earea, room->area != NULL ? room->area->name : "" );

    sprintf( json, "{\"num\":%d,\"name\":\"%s\",\"area\":\"%s\",\"exits\":%s}",
        room->vnum, ename, earea, exits );
    send_gmcp( d, "Room.Info", json );
}

static char *gmcp_class_name( int cls )
{
    switch ( cls )
    {
        case CLASS_SAIYAN:   return "Saiyan";
        case CLASS_PATRYN:   return "Patryn";
        case CLASS_FIST:     return "Fist";
        case CLASS_SORCERER: return "Sorcerer";
        case CLASS_MAZOKU:   return "Mazoku";
        default:             return "None";
    }
}

/* Phase 3: character status (changes rarely, but cheap to resend). */
void gmcp_update_status( CHAR_DATA *ch )
{
    char json[MAX_STRING_LENGTH], nm[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( ch == NULL || IS_NPC( ch ) )
        return;
    if ( ( d = ch->desc ) == NULL || !d->gmcp || d->connected != CON_PLAYING )
        return;

    gmcp_json_str( nm, ch->name );
    sprintf( json,
        "{\"name\":\"%s\",\"class\":\"%s\",\"level\":%d,\"gold\":%d,\"primal\":%d,\"exp\":%lld}",
        nm, gmcp_class_name( ch->class ), ch->level, ch->gold, ch->pcdata->primal, ch->exp );
    send_gmcp( d, "Char.Status", json );
}

/* Phase 3: one-shot lists for client-side tab completion --
 * game.Commands (commands at the player's trust) and, for Sorcerers,
 * game.Chants (hyphenated chant names). */
void gmcp_send_lists( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH], *p;
    DESCRIPTOR_DATA *d;
    int i, trust, first;

    if ( ch == NULL || IS_NPC( ch ) )
        return;
    if ( ( d = ch->desc ) == NULL || !d->gmcp || d->connected != CON_PLAYING )
        return;

    trust = get_trust( ch );
    p = buf; *p++ = '[';
    first = 1;
    for ( i = 0; cmd_table[i].name[0] != '\0'; i++ )
    {
        if ( cmd_table[i].level > trust )
            continue;
        if ( !first ) *p++ = ',';
        first = 0;
        p += sprintf( p, "\"%s\"", cmd_table[i].name );
    }
    *p++ = ']'; *p = '\0';
    send_gmcp( d, "game.Commands", buf );

    if ( ch->class == CLASS_SORCERER )
    {
        p = buf; *p++ = '['; first = 1;
        for ( i = 0; i < MAX_CHANT; i++ )
        {
            const char *n = chant_table[i].name;
            if ( !first ) *p++ = ',';
            first = 0;
            *p++ = '"';
            while ( *n != '\0' ) { *p++ = ( *n == ' ' ) ? '-' : *n; n++; }
            *p++ = '"';
        }
        *p++ = ']'; *p = '\0';
        send_gmcp( d, "game.Chants", buf );
    }
}

/* Phase 1+3 heartbeat: once per second from second_update. */
void gmcp_update_all( void )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( !d->gmcp || d->connected != CON_PLAYING || d->character == NULL )
            continue;
        gmcp_update_char( d->character );        /* Char.Vitals */
        gmcp_update_status( d->character );      /* Char.Status */
        if ( !d->gmcp_lists )                    /* command/chant lists, once */
        {
            gmcp_send_lists( d->character );
            d->gmcp_lists = TRUE;
        }
    }
}
