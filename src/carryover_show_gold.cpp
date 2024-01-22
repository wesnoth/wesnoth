/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "carryover_show_gold.hpp"

#include "config.hpp"
#include "log.hpp"
#include "team.hpp"
#include "game_state.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/outro.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/retval.hpp"
#include "formula/string_utils.hpp"
#include "map/map.hpp"
#include <cassert>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)


void carryover_show_gold(game_state& state, bool hidden, bool is_observer, bool is_test)
{
	assert(state.end_level_data_);
	game_board& board = state.board_;
	const end_level_data& end_level = *state.end_level_data_;
	const bool is_victory = end_level.is_victory;
	// We need to write the carryover amount to the team that's why we need non const
	std::vector<team>& teams = board.teams();

	// maybe this can be the case for scenario that only contain a story and end during the prestart event ?
	if(teams.size() < 1) {
		return;
	}

	std::ostringstream report;
	std::string title;


	if(is_observer) {
		title = _("Scenario Report");
	} else if(is_victory) {
		title = _("Victory");
		report << "<b>" << _("You have emerged victorious!") << "</b>";
	} else {
		title = _("Defeat");
		report << _("You have been defeated!");
	}

	const std::string& next_scenario = state.get_game_data()->next_scenario();
	const bool has_next_scenario = !next_scenario.empty() && next_scenario != "null";

	int persistent_teams = 0;
	for(const team& t : teams) {
		if(t.persistent()) {
			++persistent_teams;
		}
	}

	if(persistent_teams > 0 && ((has_next_scenario && end_level.proceed_to_next_level) || is_test)) {
		const gamemap& map = board.map();
		const tod_manager& tod = state.get_tod_man();

		const int turns_left = std::max<int>(0, tod.number_of_turns() - tod.turn());
		for(team& t : teams) {
			if(!t.persistent() || t.lost()) {
				continue;
			}

			const int finishing_bonus_per_turn = map.villages().size() * t.village_gold() + t.base_income();
			const int finishing_bonus = t.carryover_bonus() * finishing_bonus_per_turn * turns_left;

			t.set_carryover_gold(div100rounded((t.gold() + finishing_bonus) * t.carryover_percentage()));

			if(!t.is_local_human()) {
				continue;
			}

			if(persistent_teams > 1) {
				report << "\n\n<b>" << t.side_name() << "</b>";
			}

			report << "<small>\n" << _("Remaining gold: ") << utils::half_signed_value(t.gold()) << "</small>";

			if(t.carryover_bonus() != 0) {
				if(turns_left > -1) {
					report << "\n\n<b>" << _("Turns finished early: ") << turns_left << "</b>\n"
						<< "<small>" << _("Early finish bonus: ") << finishing_bonus_per_turn << _(" per turn") << "</small>\n"
						<< "<small>" << _("Total bonus: ") << finishing_bonus << "</small>\n";
				}

				report << "<small>" << _("Total gold: ") << utils::half_signed_value(t.gold() + finishing_bonus) << "</small>";
			}

			if(t.gold() > 0) {
				report << "\n<small>" << _("Carryover percentage: ") << t.carryover_percentage() << "</small>";
			}

			if(t.carryover_add()) {
				report << "\n\n<big><b>" << _("Bonus gold: ") << utils::half_signed_value(t.carryover_gold()) << "</b></big>";
			} else {
				report << "\n\n<big><b>" << _("Retained gold: ") << utils::half_signed_value(t.carryover_gold()) << "</b></big>";
			}

			std::string goldmsg;
			utils::string_map symbols;

			symbols["gold"] = lexical_cast_default<std::string>(t.carryover_gold());

			// Note that both strings are the same in English, but some languages will
			// want to translate them differently.
			if(t.carryover_add()) {
				if(t.carryover_gold() > 0) {
					goldmsg = VNGETTEXT(
						"You will start the next scenario with $gold on top of the defined minimum starting gold.",
						"You will start the next scenario with $gold on top of the defined minimum starting gold.",
						t.carryover_gold(), symbols
					);

				} else {
					goldmsg = VNGETTEXT(
						"You will start the next scenario with the defined minimum starting gold.",
						"You will start the next scenario with the defined minimum starting gold.",
						t.carryover_gold(), symbols
					);
				}
			} else {
				goldmsg = VNGETTEXT(
					"You will start the next scenario with $gold or its defined minimum starting gold, "
					"whichever is higher.",
					"You will start the next scenario with $gold or its defined minimum starting gold, "
					"whichever is higher.",
					t.carryover_gold(), symbols
				);
			}

			// xgettext:no-c-format
			report << "\n" << goldmsg;
		}
	}

	if(end_level.transient.carryover_report && !hidden) {
		gui2::show_transient_message(title, report.str(), "", true);
	}
}
