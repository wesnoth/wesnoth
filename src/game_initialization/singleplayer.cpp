/*
	Copyright (C) 2008 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "game_initialization/singleplayer.hpp"

#include "addon/manager_ui.hpp"
#include "config.hpp"
#include "game_initialization/configure_engine.hpp"
#include "game_initialization/connect_engine.hpp"
#include "game_initialization/create_engine.hpp"
#include "game_launcher.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/sp_options_configure.hpp"
#include "gui/widgets/retval.hpp"
#include "wml_exception.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

namespace sp
{
bool select_campaign(saved_game& state, jump_to_campaign_info jump_to_campaign)
{
	while(true) {
		ng::create_engine create_eng(state);
		create_eng.set_current_level_type(level_type::type::sp_campaign);

		const std::vector<ng::create_engine::level_ptr> campaigns =
			create_eng.get_levels_by_type_unfiltered(level_type::type::sp_campaign);

		if(campaigns.empty()) {
			gui2::show_error_message(_("No campaigns are available."));
			return false;
		}

		std::string random_mode = "", difficulty = "";

		// No campaign selected from command line
		if(jump_to_campaign.campaign_id.empty()) {
			gui2::dialogs::campaign_selection dlg(create_eng);

			try {
				dlg.show();
			} catch(const wml_exception& e) {
				e.show();
				return false;
			}

			if(dlg.get_retval() == gui2::retval::OK) {
				switch(dlg.get_rng_mode()) {
					case gui2::dialogs::campaign_selection::RNG_DEFAULT:
						random_mode = "";
						break;
					case gui2::dialogs::campaign_selection::RNG_SAVE_SEED:
						random_mode = "deterministic";
						break;
					case gui2::dialogs::campaign_selection::RNG_BIASED:
						random_mode = "biased";
						break;
				}
				difficulty = dlg.get_difficulty();
			} else {
				if (dlg.get_retval() == gui2::dialogs::campaign_selection::OPEN_ADDON_MANAGER) {
					manage_addons();
				}
				return false;
			}

		} else {
			// Don't reset the campaign_id_ so we can know
			// if we should quit the game or return to the main menu

			// Checking for valid campaign name
			const auto campaign = std::find_if(campaigns.begin(), campaigns.end(), [&jump_to_campaign](const ng::create_engine::level_ptr& level) {
				return level->data()["id"] == jump_to_campaign.campaign_id;
			});

			// Didn't find a campaign with that id
			if(campaign == campaigns.end()) {
				ERR_NG << "No such campaign id to jump to: [" << jump_to_campaign.campaign_id << "]";
				return false;
			}

			create_eng.set_current_level(std::distance(campaigns.begin(), campaign));
		}

		state.classification().random_mode = random_mode;

		const auto selected_difficulty = difficulty;

		if(selected_difficulty == "FAIL") return false;
		if(selected_difficulty == "CANCEL") {
			if(!jump_to_campaign.campaign_id.empty()) {
				jump_to_campaign.campaign_id = "";
			}

			// Canceled difficulty dialog, relaunch the campaign selection dialog
			return select_campaign(state, jump_to_campaign);
		}

		create_eng.prepare_for_era_and_mods();
		create_eng.prepare_for_campaign(selected_difficulty);

		if(!jump_to_campaign.scenario_id.empty()) {
			state.set_carryover_sides_start(
				config {"next_scenario", jump_to_campaign.scenario_id}
			);
		}

		if(!state.valid()) {
			ERR_NG << "Cannot load scenario with id=" << state.get_scenario_id();
			return false;
		}

		if(configure_campaign(state, create_eng)) {
			break;
		}
	}

	return true;
}

bool configure_campaign(saved_game& state, ng::create_engine& create_eng)
{
	// We create the config engine here in order to ensure values like use_map_settings are set correctly
	// TODO: should this be passed to this function instead of created here?
	ng::configure_engine config_eng(create_eng.get_state());

	// TODO: needed?
	config_eng.update_initial_cfg(create_eng.current_level().data());

	if(!gui2::dialogs::sp_options_configure::execute(create_eng, config_eng)) {
		return false;
	}

	create_eng.get_parameters();
	create_eng.prepare_for_new_level();

	ng::connect_engine connect_eng(state, true, nullptr);
	connect_eng.start_game();

	return true;
}

} // end namespace sp
