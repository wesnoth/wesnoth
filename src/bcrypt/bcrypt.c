/*
 * bcrypt wrapper library
 *
 * Written in 2011, 2013, 2014, 2015 by Ricardo Garcia <r@rg3.name>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

/*
 * 21FEB2018 - Gregory A Lundberg
 * 
 * Deleted all unneeded code. This converts this source file from POSIX-only
 * to fully portable, strictly compliant ISO Standard C.
 */

#include <stddef.h>
#include <string.h>

#include "bcrypt.h"
#include "crypt_blowfish/ow-crypt.h"

/*
 * This is a best effort implementation. Nothing prevents a compiler from
 * optimizing this function and making it vulnerable to timing attacks, but
 * this method is commonly used in crypto libraries like NaCl.
 *
 * Return value is zero if both strings are equal and nonzero otherwise.
*/
static int timing_safe_strcmp(const char *str1, const char *str2)
{
	const unsigned char *u1;
	const unsigned char *u2;
	int ret;
	int i;

	int len1 = strlen(str1);
	int len2 = strlen(str2);

	/* In our context both strings should always have the same length
	 * because they will be hashed passwords. */
	if (len1 != len2)
		return 1;

	/* Force unsigned for bitwise operations. */
	u1 = (const unsigned char *)str1;
	u2 = (const unsigned char *)str2;

	ret = 0;
	for (i = 0; i < len1; ++i)
		ret |= (u1[i] ^ u2[i]);

	return ret;
}

int bcrypt_hashpw(const char *passwd, const char salt[BCRYPT_HASHSIZE], char hash[BCRYPT_HASHSIZE])
{
	char *aux;
	aux = crypt_rn(passwd, salt, hash, BCRYPT_HASHSIZE);
	return (aux == NULL)?1:0;
}

int bcrypt_checkpw(const char *passwd, const char hash[BCRYPT_HASHSIZE])
{
	int ret;
	char outhash[BCRYPT_HASHSIZE];

	ret = bcrypt_hashpw(passwd, hash, outhash);
	if (ret != 0)
		return -1;

	return timing_safe_strcmp(hash, outhash);
}
