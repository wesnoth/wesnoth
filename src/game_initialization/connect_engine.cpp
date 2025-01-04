/*
	Copyright (C) 2013 - 2024
	by Andrius Silinskas <silinskas.andrius@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "game_initialization/multiplayer.hpp"
#include "game_initialization/playcampaign.hpp"
#include "preferences/preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "mt_rng.hpp"
#include "side_controller.hpp"
#include "team.hpp"

#include <array>
#include <cstdlib>

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

namespace
{
const std::array controller_names {
	side_controller::human,
	side_controller::human,
	side_controller::ai,
	side_controller::none,
	side_controller::reserved
};

const std::set<std::string> children_to_swap {
	"village",
	"unit",
	"ai"
};
} // end anon namespace

namespace ng {

connect_engine::connect_engine(saved_game& state, const bool first_scenario, mp_game_metadata* metadata)
	: level_()
	, state_(state)
	, params_(state.mp_settings())
	, default_controller_(metadata ? CNTR_NETWORK : CNTR_LOCAL)
	, mp_metadata_(metadata)
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
	force_lock_settings_ = (state.mp_settings().saved_game != saved_game_mode::type::midgame) && scenario()["force_lock_settings"].to_bool(!is_mp);

	// Original level sides.
	config::child_itors sides = current_config()->child_range("side");

	// AI algorithms.
	if(auto era = level_.optional_child("era")) {
		ai::configuration::add_era_ai_from_config(*era);
	}
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
	for(const config& era : level_.mandatory_child("era").child_range("multiplayer_side")) {
		era_factions_.push_back(&era);
	}

	// Sort alphabetically, but with the random faction options always first.
	// Since some eras have multiple random options we can't just assume there is
	// only one random faction on top of the list.
	std::sort(era_factions_.begin(), era_factions_.end(), [](const config* c1, const config* c2) {
		const config& lhs = *c1;
		const config& rhs = *c2;

		// Random factions always first.
		if(lhs["random_faction"].to_bool() && !rhs["random_faction"].to_bool()) {
			return true;
		}

		if(!lhs["random_faction"].to_bool() && rhs["random_faction"].to_bool()) {
			return false;
		}

		return translation::compare(lhs["name"].str(), rhs["name"].str()) < 0;
	});

	game_config::add_color_info(game_config_view::wrap(scenario()));

	// Create side engines.
	int index = 0;
	for(const config& s : sides) {
		side_engines_.emplace_back(new side_engine(s, *this, index++));
	}

	if(first_scenario_) {
		// Add host to the connected users list.
		import_user(prefs::get().login(), false);
	} else {
		// Add host but don't assign a side to him.
		import_user(prefs::get().login(), true);

		// Load reserved players information into the sides.
		load_previous_sides_users();
	}

	// Only updates the sides in the level.
	update_level();

	// If we are connected, send data to the connected host.
	send_level_data();
}


config* connect_engine::current_config() {
	return &scenario();
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
	if(mp_metadata_) {
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
	DBG_MP << "updating level";

	scenario().clear_children("side");

	for(side_engine_ptr side : side_engines_) {
		scenario().add_child("side", side->new_config());
	}
}

void connect_engine::update_and_send_diff()
{
	config old_level = level_;
	update_level();

	config diff = level_.get_diff(old_level);
	if(!diff.empty()) {
		config scenario_diff;
		scenario_diff.add_child("scenario_diff", std::move(diff));
		mp::send_to_server(scenario_diff);
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
				side_num << " not ready";

			return false;
		}
	}

	DBG_MP << "all sides are ready";

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

std::multimap<std::string, config> side_engine::get_side_children()
{
	std::multimap<std::string, config> children;

	for(const std::string& to_swap : children_to_swap) {
		for(const config& child : cfg_.child_range(to_swap)) {
			children.emplace(to_swap, child);
		}
	}

	return children;
}

void side_engine::set_side_children(const std::multimap<std::string, config>& children)
{
	for(const std::string& children_to_remove : children_to_swap) {
		cfg_.clear_children(children_to_remove);
	}

	for(std::pair<std::string, config> child_map : children) {
		cfg_.add_child(child_map.first, child_map.second);
	}
}

void connect_engine::start_game()
{
	DBG_MP << "starting a new game";

	// Resolves the "random faction", "random gender" and "random message"
	// Must be done before shuffle sides, or some cases will cause errors
	randomness::mt_rng rng; // Make an RNG for all the shuffling and random faction operations
	for(side_engine_ptr side : side_engines_) {
		std::vector<std::string> avoid_faction_ids;

		// If we aren't resolving random factions independently at random, calculate which factions should not appear for this side.
		if(params_.mode != random_faction_mode::type::independent) {
			for(side_engine_ptr side2 : side_engines_) {
				if(!side2->flg().is_random_faction()) {
					switch(params_.mode) {
						case random_faction_mode::type::no_mirror:
							avoid_faction_ids.push_back(side2->flg().current_faction()["id"].str());
							break;
						case random_faction_mode::type::no_ally_mirror:
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
	if(state_.mp_settings().shuffle_sides && !force_lock_settings_ && !(level_.has_child("snapshot") && level_.mandatory_child("snapshot").has_child("side"))) {

		// Only playable sides should be shuffled.
		std::vector<int> playable_sides;
		for(side_engine_ptr side : side_engines_) {
			if(side->allow_player() && side->allow_shuffle()) {
				playable_sides.push_back(side->index());
			}
		}

		// Fisher-Yates shuffle.
		for(int i = playable_sides.size(); i > 1; i--) {
			const int j_side = playable_sides[rng.get_next_random() % i];
			const int i_side = playable_sides[i - 1];

			if(i_side == j_side) continue; //nothing to swap

			// First we swap everything about a side with another
			std::swap(side_engines_[j_side], side_engines_[i_side]);

			// Some 'child' variables such as village ownership and initial side units need to be swapped over as well
			std::multimap<std::string, config> tmp_side_children = side_engines_[j_side]->get_side_children();
			side_engines_[j_side]->set_side_children(side_engines_[i_side]->get_side_children());
			side_engines_[i_side]->set_side_children(tmp_side_children);

			// Then we revert the swap for fields that are unique to player control and the team they selected
			std::swap(side_engines_[j_side]->index_, side_engines_[i_side]->index_);
			std::swap(side_engines_[j_side]->team_,  side_engines_[i_side]->team_);
		}
	}

	// Make other clients not show the results of resolve_random().
	config lock("stop_updates");
	mp::send_to_server(lock);

	update_and_send_diff();

	save_reserved_sides_information();

	// Build the gamestate object after updating the level.
	mp::level_to_gamestate(level_, state_);

	mp::send_to_server(config("start_game"));
}

void connect_engine::start_game_commandline(const commandline_options& cmdline_opts, const game_config_view& game_config)
{
	DBG_MP << "starting a new game in commandline mode";

	randomness::mt_rng rng;

	unsigned num = 0;
	for(side_engine_ptr side : side_engines_) {
		num++;

		// Set the faction, if commandline option is given.
		if(cmdline_opts.multiplayer_side) {
			for(const auto& [side_num, faction_id] : *cmdline_opts.multiplayer_side) {
				if(side_num == num) {
					if(std::find_if(era_factions_.begin(), era_factions_.end(),
						   [fid = faction_id](const config* faction) { return (*faction)["id"] == fid; })
						!= era_factions_.end()
					) {
						DBG_MP << "\tsetting side " << side_num << "\tfaction: " << faction_id;
						side->set_faction_commandline(faction_id);
					} else {
						ERR_MP << "failed to set side " << side_num << " to faction " << faction_id;
					}
				}
			}
		}

		// Set the controller, if commandline option is given.
		if(cmdline_opts.multiplayer_controller) {
			for(const auto& [side_num, faction_id] : *cmdline_opts.multiplayer_controller) {
				if(side_num == num) {
					DBG_MP << "\tsetting side " << side_num << "\tfaction: " << faction_id;
					side->set_controller_commandline(faction_id);
				}
			}
		}

		// Set AI algorithm to default for all sides,
		// then override if commandline option was given.
		std::string ai_algorithm = game_config.mandatory_child("ais")["default_ai_algorithm"].str();
		side->set_ai_algorithm(ai_algorithm);

		if(cmdline_opts.multiplayer_algorithm) {
			for(const auto& [side_num, faction_id] : *cmdline_opts.multiplayer_algorithm) {
				if(side_num == num) {
					DBG_MP << "\tsetting side " << side_num << "\tfaction: " << faction_id;
					side->set_ai_algorithm(faction_id);
				}
			}
		}

		// Finally, resolve "random faction",
		// "random gender" and "random message", if any remains unresolved.
		side->resolve_random(rng);
	} // end top-level loop

	update_and_send_diff();

	// Update sides with commandline parameters.
	if(cmdline_opts.multiplayer_turns) {
		DBG_MP << "\tsetting turns: " << *cmdline_opts.multiplayer_turns;
		scenario()["turns"] = *cmdline_opts.multiplayer_turns;
	}

	for(config& side : scenario().child_range("side")) {
		if(cmdline_opts.multiplayer_ai_config) {
			for(const auto& [side_num, faction_id] : *cmdline_opts.multiplayer_ai_config) {
				if(side_num == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] << "\tai_config: " << faction_id;
					side["ai_config"] = faction_id;
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

		if(cmdline_opts.multiplayer_parm) {
			for(const auto& [side_num, pname, pvalue] : *cmdline_opts.multiplayer_parm) {
				if(side_num == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] << " " << pname << ": " << pvalue;
					side[pname] = pvalue;
				}
			}
		}
	}

	save_reserved_sides_information();

	// Build the gamestate object after updating the level
	mp::level_to_gamestate(level_, state_);
	mp::send_to_server(config("start_game"));
}

void connect_engine::leave_game()
{
	DBG_MP << "leaving the game";

	mp::send_to_server(config("leave_game"));
}

std::pair<bool, bool> connect_engine::process_network_data(const config& data)
{
	std::pair<bool, bool> result(false, true);

	if(data.has_child("leave_game")) {
		result.first = true;
		return result;
	}

	// A side has been dropped.
	if(auto side_drop = data.optional_child("side_drop")) {
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
			mp::send_to_server(response);

			ERR_CF << "ERROR: No username provided with the side.";

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
				mp::send_to_server(response);

				return result;
			} else {
				connected_users_rw().erase(name);
				update_side_controller_options();
				config observer_quit;
				observer_quit.add_child("observer_quit")["name"] = name;
				mp::send_to_server(observer_quit);
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
					mp::send_to_server(response);

					config res;
					config& kick = res.add_child("kick");
					kick["username"] = data["name"];
					mp::send_to_server(res);

					update_and_send_diff();

					ERR_CF << "ERROR: Couldn't assign a side to '" <<
						name << "'";

					return result;
				}
			}

			LOG_CF << "client has taken a valid position";

			import_user(data, false, side_taken);
			update_and_send_diff();

			// Wait for them to choose faction if allowed.
			side_engines_[side_taken]->set_waiting_to_choose_status(side_engines_[side_taken]->allow_changes());
			LOG_MP << "waiting to choose status = " << side_engines_[side_taken]->allow_changes();
			result.second = false;

			LOG_NW << "sent player data";
		} else {
			ERR_CF << "tried to take illegal side: " << side_taken;

			config response;
			response["failed"] = true;
			mp::send_to_server(response);
		}
	}

	if(auto change_faction = data.optional_child("change_faction")) {
		int side_taken = find_user_side_index_by_id(change_faction["name"]);
		if(side_taken != -1 || !first_scenario_) {
			import_user(*change_faction, false, side_taken);
			update_and_send_diff();
		}
	}

	if(auto observer = data.optional_child("observer")) {
		import_user(*observer, true);
		update_and_send_diff();
	}

	if(auto observer = data.optional_child("observer_quit")) {
		const std::string& observer_name = observer["name"];

		if(connected_users().find(observer_name) != connected_users().end()) {
			connected_users_rw().erase(observer_name);
			update_side_controller_options();

			// If the observer was assigned a side, we need to send an update to other
			// players so they no longer see the observer assigned to that side.
			if(find_user_side_index_by_id(observer_name) != -1) {
				update_and_send_diff();
			}
		}
	}

	return result;
}

int connect_engine::find_user_side_index_by_id(const std::string& id) const
{
	std::size_t i = 0;
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
		mp::send_to_server(config {
			"create_game", config {
				"name", params_.name,
				"password", params_.password,
				"ignored", prefs::get().get_ignored_delim(),
				"auto_hosted", false,
			},
		});
		mp::send_to_server(level_);
	} else {
		config next_level;
		next_level.add_child("store_next_scenario", level_);
		mp::send_to_server(next_level);
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

	level_.mandatory_child("multiplayer")["side_users"] = utils::join_map(side_users);
}

void connect_engine::load_previous_sides_users()
{
	std::map<std::string, std::string> side_users = utils::map_split(level_.mandatory_child("multiplayer")["side_users"]);
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
		if(connected_users().find(name) != connected_users().end() || !mp_metadata_) {
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
	if(mp_metadata_) {
		return mp_metadata_->connected_players;
	}

	static std::set<std::string> empty;
	return empty;
}

std::set<std::string>& connect_engine::connected_users_rw()
{
	assert(mp_metadata_);
	return mp_metadata_->connected_players;
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
	, income_(cfg["income"].to_int())
	, reserved_for_(cfg["current_player"])
	, player_id_()
	, ai_algorithm_()
	, chose_random_(cfg["chose_random"].to_bool(false))
	, disallow_shuffle_(cfg["disallow_shuffle"].to_bool(false))
	, flg_(parent_.era_factions_, cfg_, parent_.force_lock_settings_, parent_.params_.use_map_settings, parent_.params_.saved_game == saved_game_mode::type::midgame)
	, allow_changes_(parent_.params_.saved_game != saved_game_mode::type::midgame && !(flg_.choosable_factions().size() == 1 && flg_.choosable_leaders().size() == 1 && flg_.choosable_genders().size() == 1))
	, waiting_to_choose_faction_(allow_changes_)
	, color_options_(game_config::default_colors)
	//TODO: what should we do if color_ is out of range?
	, color_id_(color_options_.at(color_))
{

	// Save default attributes that could be overwritten by the faction, so that correct faction lists would be
	// initialized by flg_manager when the new side config is sent over network.
	cfg_.clear_children("default_faction");
	cfg_.add_child("default_faction", config {
		"faction", cfg_["faction"],
		"recruit", cfg_["recruit"],
	});
	if(auto p_cfg = cfg_.optional_child("leader")) {
		cfg_.mandatory_child("default_faction").add_child("leader", config {
			"type", (p_cfg)["type"],
			"gender", (p_cfg)["gender"],
		});
	}


	if(cfg_["side"].to_int(index_ + 1) != index_ + 1) {
		ERR_CF << "found invalid side=" << cfg_["side"].to_int(index_ + 1) << " in definition of side number " << index_ + 1;
	}

	cfg_["side"] = index_ + 1;

	if(cfg_["controller"] != side_controller::human && cfg_["controller"] != side_controller::ai && cfg_["controller"] != side_controller::none) {
		//an invalid controller type was specified. Remove it to prevent asertion failures later.
		cfg_.remove_attribute("controller");
	}

	update_controller_options();

	// Tweak the controllers.
	if(parent_.state_.classification().is_scenario() && cfg_["controller"].blank()) {
		cfg_["controller"] = side_controller::ai;
	}

	if(cfg_["controller"] == side_controller::none) {
		set_controller(CNTR_EMPTY);
	} else if(cfg_["controller"] == side_controller::ai) {
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
		WRN_MP << "In side_engine constructor: Could not find my team_name " << cfg["team_name"] << " among the mp connect engine's list of team names. I am being assigned to the first team. This may indicate a bug!";
	} else {
		team_ = team_name_index;
	}

	// Check the value of the config's color= key.
	const std::string given_color = team::get_side_color_id_from_config(cfg_);

	if(!given_color.empty()) {
		// If it's valid, save the color...
		color_id_ = given_color;

		// ... and find the appropriate index for it.
		const auto iter = std::find(color_options_.begin(), color_options_.end(), color_id_);

		if(iter != color_options_.end()) {
			color_ = std::distance(color_options_.begin(), iter);
		} else {
			color_options_.push_back(color_id_);
			color_ = color_options_.size() - 1;
		}
	}

	// Initialize ai algorithm.
	if(auto ai = cfg.optional_child("ai")) {
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
	if(parent_.params_.saved_game != saved_game_mode::type::midgame) {
		// Merge the faction data to res.
		config faction = flg_.current_faction();
		LOG_MP << "side_engine::new_config: side=" << index_ + 1 << " faction=" << faction["id"] << " recruit=" << faction["recruit"];
		res["faction_name"] = faction["name"];
		res["faction"] = faction["id"];
		faction.remove_attributes("id", "name", "image", "gender", "type", "description");
		res.append(faction);
	}

	res["controller"] = controller_names[controller_];

	// The hosts receives the serversided controller tweaks after the start event, but
	// for mp sync it's very important that the controller types are correct
	// during the start/prestart event (otherwise random unit creation during prestart fails).
	res["is_local"] = player_id_ == prefs::get().login() || controller_ == CNTR_COMPUTER || controller_ == CNTR_LOCAL;

	// This function (new_config) is only meant to be called by the host's machine, which is why this check
	// works. It essentially certifies that whatever side has the player_id that matches the host's login
	// will be flagged. The reason we cannot check mp_game_metadata::is_host is because that flag is *always*
	// true on the host's machine, meaning this flag would be set to true for every side.
	res["is_host"] = player_id_ == prefs::get().login();

	std::string desc = user_description();
	if(!desc.empty()) {
		res["user_description"] = t_string(desc, "wesnoth");

		desc = VGETTEXT("$playername $side", {
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
		// If this is a saved game and "use_saved" (the default) was chosen for the
		// AI algorithm, we do nothing. Otherwise we add the chosen AI and if this
		// is a saved game, we also remove the old stages from the AI config.
		if(ai_algorithm_ != "use_saved") {
			if(parent_.params_.saved_game == saved_game_mode::type::midgame) {
				for (config &ai_config : res.child_range("ai")) {
					ai_config.clear_children("stage");
				}
			}
			res.add_child_at("ai", config {"ai_algorithm", ai_algorithm_}, 0);
		}
	}

	// A side's "current_player" is the player which has currently taken that side or the one for which it is reserved.
	// The "player_id" is the id of the client who controls that side. It's always the host for Local and AI players and
	// always empty for free/reserved sides or null controlled sides. You can use !res["player_id"].empty() to check
	// whether a side is already taken.
	assert(!prefs::get().login().empty());
	if(controller_ == CNTR_LOCAL) {
		res["player_id"] = prefs::get().login();
		res["current_player"] = prefs::get().login();
	} else if(controller_ == CNTR_RESERVED) {
		res.remove_attribute("player_id");
		res["current_player"] = reserved_for_;
	} else if(controller_ == CNTR_COMPUTER) {
		// TODO: what is the content of player_id_ here ?
		res["current_player"] = desc;
		res["player_id"] = prefs::get().login();
	} else if(!player_id_.empty()) {
		res["player_id"] = player_id_;
		res["current_player"] = player_id_;
	}

	res["allow_changes"] = allow_changes_;
	res["chose_random"] = chose_random_;

	if(parent_.params_.saved_game != saved_game_mode::type::midgame) {

		if(!flg_.leader_lock()) {
			if(controller_ != CNTR_EMPTY) {
				auto& leader = res.child_or_add("leader");
				leader["type"] = flg_.current_leader();
				leader["gender"] = flg_.current_gender();
				LOG_MP << "side_engine::new_config: side=" << index_ + 1 << " type=" << leader["type"]
					   << " gender=" << leader["gender"];
			} else if(!controller_lock_) {
				//if controller_lock_ == false and controller_ == CNTR_EMPTY, this means the user disalbles this side, so remove it's leader.
				res.remove_children("leader");
			}
		}

		const std::string& new_team_name = parent_.team_data_[team_].team_name;

		if(res["user_team_name"].empty() || !parent_.params_.use_map_settings || res["team_name"] != new_team_name) {
			res["team_name"] = new_team_name;
			res["user_team_name"] = parent_.team_data_[team_].user_team_name;
		}

		res["allow_player"] = allow_player_;
		res["color"] = color_id_;
		res["gold"] = gold_;
		res["income"] = income_;
	}


	if(parent_.params_.use_map_settings && parent_.params_.saved_game != saved_game_mode::type::midgame) {
		if(cfg_.has_attribute("name")){
			res["name"] = cfg_["name"];
		}
		if(cfg_.has_attribute("user_description") && controller_ == CNTR_COMPUTER){
			res["user_description"] = cfg_["user_description"];
		}
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
		if(player_id_ == prefs::get().login() || !waiting_to_choose_faction_ || !allow_changes_) {
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
	if(parent_.params_.saved_game == saved_game_mode::type::midgame) {
		return;
	}

	chose_random_ = flg_.is_random_faction();

	flg_.resolve_random(rng, avoid_faction_ids);

	LOG_MP << "side " << (index_ + 1) << ": faction=" <<
		(flg_.current_faction())["name"] << ", leader=" <<
		flg_.current_leader() << ", gender=" << flg_.current_gender();
}

void side_engine::reset()
{
	player_id_.clear();
	set_waiting_to_choose_status(false);
	set_controller(parent_.default_controller_);

	if(parent_.params_.saved_game != saved_game_mode::type::midgame) {
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
	if(parent_.mp_metadata_) {
		add_controller_option(CNTR_NETWORK, _("Network Player"), side_controller::human);
	}

	add_controller_option(CNTR_LOCAL, _("Local Player"), side_controller::human);
	add_controller_option(CNTR_COMPUTER, _("Computer Player"), side_controller::ai);
	add_controller_option(CNTR_EMPTY, _("Nobody"), side_controller::none);

	if(!reserved_for_.empty()) {
		add_controller_option(CNTR_RESERVED, _("Reserved"), side_controller::human);
	}

	// Connected users.
	for(const std::string& user : parent_.connected_users()) {
		add_controller_option(parent_.default_controller_, user, side_controller::human);
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

	if(controller_name == side_controller::ai) {
		set_controller(CNTR_COMPUTER);
	}

	if(controller_name == side_controller::none) {
		set_controller(CNTR_EMPTY);
	}

	player_id_.clear();
}

void side_engine::add_controller_option(ng::controller controller,
		const std::string& name, const std::string& controller_value)
{
	if(controller_lock_ && !cfg_["controller"].empty() && cfg_["controller"] != controller_value) {
		return;
	}

	controller_options_.emplace_back(controller, name);
}

} // end namespace ng
