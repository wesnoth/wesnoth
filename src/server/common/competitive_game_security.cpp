/*
	Part of the Battle for Wesnoth Project
*/

#include "server/common/competitive_game_security.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

#include <array>
#include <iomanip>
#include <sstream>
#include <vector>

namespace
{
	// Encode binary cryptographic output without exposing the raw bytes in WML
	// or database fields.
	std::string hex_encode(const unsigned char* data, const std::size_t size)
	{
		std::ostringstream result;
		result << std::hex << std::setfill('0');
		for(std::size_t index = 0; index < size; ++index) {
			result << std::setw(2) << static_cast<unsigned int>(data[index]);
		}
		return result.str();
	}

	std::string random_hex(const std::size_t byte_count)
	{
		std::vector<unsigned char> bytes(byte_count);
		// OpenSSL's CSPRNG is required here; a fallback pseudo-random generator
		// would make identifiers and resume credentials predictable.
		if(RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1) {
			return {};
		}
		return hex_encode(bytes.data(), bytes.size());
	}
}

namespace competitive_game_security
{
	std::string generate_identifier()
	{
		// Keep the familiar UUID representation while using cryptographically
		// random bytes; this identifier is not used as an authentication secret.
		const std::string raw = random_hex(16);
		if(raw.size() != 32) {
			return {};
		}
		return raw.substr(0, 8) + "-" + raw.substr(8, 4) + "-" + raw.substr(12, 4) + "-"
			+ raw.substr(16, 4) + "-" + raw.substr(20);
	}

	std::string generate_resume_token()
	{
		// The token is returned to the authorized client but only its digest is
		// persisted, limiting the impact of a database disclosure.
		return random_hex(32);
	}

	std::string sha256(const std::string& value)
	{
		// SHA-256 is used for one-way storage of resume tokens and database UUIDs.
		std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
		unsigned int digest_size = 0;
		EVP_MD_CTX* context = EVP_MD_CTX_new();
		if(!context || EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1
			|| EVP_DigestUpdate(context, value.data(), value.size()) != 1
			|| EVP_DigestFinal_ex(context, digest.data(), &digest_size) != 1) {
			EVP_MD_CTX_free(context);
			return {};
		}
		EVP_MD_CTX_free(context);
		return hex_encode(digest.data(), digest_size);
	}

	std::string hmac_sha256(const std::string& secret, const std::string& value)
	{
		// HMAC binds the resume signature to a server-side secret as well as the
		// value, preventing a client from minting a valid signature by itself.
		std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
		unsigned int digest_size = 0;
		if(!HMAC(EVP_sha256(), secret.data(), static_cast<int>(secret.size()),
			reinterpret_cast<const unsigned char*>(value.data()), value.size(), digest.data(), &digest_size)) {
			return {};
		}
		return hex_encode(digest.data(), digest_size);
	}
}
