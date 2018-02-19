/*
   Copyright (C) 2013 - 2018 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "game_initialization/connect_engine.hpp"

#include "ai/configuration.hpp"
#include "formula/string_utils.hpp"
#include "game_initialization/mp_game_utils.hpp"
#include "game_initialization/playcampaign.hpp"
#include "preferences/credentials.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "mt_rng.hpp"
#include "tod_manager.hpp"
#include "wesnothd_connection.hpp"

#include <cstdlib>
#include <ctime>

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_connect_engine("mp/connect/engine");
#define DBG_MP LOG_STREAM(debug, log_mp_connect_engine)
#define LOG_MP LOG_STREAM(info, log_mp_connect_engine)
#define WRN_MP LOG_STREAM(warn, log_mp_connect_engine)
#define ERR_MP LOG_STREAM(err, log_mp_connect_engine)

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)

static const std::string controller_names[] {
	"human",
	"human",
	"ai",
	"null",
	"reserved"
};

static const std::string attributes_to_trim[] {
	"side",
	"type",
	"gender",
	"recruit",
	"player_id",
	"previous_recruits",
	"controller",
	"current_player",
	"team_name",
	"user_team_name",
	"color",
	"gold",
	"income",
	"allow_changes",
	"faction"
};

namespace ng {

connect_engine::connect_engine(saved_game& state, const bool first_scenario, mp_campaign_info* campaign_info)
	: level_()
	, state_(state)
	, params_(state.mp_settings())
	, default_controller_(campaign_info ? CNTR_NETWORK : CNTR_LOCAL)
	, campaign_info_(campaign_info)
	, first_scenario_(first_scenario)
	, force_lock_settings_()
	, side_engines_()
	, era_factions_()
	, team_data_()
{
	// Initial level config from the mp_game_settings.
	level_ = mp::initial_level_config(state_);
	if(level_.empty()) {
		return;
	}

	const bool is_mp = state_.classification().is_normal_mp_game();
	force_lock_settings_ = (!state.mp_settings().saved_game) && scenario()["force_lock_settings"].to_bool(!is_mp);

	// Original level sides.
	config::child_itors sides = current_config()->child_range("side");

	// AI algorithms.
	ai::configuration::add_era_ai_from_config(level_.child("era"));
	ai::configuration::add_mod_ai_from_config(level_.child_range("modification"));

	// Set the team name lists and modify the original level sides if necessary.
	std::vector<std::string> original_team_names;
	std::string team_prefix(_("Team") + " ");

	int side_count = 1;
	for(config& side : sides) {
		const std::string side_str = std::to_string(side_count);

		config::attribute_value& team_name = side["team_name"];
		config::attribute_value& user_team_name = side["user_team_name"];

		// Revert to default values if appropriate.
		if(team_name.empty()) {
			team_name = side_str;
		}

		if(params_.use_map_settings && user_team_name.empty()) {
			user_team_name = team_name;
		}

		bool add_team = true;
		if(params_.use_map_settings) {
			// Only add a team if it is not found.
			if(std::any_of(team_data_.begin(), team_data_.end(), [&team_name](const team_data_pod& data){
				return data.team_name == team_name.str();
			})) {
				add_team = false;
			}
		} else {
			// Always add a new team for every side, but leave the specified team assigned to a side if there is one.
			auto name_itor = std::find(original_team_names.begin(), original_team_names.end(), team_name.str());

			// Note that the prefix "Team " is untranslatable, as team_name is not meant to be translated. This is needed
			// so that the attribute is not interpretted as an int when reading from config, which causes bugs later.
			if(name_itor == original_team_names.end()) {
				original_team_names.push_back(team_name);

				team_name = "Team " + std::to_string(original_team_names.size());
			} else {
				team_name = "Team " + std::to_string(std::distance(original_team_names.begin(), name_itor) + 1);
			}

			user_team_name = team_prefix + side_str;
		}

		// Write the serialized translatable team name back to the config. Without this,
		// the string can appear all messed up after leaving and rejoining a game (see
		// issue #2040. This affected the mp_join_game dialog). I don't know why that issue
		// didn't appear the first time you join a game, but whatever.
		//
		// The difference between that dialog and mp_staging is that the latter has access
		// to connect_engine object, meaning it has access to serialized versions of the
		// user_team_name string stored in the team_data_ vector. mp_join_game handled the
		// raw side config instead. Again, I don't know why issues only cropped up on a
		// subsequent join and not the first, but it doesn't really matter.
		//
		// This ensures both dialogs have access to the serialized form of the utn string.
		// As for why this needs to be done in the first place, apparently the simple_wml
		// format the server (wesnothd) uses doesn't preserve translatable strings (see
		// issue #342).
		//
		// --vultraz, 2018-02-06
		user_team_name = user_team_name.t_str().to_serialized();

		if(add_team) {
			team_data_pod data;
			data.team_name = params_.use_map_settings ? team_name : "Team " + side_str;
			data.user_team_name = user_team_name.str();
			data.is_player_team = side["allow_player"].to_bool(true);

			team_data_.push_back(data);
		}

		++side_count;
	}

	// Selected era's factions.
	for(const config& era : level_.child("era").child_range("multiplayer_side")) {
		era_factions_.push_back(&era);
	}

	game_config::add_color_info(scenario());

	// Create side engines.
	int index = 0;
	for(const config& s : sides) {
		side_engines_.emplace_back(new side_engine(s, *this, index));

		index++;
	}

	if(first_scenario_) {
		// Add host to the connected users list.
		import_user(preferences::login(), false);
	} else {
		// Add host but don't assign a side to him.
		import_user(preferences::login(), true);

		// Load reserved players information into the sides.
		load_previous_sides_users();
	}

	// Only updates the sides in the level.
	update_level();

	// If we are connected, send data to the connected host.
	send_level_data();
}


config* connect_engine::current_config() {
	if(config& s = scenario()) {
		return &s;
	}

	return nullptr;
}

void connect_engine::import_user(const std::string& name, const bool observer, int side_taken)
{
	config user_data;
	user_data["name"] = name;
	import_user(user_data, observer, side_taken);
}

void connect_engine::import_user(const config& data, const bool observer, int side_taken)
{
	const std::string& username = data["name"];
	assert(!username.empty());
	if(campaign_info_) {
		connected_users_rw().insert(username);
	}

	update_side_controller_options();

	if(observer) {
		return;
	}

	bool side_assigned = false;
	if(side_taken >= 0) {
		side_engines_[side_taken]->place_user(data, true);
		side_assigned = true;
	}

	// Check if user has a side(s) reserved for him.
	for(side_engine_ptr side : side_engines_) {
		if(side->reserved_for() == username && side->player_id().empty() && side->controller() != CNTR_COMPUTER) {
			side->place_user(data);

			side_assigned = true;
		}
	}

	// If no sides were assigned for a user,
	// take a first available side.
	if(side_taken < 0 && !side_assigned) {
		for(side_engine_ptr side : side_engines_) {
			if(side->available_for_user(username) ||
				side->controller() == CNTR_LOCAL) {
					side->place_user(data);

					side_assigned = true;
					break;
			}
		}
	}

	// Check if user has taken any sides, which should get control
	// over any other sides.
	for(side_engine_ptr user_side : side_engines_) {
		if(user_side->player_id() == username && !user_side->previous_save_id().empty()) {
			for(side_engine_ptr side : side_engines_){
				if(side->player_id().empty() && side->previous_save_id() == user_side->previous_save_id()) {
					side->place_user(data);
				}
			}
		}
	}
}

bool connect_engine::sides_available() const
{
	for(side_engine_ptr side : side_engines_) {
		if(side->available_for_user()) {
			return true;
		}
	}

	return false;
}

void connect_engine::update_level()
{
	DBG_MP << "updating level" << std::endl;

	scenario().clear_children("side");

	for(side_engine_ptr side : side_engines_) {
		scenario().add_child("side", side->new_config());
	}
}

void connect_engine::update_and_send_diff(bool /*update_time_of_day*/)
{
	config old_level = level_;
	update_level();

	config diff = level_.get_diff(old_level);
	if(!diff.empty()) {
		config scenario_diff;
		scenario_diff.add_child("scenario_diff", std::move(diff));
		send_to_server(scenario_diff);
	}
}

