#ifndef BCRYPT_H_
#define BCRYPT_H_
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

#define BCRYPT_HASHSIZE	(64)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This function expects a work factor between 4 and 31 and a char array to
 * store the resulting generated salt. The char array should typically have
 * BCRYPT_HASHSIZE bytes at least. If the provided work factor is not in the
 * previous range, it will default to 12.
 *
 * The return value is zero if the salt could be correctly generated and
 * nonzero otherwise.
 *
 */
int bcrypt_gensalt(int workfactor, char salt[BCRYPT_HASHSIZE]);

/*
 * This function expects a password to be hashed, a salt to hash the password
 * with and a char array to leave the result. Both the salt and the hash
 * parameters should have room for BCRYPT_HASHSIZE characters at least.
 *
 * It can also be used to verify a hashed password. In that case, provide the
 * expected hash in the salt parameter and verify the output hash is the same
 * as the input hash. However, to avoid timing attacks, it's better to use
 * bcrypt_checkpw when verifying a password.
 *
 * The return value is zero if the password could be hashed and nonzero
 * otherwise.
 */
int bcrypt_hashpw(const char *passwd, const char salt[BCRYPT_HASHSIZE],
		  char hash[BCRYPT_HASHSIZE]);

/*
 * This function expects a password and a hash to verify the password against.
 * The internal implementation is tuned to avoid timing attacks.
 *
 * The return value will be -1 in case of errors, zero if the provided password
 * matches the given hash and greater than zero if no errors are found and the
 * passwords don't match.
 *
 */
int bcrypt_checkpw(const char *passwd, const char hash[BCRYPT_HASHSIZE]);

/*
 * Brief Example
 * -------------
 *
 * Hashing a password:
 *
 *	char salt[BCRYPT_HASHSIZE];
 *	char hash[BCRYPT_HASHSIZE];
 *	int ret;
 *
 *	ret = bcrypt_gensalt(12, salt);
 *	assert(ret == 0);
 *	ret = bcrypt_hashpw("thepassword", salt, hash);
 *	assert(ret == 0);
 *
 *
 * Verifying a password:
 *
 *	int ret;
 *
 *      ret = bcrypt_checkpw("thepassword", "expectedhash");
 *      assert(ret != -1);
 *
 *	if (ret == 0) {
 *		printf("The password matches\n");
 *	} else {
 *		printf("The password does NOT match\n");
 *	}
 *
 */

#ifdef __cplusplus
}
#endif

#endif
