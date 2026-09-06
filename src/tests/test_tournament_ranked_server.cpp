/*
	Part of the Battle for Wesnoth Project
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>

#include "server/common/competitive_game_security.hpp"

BOOST_AUTO_TEST_SUITE(competitive_server)

BOOST_AUTO_TEST_CASE(sha256_returns_the_expected_digest)
{
	BOOST_CHECK_EQUAL(
		competitive_game_security::sha256("abc"),
		"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

BOOST_AUTO_TEST_CASE(hmac_sha256_returns_the_expected_digest)
{
	BOOST_CHECK_EQUAL(
		competitive_game_security::hmac_sha256("key", "The quick brown fox jumps over the lazy dog"),
		"f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
}

BOOST_AUTO_TEST_CASE(resume_identifiers_have_the_expected_shape)
{
	const std::string identifier = competitive_game_security::generate_identifier();
	const std::string token = competitive_game_security::generate_resume_token();

	BOOST_CHECK_EQUAL(identifier.size(), 36);
	BOOST_CHECK_EQUAL(token.size(), 64);
	BOOST_CHECK_EQUAL(identifier[8], '-');
	BOOST_CHECK_EQUAL(identifier[13], '-');
	BOOST_CHECK_EQUAL(identifier[18], '-');
	BOOST_CHECK_EQUAL(identifier[23], '-');
	BOOST_CHECK_NE(identifier, competitive_game_security::generate_identifier());
	BOOST_CHECK_NE(token, competitive_game_security::generate_resume_token());
}

BOOST_AUTO_TEST_SUITE_END()