bool connect_engine::can_start_game() const
{
	if(side_engines_.empty()) {
		return true;
	}

	// First check if all sides are ready to start the game.
	for(side_engine_ptr side : side_engines_) {
		if(!side->ready_for_start()) {
			const int side_num = side->index() + 1;
			DBG_MP << "not all sides are ready, side " <<
				side_num << " not ready\n";

			return false;
		}
	}

	DBG_MP << "all sides are ready" << std::endl;

	/*
	 * If at least one human player is slotted with a player/ai we're allowed
	 * to start. Before used a more advanced test but it seems people are
	 * creative in what is used in multiplayer [1] so use a simpler test now.
	 * [1] http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=568029
	 */
	for(side_engine_ptr side : side_engines_) {
		if(side->controller() != CNTR_EMPTY && side->allow_player()) {
			return true;
		}
	}

	return false;
}

void connect_engine::send_to_server(const config& cfg) const
{
	if(campaign_info_) {
		campaign_info_->connection.send_data(cfg);
	}
}

bool connect_engine::receive_from_server(config& dst) const
{
	if(campaign_info_) {
		return campaign_info_->connection.receive_data(dst);
	}
	else {
		return false;
	}
}

std::vector<std::string> side_engine::get_children_to_swap()
{
	std::vector<std::string> children;

	children.push_back("village");
	children.push_back("unit");
	children.push_back("ai");

	return children;
}

std::multimap<std::string, config> side_engine::get_side_children()
{
	std::multimap<std::string, config> children;

	for(const std::string& children_to_swap : get_children_to_swap()) {
		for(const config& child : cfg_.child_range(children_to_swap)) {
			children.emplace(children_to_swap, child);
		}
	}

	return children;
}

void side_engine::set_side_children(std::multimap<std::string, config> children)
{
	for(const std::string& children_to_remove : get_children_to_swap()) {
		cfg_.clear_children(children_to_remove);
	}

	for(std::pair<std::string, config> child_map : children) {
		cfg_.add_child(child_map.first, child_map.second);
	}
}

