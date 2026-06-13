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

/* Phase 1: push one player's current vitals (for status gauges). */
void gmcp_update_char( CHAR_DATA *ch )
{
    char json[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( ch == NULL || IS_NPC( ch ) )
        return;
    if ( ( d = ch->desc ) == NULL || !d->gmcp )
        return;

    sprintf( json,
        "{\"hp\":%d,\"maxhp\":%d,\"mana\":%d,\"maxmana\":%d,\"move\":%d,\"maxmove\":%d}",
        ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    send_gmcp( d, "Char.Vitals", json );
}

/* Phase 1 heartbeat: once per second from second_update. */
void gmcp_update_all( void )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->gmcp && d->connected == CON_PLAYING && d->character != NULL )
            gmcp_update_char( d->character );
}
