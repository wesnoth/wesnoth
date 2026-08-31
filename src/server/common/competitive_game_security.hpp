/*
	Part of the Battle for Wesnoth Project
*/

#pragma once

#include <string>

namespace competitive_game_security
{
	/**
	 * Competitive save authentication flow:
	 *
	 * 1. The server generates a public match identifier and a private resume
	 *    token using cryptographically secure random bytes.
	 * 2. Only the SHA-256 digest of the resume token is stored in the database;
	 *    the plaintext token is kept in the save metadata and sent to clients.
	 * 3. The server creates an HMAC signature over "match_id:resume_token" using
	 *    the server-only competitive resume secret. The signature is also sent
	 *    with the save metadata.
	 * 4. When a save is loaded, the server hashes the supplied token and checks
	 *    the database record, verifies the HMAC signature, and confirms that the
	 *    tournament metadata and lifecycle status still match. Player and team
	 *    eligibility are validated separately by the server before continuation.
	 *
	 * The match identifier is therefore a lookup key, not an authentication
	 * secret. A database copy alone cannot be used to resume a game because the
	 * plaintext token and the server-only HMAC secret are required.
	 */
	/** Generate a UUID-shaped identifier that is safe to expose to clients. */
	std::string generate_identifier();
	/** Generate a private random token used to authorize a competitive resume. */
	std::string generate_resume_token();

	/** Return the lowercase hexadecimal SHA-256 digest of a value. */
	std::string sha256(const std::string& value);
	/** Return the lowercase hexadecimal HMAC-SHA-256 digest of a value. */
	std::string hmac_sha256(const std::string& secret, const std::string& value);
}