void connect_engine::start_game()
{
	DBG_MP << "starting a new game" << std::endl;

	// Resolves the "random faction", "random gender" and "random message"
	// Must be done before shuffle sides, or some cases will cause errors
	randomness::mt_rng rng; // Make an RNG for all the shuffling and random faction operations
	for(side_engine_ptr side : side_engines_) {
		std::vector<std::string> avoid_faction_ids;

		// If we aren't resolving random factions independently at random, calculate which factions should not appear for this side.
		if(params_.random_faction_mode != mp_game_settings::RANDOM_FACTION_MODE::DEFAULT) {
			for(side_engine_ptr side2 : side_engines_) {
				if(!side2->flg().is_random_faction()) {
					switch(params_.random_faction_mode.v) {
						case mp_game_settings::RANDOM_FACTION_MODE::NO_MIRROR:
							avoid_faction_ids.push_back(side2->flg().current_faction()["id"].str());
							break;
						case mp_game_settings::RANDOM_FACTION_MODE::NO_ALLY_MIRROR:
							if(side2->team() == side->team()) {// TODO: When the connect engines are fixed to allow multiple teams, this should be changed to "if side1 and side2 are allied, i.e. their list of teams has nonempty intersection"
								avoid_faction_ids.push_back(side2->flg().current_faction()["id"].str());
							}
							break;
						default:
							break; // assert(false);
					}
				}
			}
		}
		side->resolve_random(rng, avoid_faction_ids);
	}

	// Shuffle sides (check settings and if it is a re-loaded game).
	// Must be done after resolve_random() or shuffle sides, or they won't work.
	if(state_.mp_settings().shuffle_sides && !force_lock_settings_ && !(level_.child("snapshot") && level_.child("snapshot").child("side"))) {

		// Only playable sides should be shuffled.
		std::vector<int> playable_sides;
		for(side_engine_ptr side : side_engines_) {
			if(side->allow_player() && side->allow_shuffle()) {
				playable_sides.push_back(side->index());
			}
		}

		// Fisher-Yates shuffle.
		for(int i = playable_sides.size(); i > 1; i--) {
			int j_side = playable_sides[rng.get_next_random() % i];
			int i_side = playable_sides[i - 1];

			if(i_side == j_side) continue; //nothing to swap

			// First we swap everything about a side with another
			side_engine_ptr tmp_side = side_engines_[j_side];
			side_engines_[j_side] = side_engines_[i_side];
			side_engines_[i_side] = tmp_side;

			// Some 'child' variables such as village ownership and
			// initial side units need to be swapped over as well
			std::multimap<std::string, config> tmp_side_children = side_engines_[j_side]->get_side_children();
			side_engines_[j_side]->set_side_children(side_engines_[i_side]->get_side_children());
			side_engines_[i_side]->set_side_children(tmp_side_children);

			// Then we revert the swap for fields that are unique to
			// player control and the team they selected
			int tmp_index = side_engines_[j_side]->index();
			side_engines_[j_side]->set_index(side_engines_[i_side]->index());
			side_engines_[i_side]->set_index(tmp_index);

			int tmp_team = side_engines_[j_side]->team();
			side_engines_[j_side]->set_team(side_engines_[i_side]->team());
			side_engines_[i_side]->set_team(tmp_team);
		}
	}

	// Make other clients not show the results of resolve_random().
	config lock("stop_updates");
	send_to_server(lock);

	update_and_send_diff(true);

	save_reserved_sides_information();

	// Build the gamestate object after updating the level.
	mp::level_to_gamestate(level_, state_);

	send_to_server(config("start_game"));
}

