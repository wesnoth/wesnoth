#include "singleplayer.hpp"
#include "config.hpp"
#include "config_assign.hpp"
#include "game_config_manager.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include "multiplayer.hpp"
#include "multiplayer_configure.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_ui.hpp"
#include "playcampaign.hpp"
#include "resources.hpp"
#include "wml_exception.hpp"

namespace {

mp::chat gamechat;
config gamelist;

}

namespace sp {

bool enter_create_mode(game_display& disp, const config& game_config,
	saved_game& state, jump_to_campaign_info jump_to_campaign, bool local_players_only) {

	bool configure_canceled = false;

	do {

		ng::create_engine create_eng(disp, state);
		create_eng.set_current_level_type(ng::level::TYPE::SP_CAMPAIGN);

		std::vector<ng::create_engine::level_ptr> campaigns(
			create_eng.get_levels_by_type_unfiltered(ng::level::TYPE::SP_CAMPAIGN));

		if (campaigns.empty()) {
		  gui2::show_error_message(disp.video(),
					  _("No campaigns are available.\n"));
			return false;
		}

		bool use_deterministic_mode = false;
		// No campaign selected from command line
		if (jump_to_campaign.campaign_id_.empty() == true)
		{
			gui2::tcampaign_selection dlg(create_eng);

			try {
				dlg.show(disp.video());
			} catch(twml_exception& e) {
				e.show(disp.video());
				return false;
			}

			if(dlg.get_retval() != gui2::twindow::OK) {
				return false;
			}

			use_deterministic_mode = dlg.get_deterministic();

		}
		else
		{
			// don't reset the campaign_id_ so we can know
			// if we should quit the game or return to the main menu

			// checking for valid campaign name
			bool not_found = true;
			for(size_t i = 0; i < campaigns.size(); ++i)
			{
				if (campaigns[i]->data()["id"] == jump_to_campaign.campaign_id_)
				{
					create_eng.set_current_level(i);
					not_found = false;
					break;
				}

			}

			// didn't find any campaign with that id
			if (not_found)
			{
				//TODO: use ERR_NG or similar
				std::cerr<<"No such campaign id to jump to: ["<<jump_to_campaign.campaign_id_<<"]\n";
				return false;
			}

		}

		std::string random_mode = use_deterministic_mode ? "deterministic" : "";
		state.classification().random_mode = random_mode;

		std::string selected_difficulty = create_eng.select_campaign_difficulty(jump_to_campaign.difficulty_);

		if (selected_difficulty == "FAIL") return false;
		if (selected_difficulty == "CANCEL") {
			if (jump_to_campaign.campaign_id_.empty() == false)
			{
				jump_to_campaign.campaign_id_ = "";
			}
			// canceled difficulty dialog, relaunch the campaign selection dialog
			return enter_create_mode(disp, game_config, state, jump_to_campaign, local_players_only);
		}

		create_eng.prepare_for_era_and_mods();
		create_eng.prepare_for_campaign(selected_difficulty);

		if(!jump_to_campaign.scenario_id_.empty()) {
			state.set_carryover_sides_start(
				config_of("next_scenario", jump_to_campaign.scenario_id_)
			);
		}
		create_eng.prepare_for_new_level();

		create_eng.get_parameters();
		if(!state.valid())
		{
			//TODO: use ERR_NG or similar
			std::cerr << "Cannot load scenario with id=" << state.get_scenario_id() << "\n";
			return false;
		}
		configure_canceled = !enter_configure_mode(disp, game_config_manager::get()->game_config(), state, local_players_only);

	} while (configure_canceled);

	return true;
}

bool enter_configure_mode(game_display& disp, const config& game_config, saved_game& state, bool local_players_only) {
	bool connect_canceled;
	do {
		connect_canceled = false;

		mp::ui::result res;

		{
			mp::configure ui(disp, game_config, gamechat, gamelist, state, local_players_only);
			mp::run_lobby_loop(disp, ui);
			res = ui.get_result();
			ui.get_parameters();
		}

		switch (res) {
		case mp::ui::CREATE:
			connect_canceled = !enter_connect_mode(disp, game_config, state, local_players_only);
			break;
		case mp::ui::QUIT:
		default:
			return false;
		}
	} while (connect_canceled);
	return true;
}

bool enter_connect_mode(game_display& disp, const config& game_config,
	saved_game& state, bool local_players_only) {

	ng::connect_engine connect_eng(state, true, NULL);

	if (state.mp_settings().show_connect) {
		mp::ui::result res;
		gamelist.clear();
		{
			mp::connect ui(disp, state.mp_settings().name, game_config, gamechat, gamelist, connect_eng);
			mp::run_lobby_loop(disp, ui);
			res = ui.get_result();

			if (res == mp::ui::PLAY) {
				ui.start_game();
			}
		}
		switch (res) {
		case mp::ui::PLAY:
			return true;
		case mp::ui::CREATE:
			enter_create_mode(disp, game_config, state, jump_to_campaign_info(false, -1, "", ""), local_players_only);
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
