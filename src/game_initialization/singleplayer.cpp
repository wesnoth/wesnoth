/*
   Copyright (C) 2008 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_initialization/singleplayer.hpp"
#include "config.hpp"
#include "config_assign.hpp"
#include "game_config_manager.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/sp_options_configure.hpp"
#include "gui/widgets/window.hpp"
#include "game_initialization/multiplayer.hpp"
#include "game_initialization/multiplayer_configure.hpp"
#include "game_initialization/multiplayer_connect.hpp"
#include "game_initialization/multiplayer_ui.hpp"
#include "game_initialization/playcampaign.hpp"
#include "wml_exception.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {

mp::chat gamechat;
config gamelist;

}

namespace sp {

bool enter_create_mode(CVideo& video, const config& game_config, saved_game& state, jump_to_campaign_info jump_to_campaign, bool local_players_only)
{
	bool configure_canceled = false;

	do {
		ng::create_engine create_eng(video, state);

		create_eng.set_current_level_type(ng::level::TYPE::SP_CAMPAIGN);

		const std::vector<ng::create_engine::level_ptr> campaigns =
			create_eng.get_levels_by_type_unfiltered(ng::level::TYPE::SP_CAMPAIGN);

		if(campaigns.empty()) {
			gui2::show_error_message(video, _("No campaigns are available."));
			return false;
		}

		std::string random_mode = "";

		// No campaign selected from command line
		if(jump_to_campaign.campaign_id_.empty()) {
			gui2::dialogs::campaign_selection dlg(create_eng);

			try {
				dlg.show(video);
			} catch(wml_exception& e) {
				e.show(video);
				return false;
			}

			if(dlg.get_retval() != gui2::window::OK) {
				return false;
			}

			if(dlg.get_deterministic()) {
				random_mode = "deterministic";
			}
		} else {
			// Don't reset the campaign_id_ so we can know
			// if we should quit the game or return to the main menu

			// Checking for valid campaign name
			const auto campaign = std::find_if(campaigns.begin(), campaigns.end(), [&jump_to_campaign](ng::create_engine::level_ptr level) {
				return level->data()["id"] == jump_to_campaign.campaign_id_;
			});

			// Didn't find a campaign with that id
			if(campaign == campaigns.end()) {
				ERR_NG << "No such campaign id to jump to: [" << jump_to_campaign.campaign_id_ << "]" << std::endl;
				return false;
			}

			create_eng.set_current_level(campaign - campaigns.begin());
		}

		state.classification().random_mode = random_mode;

		const std::string selected_difficulty = create_eng.select_campaign_difficulty(jump_to_campaign.difficulty_);

		if(selected_difficulty == "FAIL") return false;
		if(selected_difficulty == "CANCEL") {
			if(!jump_to_campaign.campaign_id_.empty()) {
				jump_to_campaign.campaign_id_ = "";
			}

			// Canceled difficulty dialog, relaunch the campaign selection dialog
			return enter_create_mode(video, game_config, state, jump_to_campaign, local_players_only);
		}

		create_eng.prepare_for_era_and_mods();
		create_eng.prepare_for_campaign(selected_difficulty);

		if(!jump_to_campaign.scenario_id_.empty()) {
			state.set_carryover_sides_start(
				config_of("next_scenario", jump_to_campaign.scenario_id_)
			);
		}

		if(!state.valid()) {
			ERR_NG << "Cannot load scenario with id=" << state.get_scenario_id() << std::endl;
			return false;
		}

		configure_canceled = !enter_configure_mode(video, game_config_manager::get()->game_config(), state, create_eng, local_players_only);

	} while (configure_canceled);

	return true;
}

bool enter_configure_mode(CVideo& video, const config& game_config, saved_game& state, ng::create_engine& create_eng, bool local_players_only)
{
	if(!gui2::dialogs::sp_options_configure::execute(create_eng, video)) {
		return false;
	}

	create_eng.get_parameters();
	create_eng.prepare_for_new_level();

	enter_connect_mode(video, game_config, state, local_players_only);

	return true;
}

bool enter_connect_mode(CVideo& video, const config& game_config, saved_game& state, bool local_players_only)
{
	ng::connect_engine connect_eng(state, true, nullptr);

	if(state.mp_settings().show_connect) {
		mp::ui::result res;
		gamelist.clear();
		{
			mp::connect ui(video, 0, state.mp_settings().name, game_config, gamechat, gamelist, connect_eng);
			mp::run_lobby_loop(video, ui);
			res = ui.get_result();

			if (res == mp::ui::PLAY) {
				ui.start_game();
			}
		}
		switch (res) {
		case mp::ui::PLAY:
			return true;
		case mp::ui::CREATE:
			enter_create_mode(video, game_config, state, jump_to_campaign_info(false, -1, "", ""), local_players_only);
			break;
		case mp::ui::QUIT:
		default:
			return false;
		}
		return true;
	} else {
		connect_eng.start_game();
		return true;
	}
}

} // end namespace sp