void connect_engine::start_game_commandline(const commandline_options& cmdline_opts)
{
	DBG_MP << "starting a new game in commandline mode" << std::endl;

	typedef std::tuple<unsigned int, std::string> mp_option;

	randomness::mt_rng rng;

	unsigned num = 0;
	for(side_engine_ptr side : side_engines_) {
		num++;

		// Set the faction, if commandline option is given.
		if(cmdline_opts.multiplayer_side) {
			for(const mp_option& option : *cmdline_opts.multiplayer_side) {

				if(std::get<0>(option) == num) {
					if(std::find_if(era_factions_.begin(), era_factions_.end(), [&option](const config* faction) { return (*faction)["id"] == std::get<1>(option); }) != era_factions_.end()) {
						DBG_MP << "\tsetting side " << std::get<0>(option) << "\tfaction: " << std::get<1>(option) << std::endl;

						side->set_faction_commandline(std::get<1>(option));
					}
					else {
						ERR_MP << "failed to set side " << std::get<0>(option) << " to faction " << std::get<1>(option) << std::endl;
					}
				}
			}
		}

		// Set the controller, if commandline option is given.
		if(cmdline_opts.multiplayer_controller) {
			for(const mp_option& option : *cmdline_opts.multiplayer_controller) {

				if(std::get<0>(option) == num) {
					DBG_MP << "\tsetting side " << std::get<0>(option) <<
						"\tfaction: " << std::get<1>(option) << std::endl;

					side->set_controller_commandline(std::get<1>(option));
				}
			}
		}

		// Set AI algorithm to RCA AI for all sides,
		// then override if commandline option was given.
		side->set_ai_algorithm("ai_default_rca");
		if(cmdline_opts.multiplayer_algorithm) {
			for(const mp_option& option : *cmdline_opts.multiplayer_algorithm) {

				if(std::get<0>(option) == num) {
					DBG_MP << "\tsetting side " << std::get<0>(option) <<
						"\tfaction: " << std::get<1>(option) << std::endl;

					side->set_ai_algorithm(std::get<1>(option));
				}
			}
		}

		// Finally, resolve "random faction",
		// "random gender" and "random message", if any remains unresolved.
		side->resolve_random(rng);
	} // end top-level loop

	update_and_send_diff(true);

	// Update sides with commandline parameters.
	if(cmdline_opts.multiplayer_turns) {
		DBG_MP << "\tsetting turns: " << *cmdline_opts.multiplayer_turns <<
			std::endl;
		scenario()["turns"] = *cmdline_opts.multiplayer_turns;
	}

	for(config &side : scenario().child_range("side")) {
		if(cmdline_opts.multiplayer_ai_config) {
			for(const mp_option& option : *cmdline_opts.multiplayer_ai_config) {

				if(std::get<0>(option) == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] <<
						"\tai_config: " << std::get<1>(option) << std::endl;

					side["ai_config"] = std::get<1>(option);
				}
			}
		}

		// Having hard-coded values here is undesirable,
		// but that's how it is done in the MP lobby
		// part of the code also.
		// Should be replaced by settings/constants in both places
		if(cmdline_opts.multiplayer_ignore_map_settings) {
			side["gold"] = 100;
			side["income"] = 1;
		}

		typedef std::tuple<unsigned int, std::string, std::string> mp_parameter;

		if(cmdline_opts.multiplayer_parm) {
			for(const mp_parameter& parameter : *cmdline_opts.multiplayer_parm) {

				if(std::get<0>(parameter) == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] << " " <<
						std::get<1>(parameter) << ": " << std::get<2>(parameter) << std::endl;

					side[std::get<1>(parameter)] = std::get<2>(parameter);
				}
			}
		}
    }

	save_reserved_sides_information();

	// Build the gamestate object after updating the level
	mp::level_to_gamestate(level_, state_);
	send_to_server(config("start_game"));
}

void connect_engine::leave_game()
{
	DBG_MP << "leaving the game" << std::endl;

	send_to_server(config("leave_game"));
}

std::pair<bool, bool> connect_engine::process_network_data(const config& data)
{
	std::pair<bool, bool> result(std::make_pair(false, true));

	if(data.child("leave_game")) {
		result.first = true;
		return result;
	}

	// A side has been dropped.
	if(const config& side_drop = data.child("side_drop")) {
		unsigned side_index = side_drop["side_num"].to_int() - 1;

		if(side_index < side_engines_.size()) {
			side_engine_ptr side_to_drop = side_engines_[side_index];

			// Remove user, whose side was dropped.
			connected_users_rw().erase(side_to_drop->player_id());
			update_side_controller_options();

			side_to_drop->reset();

			update_and_send_diff();

			return result;
		}
	}

	// A player is connecting to the game.
	if(!data["side"].empty()) {
		unsigned side_taken = data["side"].to_int() - 1;

		// Checks if the connecting user has a valid and unique name.
		const std::string name = data["name"];
		if(name.empty()) {
			config response;
			response["failed"] = true;
			send_to_server(response);

			ERR_CF << "ERROR: No username provided with the side." << std::endl;

			return result;
		}

		if(connected_users().find(name) != connected_users().end()) {
			// TODO: Seems like a needless limitation
			// to only allow one side per player.
			if(find_user_side_index_by_id(name) != -1) {
				config response;
				response["failed"] = true;
				response["message"] = "The nickname '" + name +
					"' is already in use.";
				send_to_server(response);

				return result;
			} else {
				connected_users_rw().erase(name);
				update_side_controller_options();
				config observer_quit;
				observer_quit.add_child("observer_quit")["name"] = name;
				send_to_server(observer_quit);
			}
		}

		// Assigns this user to a side.
		if(side_taken < side_engines_.size()) {
			if(!side_engines_[side_taken]->available_for_user(name)) {
				// This side is already taken.
				// Try to reassing the player to a different position.
				side_taken = 0;
				for(side_engine_ptr s : side_engines_) {
					if(s->available_for_user()) {
						break;
					}

					side_taken++;
				}

				if(side_taken >= side_engines_.size()) {
					config response;
					response["failed"] = true;
					send_to_server(response);

					config res;
					config& kick = res.add_child("kick");
					kick["username"] = data["name"];
					send_to_server(res);

					update_and_send_diff();

					ERR_CF << "ERROR: Couldn't assign a side to '" <<
						name << "'\n";

					return result;
				}
			}

			LOG_CF << "client has taken a valid position\n";

			import_user(data, false, side_taken);
			update_and_send_diff();

			// Wait for them to choose faction if allowed.
			side_engines_[side_taken]->set_waiting_to_choose_status(side_engines_[side_taken]->allow_changes());
			LOG_MP << "waiting to choose status = " << side_engines_[side_taken]->allow_changes() << std::endl;
			result.second = false;

			LOG_NW << "sent player data\n";
		} else {
			ERR_CF << "tried to take illegal side: " << side_taken << std::endl;

			config response;
			response["failed"] = true;
			send_to_server(response);
		}
	}

	if(const config& change_faction = data.child("change_faction")) {
		int side_taken = find_user_side_index_by_id(change_faction["name"]);
		if(side_taken != -1 || !first_scenario_) {
			import_user(change_faction, false, side_taken);
			update_and_send_diff();
		}
	}

	if(const config& observer = data.child("observer")) {
		import_user(observer, true);
		update_and_send_diff();
	}

	if(const config& observer = data.child("observer_quit")) {
		const t_string& observer_name = observer["name"];

		if(connected_users().find(observer_name) != connected_users().end()) {
			connected_users_rw().erase(observer_name);

			// If the observer was assigned a side, we need to update the controllers.
			if(find_user_side_index_by_id(observer_name) != -1) {
				update_side_controller_options();
				update_and_send_diff();
			}
		}
	}

	return result;
}

