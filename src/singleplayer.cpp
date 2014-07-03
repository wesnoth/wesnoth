#include "singleplayer.hpp"
#include "game_config_manager.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include "resources.hpp"

bool sp_create_mode(game_display& disp, const config& game_config,
	saved_game& state, jump_to_campaign_info jump_to_campaign, bool local_players_only) {

	bool configure_canceled = false;

	do {

		mp::create_engine create_eng(disp, state);
		create_eng.set_current_level_type(mp::level::SP_CAMPAIGN);

		std::vector<mp::create_engine::level_ptr> campaigns(
			create_eng.get_levels_by_type_unfiltered(mp::level::SP_CAMPAIGN));
		
		if (campaigns.empty()) {
		  gui2::show_error_message(disp.video(),
					  _("No campaigns are available.\n"));
			return false;
		}

		int campaign_num = -1;
		bool use_deterministic_mode = false;
		// No campaign selected from command line
		if (jump_to_campaign.campaign_id_.empty() == true)
		{
			gui2::tcampaign_selection dlg(campaigns);

			try {
				dlg.show(disp.video());
			} catch(twml_exception& e) {
				e.show(disp);
				return false;
			}

			if(dlg.get_retval() != gui2::twindow::OK) {
				return false;
			}

			campaign_num = dlg.get_choice();

			use_deterministic_mode = dlg.get_deterministic();

		}
		else
		{
			// don't reset the campaign_id_ so we can know
			// if we should quit the game or return to the main menu

			// checking for valid campaign name
			for(size_t i = 0; i < campaigns.size(); ++i)
			{
				if (campaigns[i]->data()["id"] == jump_to_campaign.campaign_id_)
				{
					campaign_num = i;
					break;
				}
			}

			// didn't found any campaign with that id
			if (campaign_num == -1)
			{
				std::cerr<<"No such campaign id to jump to: ["<<jump_to_campaign.campaign_id_<<"]\n";
				return false;
			}
		}

		create_eng.set_current_level(campaign_num);

		std::string random_mode = use_deterministic_mode ? "deterministic" : "";
		state.carryover_sides_start["random_mode"] = random_mode;
		state.classification().random_mode = random_mode;

		std::string selected_difficulty = create_eng.select_campaign_difficulty(jump_to_campaign.difficulty_);

		if (selected_difficulty == "FAIL") return false;
		if (selected_difficulty == "CANCEL") {
			if (jump_to_campaign.campaign_id_.empty() == false)
			{
				jump_to_campaign.campaign_id_ = "";
			}
			// canceled difficulty dialog, relaunch the campaign selection dialog
			return sp_create_mode(disp, game_config, state, jump_to_campaign, local_players_only);
		}

		create_eng.prepare_for_campaign(selected_difficulty);

		if (jump_to_campaign.scenario_id_.empty())
			state.carryover_sides_start["next_scenario"] = create_eng.current_level().data()["id"].str();
		else {
			state.carryover_sides_start["next_scenario"] = jump_to_campaign.scenario_id_;
			create_eng.current_level().set_data(
				resources::config_manager->game_config().find_child(
				lexical_cast<std::string> (game_classification::MULTIPLAYER),
				"id", jump_to_campaign.scenario_id_));
		}

		create_eng.prepare_for_new_level();

		state.mp_settings().mp_era = "era_blank";

		configure_canceled = !sp_configure_mode(disp, resources::config_manager->game_config(), state, local_players_only);

	} while (configure_canceled);

	return true;
}

bool sp_configure_mode(game_display& disp, const config& game_config,
	saved_game& state, bool local_players_only) {

	mp::configure_engine engine(state);
	engine.set_default_values();

	return sp_connect_mode(disp, game_config, state, local_players_only);
}

bool sp_connect_mode(game_display&, const config&,
	saved_game& state, bool local_players_only) {

	mp::connect_engine engine(state, local_players_only, true);
	engine.start_game();
	return true;
}
