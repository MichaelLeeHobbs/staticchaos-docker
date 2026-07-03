/*
 * password.c -- password hashing with transparent on-login migration.
 *
 * Static Chaos historically stored login passwords in plaintext (crypt() was
 * macro'd to a no-op in merc.h and libcrypt was never linked). This module
 * replaces that with real yescrypt ($y$) hashing via libxcrypt's crypt(3) /
 * crypt_gensalt(3), linked 32-bit through -lcrypt.
 *
 * Migration is transparent: legacy plaintext pfiles still load, and the first
 * successful login re-hashes the password in place (see comm.c
 * CON_GET_OLD_PASSWORD). No save-format change -- the "Password ...~" field now
 * simply holds a "$y$..." string. yescrypt hashes never contain '~', so they
 * remain safe for the tilde-terminated pfile field.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crypt.h>
#include "merc.h"

/*
 * Hash a plaintext password with a fresh random yescrypt salt.
 * Returns a str_dup'd "$y$..." string the caller owns, or NULL on failure.
 * On failure we bug() and return NULL -- callers MUST NOT fall back to storing
 * the plaintext.
 */
char *password_hash( const char *plain )
{
    char *salt;
    char *hash;

    salt = crypt_gensalt( "$y$", 0, NULL, 0 );
    if ( salt == NULL )
    {
	bug( "password_hash: crypt_gensalt() returned NULL.", 0 );
	return NULL;
    }

    hash = crypt( plain, salt );
    if ( hash == NULL || hash[0] != '$' )
    {
	bug( "password_hash: crypt() failed to produce a hash.", 0 );
	return NULL;
    }

    return str_dup( hash );
}

/*
 * Verify a plaintext password against a stored value.
 *
 * If `stored` looks like a crypt hash (leading '$'), verify with crypt() using
 * the stored hash as the salt and set *needs_upgrade = FALSE. Otherwise treat
 * `stored` as a legacy plaintext password: compare directly and set
 * *needs_upgrade = TRUE so the caller can re-hash on a successful login.
 *
 * Returns TRUE on a match. Comparison is case-sensitive either way.
 */
bool password_verify( const char *plain, const char *stored, bool *needs_upgrade )
{
    if ( stored != NULL && stored[0] == '$' )
    {
	char *hash;

	*needs_upgrade = FALSE;
	hash = crypt( plain, stored );
	return hash != NULL && strcmp( hash, stored ) == 0;
    }

    *needs_upgrade = TRUE;
    return strcmp( plain, stored == NULL ? "" : stored ) == 0;
}