int connect_engine::find_user_side_index_by_id(const std::string& id) const
{
	size_t i = 0;
	for(side_engine_ptr side : side_engines_) {
		if(side->player_id() == id) {
			break;
		}

		i++;
	}

	if(i >= side_engines_.size()) {
		return -1;
	}

	return i;
}

void connect_engine::send_level_data() const
{
	// Send initial information.
	if(first_scenario_) {
		send_to_server(config {
			"create_game", config {
				"name", params_.name,
				"password", params_.password,
			},
		});
		send_to_server(level_);
	} else {
		send_to_server(config {"update_game", config()});
		config next_level;
		next_level.add_child("store_next_scenario", level_);
		send_to_server(next_level);
	}
}

void connect_engine::save_reserved_sides_information()
{
	// Add information about reserved sides to the level config.
	// N.B. This information is needed only for a host player.
	std::map<std::string, std::string> side_users = utils::map_split(level_.child_or_empty("multiplayer")["side_users"]);
	for(side_engine_ptr side : side_engines_) {
		const std::string& save_id = side->save_id();
		const std::string& player_id = side->player_id();
		if(!save_id.empty() && !player_id.empty()) {
			side_users[save_id] = player_id;
		}
	}

	level_.child("multiplayer")["side_users"] = utils::join_map(side_users);
}

void connect_engine::load_previous_sides_users()
{
	std::map<std::string, std::string> side_users = utils::map_split(level_.child("multiplayer")["side_users"]);
	std::set<std::string> names;
	for(side_engine_ptr side : side_engines_) {
		const std::string& save_id = side->previous_save_id();
		if(side_users.find(save_id) != side_users.end()) {
			side->set_reserved_for(side_users[save_id]);

			if(side->controller() != CNTR_COMPUTER) {
				side->set_controller(CNTR_RESERVED);
				names.insert(side_users[save_id]);
			}

			side->update_controller_options();
		}
	}

	//Do this in an extra loop to make sure we import each user only once.
	for(const std::string& name : names)
	{
		if(connected_users().find(name) != connected_users().end() || !campaign_info_) {
			import_user(name, false);
		}
	}
}

void connect_engine::update_side_controller_options()
{
	for(side_engine_ptr side : side_engines_) {
		side->update_controller_options();
	}
}

const std::set<std::string>& connect_engine::connected_users() const
{
	if(campaign_info_) {
		return campaign_info_->connected_players;
	}

	static std::set<std::string> empty;
	return empty;
}

std::set<std::string>& connect_engine::connected_users_rw()
{
	assert(campaign_info_);
	return campaign_info_->connected_players;
}

