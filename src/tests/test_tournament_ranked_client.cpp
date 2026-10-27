/*
	Part of the Battle for Wesnoth Project
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>

#include "gui/dialogs/multiplayer/mp_tournament_ranked.hpp"
#include "mp_game_settings.hpp"

namespace
{
	void set_competitive_metadata(mp_game_settings& settings)
	{
		settings.competitive.ranked_mode = true;
		settings.competitive.ranked_game_id = "ranked-game-id";
		settings.competitive.ranked_resume_token = "resume-token";
		settings.competitive.ranked_resume_signature = "resume-signature";
		settings.competitive.tournament_id = "tournament-id";
		settings.competitive.tournament_name = "Tournament name";
		settings.competitive.tournament_game_id = "game-id";
		settings.competitive.tournament_phase_name = "Phase name";
		settings.competitive.tournament_group_name = "Group name";
		settings.competitive.tournament_round_number = "2";
		settings.competitive.tournament_game_number = "4";
	}
}

BOOST_AUTO_TEST_SUITE(competitive_client)

BOOST_AUTO_TEST_CASE(default_settings_are_not_competitive)
{
	const mp_game_settings settings;

	BOOST_CHECK(!settings.is_competitive());
}

BOOST_AUTO_TEST_CASE(ranked_and_tournament_settings_are_competitive)
{
	mp_game_settings ranked;
	ranked.competitive.ranked_mode = true;
	BOOST_CHECK(ranked.is_competitive());

	mp_game_settings tournament;
	tournament.competitive.tournament_game_id = "game-id";
	BOOST_CHECK(tournament.is_competitive());
}

BOOST_AUTO_TEST_CASE(competitive_metadata_survives_config_round_trip)
{
	mp_game_settings original;
	set_competitive_metadata(original);

	const mp_game_settings restored(original.to_config());

	BOOST_CHECK(restored.competitive.ranked_mode);
	BOOST_CHECK_EQUAL(restored.competitive.ranked_game_id, original.competitive.ranked_game_id);
	BOOST_CHECK_EQUAL(restored.competitive.ranked_resume_token, original.competitive.ranked_resume_token);
	BOOST_CHECK_EQUAL(restored.competitive.ranked_resume_signature, original.competitive.ranked_resume_signature);
	BOOST_CHECK_EQUAL(restored.competitive.tournament_id, original.competitive.tournament_id);
	BOOST_CHECK_EQUAL(restored.competitive.tournament_name, original.competitive.tournament_name);
	BOOST_CHECK_EQUAL(restored.competitive.tournament_game_id, original.competitive.tournament_game_id);
	BOOST_CHECK_EQUAL(restored.competitive.tournament_phase_name, original.competitive.tournament_phase_name);
	BOOST_CHECK_EQUAL(restored.competitive.tournament_group_name, original.competitive.tournament_group_name);
	BOOST_CHECK_EQUAL(restored.competitive.tournament_round_number, original.competitive.tournament_round_number);
	BOOST_CHECK_EQUAL(restored.competitive.tournament_game_number, original.competitive.tournament_game_number);
}

BOOST_AUTO_TEST_CASE(tournament_match_label_uses_exact_metadata)
{
	BOOST_CHECK_EQUAL(
		gui2::dialogs::format_tournament_match_label("Cup", "Final", "Group A", "2", "4"),
		"Cup — Final — Group A — Round 2 — Game 4");
	BOOST_CHECK_EQUAL(gui2::dialogs::format_tournament_match_label("Cup", "", "", "", ""), "Cup");
}

BOOST_AUTO_TEST_CASE(competitive_message_ids_are_localized_on_the_client)
{
	BOOST_CHECK(!gui2::dialogs::localized_competitive_message("ranked_access_required").empty());
	BOOST_CHECK(gui2::dialogs::localized_competitive_message("unknown_message").empty());
}

BOOST_AUTO_TEST_SUITE_END()