side_engine::side_engine(const config& cfg, connect_engine& parent_engine, const int index)
	: cfg_(cfg)
	, parent_(parent_engine)
	, controller_(CNTR_NETWORK)
	, current_controller_index_(0)
	, controller_options_()
	, allow_player_(cfg["allow_player"].to_bool(true))
	, controller_lock_(cfg["controller_lock"].to_bool(parent_.force_lock_settings_) && parent_.params_.use_map_settings)
	, index_(index)
	, team_(0)
	, color_(std::min(index, gamemap::MAX_PLAYERS - 1))
	, gold_(cfg["gold"].to_int(100))
	, income_(cfg["income"])
	, reserved_for_(cfg["current_player"])
	, player_id_()
	, ai_algorithm_()
	, chose_random_(cfg["chose_random"].to_bool(false))
	, disallow_shuffle_(cfg["disallow_shuffle"].to_bool(false))
	, flg_(parent_.era_factions_, cfg_, parent_.force_lock_settings_, parent_.params_.use_map_settings, parent_.params_.saved_game)
	, allow_changes_(!parent_.params_.saved_game && !(flg_.choosable_factions().size() == 1 && flg_.choosable_leaders().size() == 1 && flg_.choosable_genders().size() == 1))
	, waiting_to_choose_faction_(allow_changes_)
	, color_options_(game_config::default_colors)
	, color_id_(color_options_[color_])
{
	// Save default attributes that could be overwirtten by the faction, so that correct faction lists would be
	// initialized by flg_manager when the new side config is sent over network.
	cfg_.add_child("default_faction", config {
		"type",    cfg_["type"],
		"gender",  cfg_["gender"],
		"faction", cfg_["faction"],
		"recruit", cfg_["recruit"],
	});

	if(cfg_["side"].to_int(index_ + 1) != index_ + 1) {
		ERR_CF << "found invalid side=" << cfg_["side"].to_int(index_ + 1) << " in definition of side number " << index_ + 1 << std::endl;
	}

	cfg_["side"] = index_ + 1;

	// Check if this side should give its control to some other side.
	const size_t side_cntr_index = cfg_["controller"].to_int(-1) - 1;
	if(side_cntr_index < parent_.side_engines().size()) {
		// Remove this attribute to avoid locking side
		// to non-existing controller type.
		cfg_.remove_attribute("controller");

		cfg_["previous_save_id"] = parent_.side_engines()[side_cntr_index]->previous_save_id();
		ERR_MP << "controller=<number> is deperecated\n";
	}

	if(!parent_.params_.saved_game && cfg_["save_id"].str().empty()) {
		assert(cfg_["id"].empty()); // we already set "save_id" to "id" if "id" existed.
		cfg_["save_id"] = parent_.scenario()["id"].str() + "_" + std::to_string(index);
	}

	if(cfg_["controller"] != "human" && cfg_["controller"] != "ai" && cfg_["controller"] != "null") {
		//an invalid controller type was specified. Remove it to prevent asertion failures later.
		cfg_.remove_attribute("controller");
	}

	update_controller_options();

	// Tweak the controllers.
	if(parent_.state_.classification().campaign_type == game_classification::CAMPAIGN_TYPE::SCENARIO && cfg_["controller"].blank()) {
		cfg_["controller"] = "ai";
	}

	if(cfg_["controller"] == "null") {
		set_controller(CNTR_EMPTY);
	} else if(cfg_["controller"] == "ai") {
		set_controller(CNTR_COMPUTER);
	} else if(parent_.default_controller_ == CNTR_NETWORK && !reserved_for_.empty()) {
		// Reserve a side for "current_player", unless the side
		// is played by an AI.
		set_controller(CNTR_RESERVED);
	} else if(allow_player_) {
		set_controller(parent_.default_controller_);
	} else {
		// AI is the default.
		set_controller(CNTR_COMPUTER);
	}

	// Initialize team and color.
	unsigned team_name_index = 0;
	for(const connect_engine::team_data_pod& data : parent_.team_data_) {
		if(data.team_name == cfg["team_name"]) {
			break;
		}

		++team_name_index;
	}

	if(team_name_index >= parent_.team_data_.size()) {
		assert(!parent_.team_data_.empty());
		team_ = 0;
		WRN_MP << "In side_engine constructor: Could not find my team_name " << cfg["team_name"] << " among the mp connect engine's list of team names. I am being assigned to the first team. This may indicate a bug!" << std::endl;
	} else {
		team_ = team_name_index;
	}

	if(!cfg["color"].empty()) {
		if(cfg["color"].to_int()) {
			color_ = cfg["color"].to_int() - 1;
			color_id_ = color_options_[color_];
		} else {
			const std::string custom_color = cfg["color"].str();

			const auto iter = std::find(color_options_.begin(), color_options_.end(), custom_color);

			if(iter != color_options_.end()) {
				color_id_ = *iter;
				color_ = std::distance(color_options_.begin(), iter);
			} else {
				color_options_.push_back(custom_color);

				color_id_ = custom_color;
				color_ = color_options_.size() - 1;
			}
		}
	}

	// Initialize ai algorithm.
	if(const config& ai = cfg.child("ai")) {
		ai_algorithm_ = ai["ai_algorithm"].str();
	}
}

std::string side_engine::user_description() const
{
	switch(controller_) {
	case CNTR_LOCAL:
		return N_("Anonymous player");
	case CNTR_COMPUTER:
		if(allow_player_) {
			return ai::configuration::get_ai_config_for(ai_algorithm_)["description"];
		} else {
			return N_("Computer Player");
		}
	default:
		return "";
	}
}

config side_engine::new_config() const
{
	config res = cfg_;

	// In case of 'shuffle sides' the side index in cfg_ might be wrong which will confuse the team constructor later.
	res["side"] = index_ + 1;

	// If the user is allowed to change type, faction, leader etc,  then import their new values in the config.
	if(!parent_.params_.saved_game) {
		// Merge the faction data to res.
		config faction = flg_.current_faction();
		LOG_MP << "side_engine::new_config: side=" << index_ + 1 << " faction=" << faction["id"] << " recruit=" << faction["recruit"] << "\n";
		res["faction_name"] = faction["name"];
		res["faction"] = faction["id"];
		faction.remove_attributes("id", "name", "image", "gender", "type", "description");
		res.append(faction);
	}

	res["controller"] = controller_names[controller_];

	// The hosts receives the serversided controller tweaks after the start event, but
	// for mp sync it's very important that the controller types are correct
	// during the start/prestart event (otherwise random unit creation during prestart fails).
	res["is_local"] = player_id_ == preferences::login() || controller_ == CNTR_COMPUTER || controller_ == CNTR_LOCAL;

	// This function (new_config) is only meant to be called by the host's machine, which is why this check
	// works. It essentially certifies that whatever side has the player_id that matches the host's login
	// will be flagged. The reason we cannot check mp_campaign_info::is_host is because that flag is *always*
	// true on the host's machine, meaning this flag would be set to true for every side.
	res["is_host"] = player_id_ == preferences::login();

	std::string desc = user_description();
	if(!desc.empty()) {
		res["user_description"] = t_string(desc, "wesnoth");

		desc = vgettext("$playername $side", {
			{"playername", _(desc.c_str())},
			{"side", res["side"].str()}
		});
	} else if(!player_id_.empty()) {
		desc = player_id_;
	}

	if(res["name"].str().empty() && !desc.empty()) {
		//TODO: maybe we should add this in to the leaders config instead of the side config?
		res["name"] = desc;
	}

	if(controller_ == CNTR_COMPUTER && allow_player_) {
		// Do not import default ai cfg otherwise - all is set by scenario config.
		res.add_child_at("ai", config {"ai_algorithm", ai_algorithm_}, 0);
	}

	if(controller_ == CNTR_EMPTY) {
		res["no_leader"] = true;
	}

	// A side's "current_player" is the player which has currently taken that side or the one for which it is reserved.
	// The "player_id" is the id of the client who controls that side. It's always the host for Local and AI players and
	// always empty for free/reserved sides or null controlled sides. You can use !res["player_id"].empty() to check
	// whether a side is already taken.
	assert(!preferences::login().empty());
	if(controller_ == CNTR_LOCAL) {
		res["player_id"] = preferences::login();
		res["current_player"] = preferences::login();
	} else if(controller_ == CNTR_RESERVED) {
		res.remove_attribute("player_id");
		res["current_player"] = reserved_for_;
	} else if(controller_ == CNTR_COMPUTER) {
		// TODO: what is the content of player_id_ here ?
		res["current_player"] = desc;
		res["player_id"] = preferences::login();
	} else if(!player_id_.empty()) {
		res["player_id"] = player_id_;
		res["current_player"] = player_id_;
	}

	res["allow_changes"] = allow_changes_;
	res["chose_random"] = chose_random_;

	if(!parent_.params_.saved_game) {
		// Find a config where a default leader is and set a new type and gender values for it.
		config* leader = &res;

		if(flg_.default_leader_cfg() != nullptr) {
			for(config& side_unit : res.child_range("unit")) {
				if(*flg_.default_leader_cfg() != side_unit) {
					continue;
				}

				leader = &side_unit;

				if(flg_.current_leader() != (*leader)["type"]) {
					// If a new leader type was selected from carryover, make sure that we reset the leader.
					std::string leader_id = (*leader)["id"];
					leader->clear();

					if(!leader_id.empty()) {
						(*leader)["id"] = leader_id;
					}
				}

				break;
			}
		}

		// NOTE: the presence of a type= key overrides no_leader
		if(controller_ != CNTR_EMPTY) {
			(*leader)["type"] = flg_.current_leader();
			(*leader)["gender"] = flg_.current_gender();
			LOG_MP << "side_engine::new_config: side=" << index_ + 1 << " type=" << (*leader)["type"] << " gender=" << (*leader)["gender"] << "\n";
		} else {
			// TODO: FIX THIS SHIT! We shouldn't have a special string to denote no-leader-ness...
			(*leader)["type"] = "null";
			(*leader)["gender"] = "null";
		}

		res["team_name"] = parent_.team_data_[team_].team_name;

		// TODO: Fix this mess!
		//
		// There is a fundamental disconnect, here. One the one hand we have the idea of
		// 'teams' (which don't actually exist). A 'team' has a name (internal key:
		// team_name) and a translatable display name (internal key: user_team_name). But
		// what we actually have are sides. Sides relate to each other by 'team' (internal
		// key: team_name) and each side has it's own name for the team (internal key:
		// user_team_name).
		//
		// The confusion is that the keys from the side have names which one might expect
		// always refer to the 'team' concept. THEY DO NOT! They are simply named in such
		// a way to confuse the unwary.
		//
		// There is no simple, clean way to clear up the confusion. So, I'm applying the
		// Principle of Least Surprise. The user can see the user_team_name, and it should
		// not change. So if the side already has a user_team_name, use it.
		//
		// In the rare and unlikely (like, probably never happens) case that the side does
		// not have a user_team_name, but an (nebulous and non-deterministic term here)
		// EARLIER side has the same team_name and that side gives a user_team_name, we'll
		// use it.
		//
		// The effect of this mess, and my lame fix for it, is probably only visible when
		// randomizing the sides on a team for multi-player games. But the effect when it's
		// not fixed is an obvious mistake on the player's screen when playing a campaign
		// in single-player mode.
		//
		// At some level, all this is probably wrong, but it is the least breakage from the
		// mess I found; so deal with it, or fix it.
		//
		// If, by now, you get the impression this is a kludged-together mess which cries
		// out for an honest design and a thoughtful implementation, you're correct! But
		// I'm tired, and I'm cranky from wasting a over day on this, and so I'm exercising
		// my prerogative as a grey-beard and leaving this for someone else to clean up.
		if(res["user_team_name"].empty() || !parent_.params_.use_map_settings) {
			res["user_team_name"] = parent_.team_data_[team_].user_team_name;
		}

		res["allow_player"] = allow_player_;
		res["color"] = color_id_;
		res["gold"] = gold_;
		res["income"] = income_;
	}

	if(parent_.params_.use_map_settings && !parent_.params_.saved_game) {
		config trimmed = cfg_;

		for(const std::string& attribute : attributes_to_trim) {
			trimmed.remove_attribute(attribute);
		}

		if(controller_ != CNTR_COMPUTER) {
			// Only override names for computer controlled players.
			trimmed.remove_attribute("user_description");
		}

		res.merge_with(trimmed);
	}

	return res;
}

bool side_engine::ready_for_start() const
{
	if(!allow_player_) {
		// Sides without players are always ready.
		return true;
	}

	if((controller_ == CNTR_COMPUTER) ||
		(controller_ == CNTR_EMPTY) ||
		(controller_ == CNTR_LOCAL)) {

		return true;
	}

	if(available_for_user()) {
		// If controller_ == CNTR_NETWORK and player_id_.empty().
		return false;
	}

	if(controller_ == CNTR_NETWORK) {
		if(player_id_ == preferences::login() || !waiting_to_choose_faction_ || !allow_changes_) {
			// The host is ready. A network player, who got a chance
			// to choose faction if allowed, is also ready.
			return true;
		}
	}

	return false;
}

bool side_engine::available_for_user(const std::string& name) const
{
	if(controller_ == CNTR_NETWORK && player_id_.empty()) {
		// Side is free and waiting for user.
		return true;
	}

	if(controller_ == CNTR_RESERVED && name.empty()) {
		// Side is still available to someone.
		return true;
	}

	if(controller_ == CNTR_RESERVED && reserved_for_ == name) {
		// Side is available only for the player with specific name.
		return true;
	}

	return false;
}

void side_engine::resolve_random(randomness::mt_rng & rng, const std::vector<std::string> & avoid_faction_ids)
{
	if(parent_.params_.saved_game) {
		return;
	}

	chose_random_ = flg_.is_random_faction();

	flg_.resolve_random(rng, avoid_faction_ids);

	LOG_MP << "side " << (index_ + 1) << ": faction=" <<
		(flg_.current_faction())["name"] << ", leader=" <<
		flg_.current_leader() << ", gender=" << flg_.current_gender() << "\n";
}

void side_engine::reset()
{
	player_id_.clear();
	set_waiting_to_choose_status(false);
	set_controller(parent_.default_controller_);

	if(!parent_.params_.saved_game) {
		flg_.set_current_faction(0);
	}
}

void side_engine::place_user(const std::string& name)
{
	config data;
	data["name"] = name;

	place_user(data);
}

void side_engine::place_user(const config& data, bool contains_selection)
{
	player_id_ = data["name"].str();
	set_controller(parent_.default_controller_);

	if(data["change_faction"].to_bool() && contains_selection) {
		// Network user's data carry information about chosen
		// faction, leader and genders.
		flg_.set_current_faction(data["faction"].str());
		flg_.set_current_leader(data["leader"].str());
		flg_.set_current_gender(data["gender"].str());
	}

	waiting_to_choose_faction_ = false;
}

void side_engine::update_controller_options()
{
	controller_options_.clear();

	// Default options.
	if(parent_.campaign_info_) {
		add_controller_option(CNTR_NETWORK, _("Network Player"), "human");
	}

	add_controller_option(CNTR_LOCAL, _("Local Player"), "human");
	add_controller_option(CNTR_COMPUTER, _("Computer Player"), "ai");
	add_controller_option(CNTR_EMPTY, _("Empty"), "null");

	if(!reserved_for_.empty()) {
		add_controller_option(CNTR_RESERVED, _("Reserved"), "human");
	}

	// Connected users.
	for(const std::string& user : parent_.connected_users()) {
		add_controller_option(parent_.default_controller_, user, "human");
	}

	update_current_controller_index();
}

void side_engine::update_current_controller_index()
{
	int i = 0;
	for(const controller_option& option : controller_options_) {
		if(option.first == controller_) {
			current_controller_index_ = i;

			if(player_id_.empty() || player_id_ == option.second) {
				// Stop searching if no user is assigned to a side
				// or the selected user is found.
				break;
			}
		}

		i++;
	}

	assert(current_controller_index_ < controller_options_.size());
}

bool side_engine::controller_changed(const int selection)
{
	const ng::controller selected_cntr = controller_options_[selection].first;

	// Check if user was selected. If so assign a side to him/her.
	// If not, make sure that no user is assigned to this side.
	if(selected_cntr == parent_.default_controller_ && selection != 0) {
		player_id_ = controller_options_[selection].second;
		set_waiting_to_choose_status(false);
	} else {
		player_id_.clear();
	}

	set_controller(selected_cntr);

	return true;
}

void side_engine::set_controller(ng::controller controller)
{
	controller_ = controller;

	update_current_controller_index();
}

void side_engine::set_faction_commandline(const std::string& faction_name)
{
	flg_.set_current_faction(faction_name);
}

void side_engine::set_controller_commandline(const std::string& controller_name)
{
	set_controller(CNTR_LOCAL);

	if(controller_name == "ai") {
		set_controller(CNTR_COMPUTER);
	}

	if(controller_name == "null") {
		set_controller(CNTR_EMPTY);
	}

	player_id_.clear();
}

void side_engine::add_controller_option(ng::controller controller,
		const std::string& name, const std::string& controller_value)
{
	if(controller_lock_ && !cfg_["controller"].empty() &&
		cfg_["controller"] != controller_value) {

		return;
	}

	controller_options_.emplace_back(controller, name);
}

} // end namespace ng
