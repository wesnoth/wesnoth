/*
   Copyright (C) 2013 - 2014 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "multiplayer_connect_engine.hpp"

#include "ai/configuration.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "multiplayer_ui.hpp"
#include "mp_game_utils.hpp"
#include "sound.hpp"
#include "tod_manager.hpp"

#include <boost/foreach.hpp>
#include <stdlib.h>
static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_connect_engine("mp/connect/engine");
#define DBG_MP LOG_STREAM(debug, log_mp_connect_engine)
#define LOG_MP LOG_STREAM(info, log_mp_connect_engine)

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)

namespace {

const std::string controller_names[] = {
	"network",
	"human",
	"ai",
	"null",
	"reserved"
};

const std::string attributes_to_trim[] = {
	"side",
	"recruit",
	"extra_recruit",
	"previous_recruits",
	"controller",
	"current_player",
	"team_name",
	"user_team_name",
	"color",
	"gold",
	"income",
	"allow_changes"
};

}

namespace mp {

connect_engine::connect_engine(game_display& disp, game_state& state,
	const mp_game_settings& params, const bool local_players_only,
	const bool first_scenario) :
	level_(),
	state_(state),
	params_(params),
	default_controller_(local_players_only ? CNTR_LOCAL: CNTR_NETWORK),
	local_players_only_(local_players_only),
	first_scenario_(first_scenario),
	force_lock_settings_(),
	side_engines_(),
	era_factions_(),
	team_names_(),
	user_team_names_(),
	player_teams_(),
	connected_users_()
{
	// Initial level config from the mp_game_settings.
	level_ = initial_level_config(disp, params_, state_);
	if (level_.empty()) {
		return;
	}

	force_lock_settings_ = level_["force_lock_settings"].to_bool();

	// Original level sides.
	config::child_itors sides = current_config()->child_range("side");

	// AI algorithms.
	ai::configuration::add_era_ai_from_config(level_.child("era"));
	ai::configuration::add_mod_ai_from_config(
		level_.child_range("modification"));

	// Set the team name lists and modify the original level sides,
	// if necessary.
	std::vector<std::string> original_team_names;
	std::string team_prefix(std::string(_("Team")) + " ");
	int side_count = 1;
	BOOST_FOREACH(config& side, sides) {
		const std::string side_str = lexical_cast<std::string>(side_count);
		config::attribute_value& team_name = side["team_name"];
		config::attribute_value& user_team_name =
			side["user_team_name"];

		// Revert to default values if appropriate.
		if (team_name.empty()) {
			team_name = side_str;
		}
		if (params_.use_map_settings && user_team_name.empty()) {
			user_team_name = team_name;
		}

		bool add_team = true;
		if (params_.use_map_settings) {
			// Only add a team if it is not found.
			bool found = std::find(team_names_.begin(), team_names_.end(),
				team_name.str()) != team_names_.end();

			if (found) {
				add_team = false;
			}
		} else {
			// Always add a new team for every side, but leave
			// the specified team assigned to a side if there is one.
			std::vector<std::string>::const_iterator name_itor =
				std::find(original_team_names.begin(),
					original_team_names.end(), team_name.str());
			if (name_itor == original_team_names.end()) {
				original_team_names.push_back(team_name);

				team_name =
					lexical_cast<std::string>(original_team_names.size());
			} else {
				team_name = lexical_cast<std::string>(
					name_itor - original_team_names.begin() + 1);
			}

			user_team_name = team_prefix + side_str;
		}

		if (add_team) {
			team_names_.push_back(params_.use_map_settings ? team_name :
				side_str);
			user_team_names_.push_back(user_team_name.t_str().to_serialized());

			if (side["allow_player"].to_bool(true)) {
				player_teams_.push_back(user_team_name.str());
			}
		}

		++side_count;
	}

	// Selected era's factions.
	BOOST_FOREACH(const config& era,
		level_.child("era").child_range("multiplayer_side")) {

		era_factions_.push_back(&era);
	}

	// Create side engines.
	int index = 0;
	BOOST_FOREACH(const config &s, sides) {
		side_engine_ptr new_side_engine(new side_engine(s, *this, index));
		side_engines_.push_back(new_side_engine);

		index++;
	}

	// Load reserved players information into the sides.
	load_previous_sides_users(RESERVE_USERS);

	// Add host to the connected users list.
	import_user(preferences::login(), false);

	// Send initial information.
	config response;
	if (first_scenario_) {
		config& create_game = response.add_child("create_game");
		create_game["name"] = params_.name;
		if (params_.password.empty() == false) {
			response["password"] = params_.password;
		}
	} else {
		response.add_child("update_game");
	}
	network::send_data(response, 0);

	update_level();

	// If we are connected, send data to the connected host.
	send_level_data(0);
}

connect_engine::~connect_engine()
{
}

config* connect_engine::current_config() {
	config* cfg_level = NULL;

	// It might make sense to invent a mechanism of some sort to check
	// whether a config node contains information
	// that you can load from(side information, specifically).
	config &snapshot = level_.child("snapshot");
	if (snapshot && snapshot.child("side")) {
		// Savegame.
		cfg_level = &snapshot;
	} else if (!level_.child("side")) {
		// Start-of-scenario save,
		// the info has to be taken from the starting_pos.
		cfg_level = &state_.replay_start();
	} else {
		// Fresh game, no snapshot available.
		cfg_level = &level_;
	}

	return cfg_level;
}

void connect_engine::import_user(const std::string& name, const bool observer,
	int side_taken)
{
	config user_data;
	user_data["name"] = name;
	import_user(user_data, observer, side_taken);
}

void connect_engine::import_user(const config& data, const bool observer,
	int side_taken)
{
	const std::string& username = data["name"];
	assert(!username.empty());

	connected_users_.insert(username);
	update_side_controller_options();

	if (observer) {
		return;
	}

	bool side_assigned = false;
	if (side_taken >= 0) {
		side_engines_[side_taken]->place_user(data, true);
		side_assigned = true;
	}

	// Check if user has a side(s) reserved for him.
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (side->current_player() == username && side->player_id().empty()) {
			side->place_user(data);

			side_assigned = true;
		}
	}

	if (side_taken < 0) {
		// If no sides were assigned for a user,
		// take a first available side.
		if (!side_assigned) {
			BOOST_FOREACH(side_engine_ptr side, side_engines_) {
				if (side->available_for_user(username) ||
					side->controller() == CNTR_LOCAL) {
					side->place_user(data);

					side_assigned = true;
					break;
				}
			}
		}
	}

	// Check if user has taken any sides, which should get control
	// over any other sides.
	BOOST_FOREACH(side_engine_ptr user_side, side_engines_) {
		if (user_side->player_id() == username) {
			BOOST_FOREACH(side_engine_ptr side, side_engines_) {
				if (!side->current_player().empty() &&
					side->player_id().empty() &&
					side->current_player() == user_side->cfg()["side"]) {

					side->place_user(data);
				}
			}
		}
	}
}

bool connect_engine::sides_available() const
{
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (side->available_for_user()) {
			return true;
		}
	}

	return false;
}

void connect_engine::update_level()
{
	DBG_MP << "updating level" << std::endl;

	level_.clear_children("side");

	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		level_.add_child("side", side->new_config());
	}
}

void connect_engine::update_and_send_diff(bool update_time_of_day)
{
	config old_level = level_;
	update_level();

	if (update_time_of_day) {
		// Set random start ToD.
		tod_manager tod_mng(level_, level_["turns"]);
	}

	config diff = level_.get_diff(old_level);
	if (!diff.empty()) {
		config scenario_diff;
		scenario_diff.add_child("scenario_diff", diff);
		network::send_data(scenario_diff, 0);
	}
}

bool connect_engine::can_start_game() const
{
	if (side_engines_.size() == 0) {
		return true;
	}

	// First check if all sides are ready to start the game.
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (!side->ready_for_start()) {
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
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (side->controller() != CNTR_EMPTY && side->allow_player()) {
			return true;
		}
	}

	return false;
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

	BOOST_FOREACH(const std::string& children_to_swap, get_children_to_swap())
		BOOST_FOREACH(const config& child, cfg_.child_range(children_to_swap))
			children.insert(std::pair<std::string, config>(children_to_swap, child));

	return children;
}

void side_engine::set_side_children(std::multimap<std::string, config> children)
{
	BOOST_FOREACH(const std::string& children_to_remove, get_children_to_swap())
				  cfg_.clear_children(children_to_remove);

	std::pair<std::string, config> child_map;

	BOOST_FOREACH(child_map, children)
				  cfg_.add_child(child_map.first, child_map.second);
}


void connect_engine::start_game(LOAD_USERS load_users)
{
	DBG_MP << "starting a new game" << std::endl;

    // Resolves the "random faction", "random gender" and "random message"
    // Must be done before shuffle sides, or some cases will cause errors
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		side->resolve_random();
	}

	// Shuffle sides (check preferences and if it is a re-loaded game).
	// Must be done after resolve_random() or shuffle sides, or they won't work.
	if (preferences::shuffle_sides() && !(level_.child("snapshot") &&
		level_.child("snapshot").child("side"))) {

		// Only playable sides should be shuffled.
		std::vector<int> playable_sides;
		BOOST_FOREACH(side_engine_ptr side, side_engines_) {
			if (side->allow_player()) {
				playable_sides.push_back(side->index());
			}
		}

		// Fisher-Yates shuffle.
		for (int i = playable_sides.size(); i > 1; i--)
		{
			int j_side = playable_sides[rand() % i];
			int i_side = playable_sides[i - 1];
			
			if (i_side == j_side) continue; //nothing to swap

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
	network::send_data(lock, 0);

	load_previous_sides_users(load_users);

	update_and_send_diff(true);

	save_reserved_sides_information();

	// Build the gamestate object after updating the level.
	level_to_gamestate(level_, state_);

	network::send_data(config("start_game"), 0);
}

void connect_engine::start_game_commandline(
	const commandline_options& cmdline_opts)
{
	DBG_MP << "starting a new game in commandline mode" << std::endl;

	typedef boost::tuple<unsigned int, std::string> mp_option;

	unsigned num = 0;
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		num++;

		// Set the faction, if commandline option is given.
		if (cmdline_opts.multiplayer_side) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_side) {

				if (option.get<0>() == num) {
					DBG_MP << "\tsetting side " << option.get<0>() <<
						"\tfaction: " << option.get<1>() << std::endl;

					side->set_faction_commandline(option.get<1>());
				}
			}
		}

		// Set the controller, if commandline option is given.
		if (cmdline_opts.multiplayer_controller) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_controller) {

				if (option.get<0>() == num) {
					DBG_MP << "\tsetting side " << option.get<0>() <<
						"\tfaction: " << option.get<1>() << std::endl;

					side->set_controller_commandline(option.get<1>());
				}
			}
		}

		// Set AI algorithm to RCA AI for all sides,
		// then override if commandline option was given.
		side->set_ai_algorithm("ai_default_rca");
		if (cmdline_opts.multiplayer_algorithm) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_algorithm) {

				if (option.get<0>() == num) {
					DBG_MP << "\tsetting side " << option.get<0>() <<
						"\tfaction: " << option.get<1>() << std::endl;

					side->set_ai_algorithm(option.get<1>());
				}
			}
		}

		// Finally, resolve "random faction",
		// "random gender" and "random message", if any remains unresolved.
		side->resolve_random();
	} // end top-level loop

	update_and_send_diff(true);

	// Update sides with commandline parameters.
	if (cmdline_opts.multiplayer_turns) {
		DBG_MP << "\tsetting turns: " << cmdline_opts.multiplayer_turns <<
			std::endl;
		level_["turns"] = *cmdline_opts.multiplayer_turns;
	}

	BOOST_FOREACH(config &side, level_.child_range("side"))
	{
		if (cmdline_opts.multiplayer_ai_config) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_ai_config) {

				if (option.get<0>() == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] <<
						"\tai_config: " << option.get<1>() << std::endl;

					side["ai_config"] = option.get<1>();
				}
			}
		}

		// Having hard-coded values here is undesirable,
		// but that's how it is done in the MP lobby
		// part of the code also.
		// Should be replaced by settings/constants in both places
		if (cmdline_opts.multiplayer_ignore_map_settings) {
			side["gold"] = 100;
			side["income"] = 1;
		}

		typedef boost::tuple<unsigned int, std::string, std::string>
			mp_parameter;

		if (cmdline_opts.multiplayer_parm) {
			BOOST_FOREACH(const mp_parameter& parameter,
				*cmdline_opts.multiplayer_parm) {

				if (parameter.get<0>() == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] << " " <<
						parameter.get<1>() << ": " << parameter.get<2>() <<
							std::endl;

					side[parameter.get<1>()] = parameter.get<2>();
				}
			}
		}
    }

	save_reserved_sides_information();

	// Build the gamestate object after updating the level
	level_to_gamestate(level_, state_);
	network::send_data(config("start_game"), 0);
}

std::pair<bool, bool> connect_engine::process_network_data(const config& data,
	const network::connection sock)
{
	std::pair<bool, bool> result(std::make_pair(false, true));

	if (data.child("leave_game")) {
		result.first = true;
		return result;
	}

	// A side has been dropped.
	if (!data["side_drop"].empty()) {
		unsigned side_drop = data["side_drop"].to_int() - 1;

		if (side_drop < side_engines_.size()) {
			side_engine_ptr side_to_drop = side_engines_[side_drop];

			// Remove user, whose side was dropped.
			connected_users_.erase(side_to_drop->player_id());
			update_side_controller_options();

			side_to_drop->reset();

			update_and_send_diff();

			return result;
		}
	}

	// A player is connecting to the game.
	if (!data["side"].empty()) {
		unsigned side_taken = data["side"].to_int() - 1;

		// Checks if the connecting user has a valid and unique name.
		const std::string name = data["name"];
		if (name.empty()) {
			config response;
			response["failed"] = true;
			network::send_data(response, sock);

			ERR_CF << "ERROR: No username provided with the side.\n";

			return result;
		}

		if (connected_users_.find(name) != connected_users_.end()) {
			 // TODO: Seems like a needless limitation
			 // to only allow one side per player.
			if (find_user_side_index_by_id(name) != -1) {
				config response;
				response["failed"] = true;
				response["message"] = "The nickname '" + name +
					"' is already in use.";
				network::send_data(response, sock);

				return result;
			} else {
				connected_users_.erase(name);
				update_side_controller_options();
				config observer_quit;
				observer_quit.add_child("observer_quit")["name"] = name;
				network::send_data(observer_quit, 0);
			}
		}

		// Assigns this user to a side.
		if (side_taken < side_engines_.size()) {
			if (!side_engines_[side_taken]->available_for_user(name)) {
				// This side is already taken.
				// Try to reassing the player to a different position.
				side_taken = 0;
				BOOST_FOREACH(side_engine_ptr s, side_engines_) {
					if (s->available_for_user()) {
						break;
					}

					side_taken++;
				}

				if (side_taken >= side_engines_.size()) {
					config response;
					response["failed"] = true;
					network::send_data(response, sock);

					config res;
					config& kick = res.add_child("kick");
					kick["username"] = data["name"];
					network::send_data(res, 0);

					update_and_send_diff();

					ERR_CF << "ERROR: Couldn't assign a side to '" <<
						name << "'\n";

					return result;
				}
			}

			LOG_CF << "client has taken a valid position\n";

			bool was_reserved = (side_engines_[side_taken]->controller() == CNTR_RESERVED);
			import_user(data, false, side_taken);
			side_engines_[side_taken]->set_waiting_to_choose_status(!was_reserved);

			update_and_send_diff();

			result.second = false;

			LOG_NW << "sent player data\n";
		} else {
			ERR_CF << "tried to take illegal side: " << side_taken << "\n";

			config response;
			response["failed"] = true;
			network::send_data(response, sock);
		}
	}


	if (const config& change_faction = data.child("change_faction")) {
		int side_taken = find_user_side_index_by_id(change_faction["name"]);
		if (side_taken != -1 || !first_scenario_) {
			bool was_waiting_for_faction = side_engines_[side_taken]->waiting_to_choose_faction();
			import_user(change_faction, false, side_taken);

			update_and_send_diff();
			if (was_waiting_for_faction && can_start_game()) {
				DBG_MP << "play party full sound" << std::endl;
				sound::play_UI_sound(game_config::sounds::party_full_bell);
			}
		}
	}

	if (const config& observer = data.child("observer")) {
		import_user(observer, true);
		update_and_send_diff();
	}

	if (const config& observer = data.child("observer_quit")) {
		const t_string& observer_name = observer["name"];
		if (!observer_name.empty()) {
			if ((connected_users_.find(observer_name) !=
				connected_users_.end()) &&
				(find_user_side_index_by_id(observer_name) != -1)) {

				connected_users_.erase(observer_name);
				update_side_controller_options();
				update_and_send_diff();
			}
		}
	}

	return result;
}

void connect_engine::process_network_error(network::error& error)
{
	// The problem isn't related to any specific connection and
	// it's a general error. So we should just re-throw the error.
	error.disconnect();
	throw network::error(error.message);
}

void connect_engine::process_network_connection(const network::connection sock)
{
	network::send_data(config("join_game"), 0);

	// If we are connected, send data to the connected host.
	send_level_data(sock);
}

int connect_engine::find_user_side_index_by_id(const std::string& id) const
{
	size_t i = 0;
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (side->player_id() == id) {
			break;
		}

		i++;
	}

	if (i >= side_engines_.size()) {
		return -1;
	}

	return i;
}

void connect_engine::send_level_data(const network::connection sock) const
{
	if (first_scenario_) {
		network::send_data(level_, sock);
	} else {
		config next_level;
		next_level.add_child("store_next_scenario", level_);
		network::send_data(next_level, sock);
	}
}

void connect_engine::save_reserved_sides_information()
{
	// Add information about reserved sides to the level config.
	// N.B. This information is needed only for a host player.
	std::map<std::string, std::string> side_users =
		utils::map_split(level_.child_or_empty("multiplayer")["side_users"]);
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		const std::string& save_id = side->save_id();
		const std::string& player_id = side->player_id();
		if (!save_id.empty() && !player_id.empty()) {
			side_users[save_id] = player_id;
		}
	}
	level_.child("multiplayer")["side_users"] = utils::join_map(side_users);
}

void connect_engine::load_previous_sides_users(LOAD_USERS load_users)
{
	if (load_users == NO_LOAD || first_scenario_) {
		return;
	}

	std::map<std::string, std::string> side_users =
		utils::map_split(level_.child("multiplayer")["side_users"]);
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		const std::string& save_id = side->save_id();
		if (side_users.find(save_id) != side_users.end()) {
			side->set_current_player(side_users[save_id]);

			if (load_users == RESERVE_USERS) {
				side->update_controller_options();
				side->set_controller(CNTR_RESERVED);
			} else if (load_users == FORCE_IMPORT_USERS) {
				import_user(side_users[save_id], false);
			}
		}
	}
}

void connect_engine::update_side_controller_options()
{
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		side->update_controller_options();
	}
}

side_engine::side_engine(const config& cfg, connect_engine& parent_engine,
	const int index) :
	cfg_(cfg),
	parent_(parent_engine),
	controller_(CNTR_NETWORK),
	current_controller_index_(0),
	controller_options_(),
	allow_player_(cfg["allow_player"].to_bool(true)),
	allow_changes_(cfg["allow_changes"].to_bool(true)),
	controller_lock_(cfg["controller_lock"].to_bool(
		parent_.force_lock_settings_)),
	index_(index),
	team_(0),
	color_(index),
	gold_(cfg["gold"].to_int(100)),
	income_(cfg["income"]),
	current_player_(cfg["current_player"]),
	player_id_(cfg["player_id"]),
	ai_algorithm_(),
	waiting_to_choose_faction_(allow_changes_ && allow_player_),
	chose_random_(cfg["chose_random"].to_bool(false)),
	flg_(parent_.era_factions_, cfg_, parent_.force_lock_settings_,
		parent_.params_.saved_game, color_)
{
	// Check if this side should give its control to some other side.
	const int side_cntr = cfg_["controller"].to_int(-1);
	if (side_cntr != -1) {
		current_player_ = lexical_cast<std::string>(side_cntr);
	}

	update_controller_options();

	// Tweak the controllers.
	if (cfg_["controller"] == "human_ai" ||
		cfg_["controller"] == "network_ai" || 
		(cfg_["controller"] == "network" &&  !allow_player_ && parent_.params_.saved_game)) { //this is a workaround for bug #21797

		cfg_["controller"] = "ai";
	}
	if (cfg_["controller"] == "null") {
		set_controller(CNTR_EMPTY);
	} else if (cfg_["controller"] == "ai") {
		set_controller(CNTR_COMPUTER);
	} else if (!current_player_.empty()) {
		// Reserve a side for "current_player", unless the side
		// is played by an AI.
		set_controller(CNTR_RESERVED);
	} else if (allow_player_ && !parent_.params_.saved_game) {
		set_controller(parent_.default_controller_);
	} else {
		// AI is the default.
		set_controller(CNTR_COMPUTER);
	}

	// Initialize team and color.
	unsigned team_name_index = 0;
	BOOST_FOREACH(const std::string& name, parent_.team_names_) {
		if (name == cfg["team_name"]) {
			break;
		}

		team_name_index++;
	}
	if (team_name_index >= parent_.team_names_.size()) {
		assert(!parent_.team_names_.empty());
		team_ = 0;
	} else {
		team_ = team_name_index;
	}
	if (!cfg["color"].empty()) {
		color_ = game_config::color_info(cfg["color"]).index() - 1;
	}

	// Initialize ai algorithm.
	if (const config& ai = cfg.child("ai")) {
		ai_algorithm_ = ai["ai_algorithm"].str();
	}
}

side_engine::~side_engine()
{
}

config side_engine::new_config() const
{
	config res = cfg_;

	// Save default "recruit" so that correct faction lists would be
	// initialized by flg_manager when the new side config is sent over network.
	res["default_recruit"] = cfg_["recruit"];

	// If the user is allowed to change type, faction, leader etc,
	// then import their new values in the config.
	if (!parent_.params_.saved_game) {
		// Merge the faction data to res.
		config faction = flg_.current_faction();
		faction.remove_attribute("id");
		res.append(faction);
		res["faction_name"] = res["name"];
	}

	if (!cfg_.has_attribute("side") || cfg_["side"].to_int() != index_ + 1) {
		res["side"] = index_ + 1;
	}
	// Side's current player is the player which is currently taken that side
	// or the one which is reserved to it.
	res["current_player"] = !player_id_.empty() ? player_id_ :
		(controller_ == CNTR_RESERVED ? current_player_ : "");
	res["controller"] = (res["current_player"] == preferences::login()) ?
		"human" : controller_names[controller_];

	if (player_id_.empty()) {
		std::string description;
		switch(controller_) {
		case CNTR_NETWORK:
			description = N_("(Vacant slot)");

			break;
		case CNTR_LOCAL:
			if (!parent_.params_.saved_game && !cfg_.has_attribute("save_id")) {
				res["save_id"] = preferences::login() + res["side"].str();
			}

			res["player_id"] = preferences::login() + res["side"].str();
			res["current_player"] = preferences::login();
			description = N_("Anonymous local player");

			break;
		case CNTR_COMPUTER: {
			if (!parent_.params_.saved_game && !cfg_.has_attribute("saved_id")) {
				res["save_id"] = "ai" + res["side"].str();
			}

			utils::string_map symbols;
			if (allow_player_) {
				const config& ai_cfg =
					ai::configuration::get_ai_config_for(ai_algorithm_);
				res.add_child("ai", ai_cfg);
				symbols["playername"] = ai_cfg["description"];
			} else {
				// Do not import default ai cfg here -
				// all is set by scenario config.
				symbols["playername"] = _("Computer Player");
			}

			symbols["side"] = res["side"].str();
			description = vgettext("$playername $side", symbols);

			break;
		}
		case CNTR_EMPTY:
			description = N_("(Empty slot)");
			res["no_leader"] = true;

			break;
		case CNTR_RESERVED: {
			utils::string_map symbols;
			symbols["playername"] = current_player_;
			description = vgettext("(Reserved for $playername)",symbols);

			break;
		}
		case CNTR_LAST:
		default:
			description = N_("(empty)");
			assert(false);

			break;
		} // end switch

		res["user_description"] = t_string(description, "wesnoth");
	} else {
		res["player_id"] = player_id_ + res["side"];
		if (!parent_.params_.saved_game && !cfg_.has_attribute("save_id")) {
			res["save_id"] = player_id_ + res["side"];
		}
		res["user_description"] = player_id_;
	}

	res["name"] = res["user_description"];
	res["allow_changes"] = !parent_.params_.saved_game && allow_changes_;
	res["chose_random"] = chose_random_;

	if (!parent_.params_.saved_game) {
		// Find a config where a default leader is and set a new type
		// and gender values for it.
		config* leader = &res;
		if (flg_.default_leader_cfg() != NULL) {
			BOOST_FOREACH(config& side_unit, res.child_range("unit")) {
				if (*flg_.default_leader_cfg() == side_unit) {
					leader = &side_unit;
					if (flg_.current_leader() != (*leader)["type"]) {
						// If a new leader type was selected from carryover,
						// make sure that we reset the leader.
						std::string leader_id = (*leader)["id"];
						leader->clear();
						if (!leader_id.empty()) {
							(*leader)["id"] = leader_id;
						}
					}

					break;
				}
			}
		}
		(*leader)["type"] = flg_.current_leader();
		(*leader)["gender"] = flg_.current_gender();

		res["team_name"] = parent_.team_names_[team_];
		res["user_team_name"] = parent_.user_team_names_[team_];
		res["allow_player"] = allow_player_;
		res["color"] = color_ + 1;
		res["gold"] = gold_;
		res["income"] = income_;

		if (!parent_.params_.use_map_settings || res["fog"].empty() ||
			(res["fog"] != "yes" && res["fog"] != "no")) {
			res["fog"] = parent_.params_.fog_game;
		}

		if (!parent_.params_.use_map_settings || res["shroud"].empty() ||
			(res["shroud"] != "yes" && res["shroud"] != "no")) {
			res["shroud"] = parent_.params_.shroud_game;
		}

		res["share_maps"] = parent_.params_.share_maps;
		res["share_view"] =  parent_.params_.share_view;

		if (!parent_.params_.use_map_settings || res["village_gold"].empty()) {
			res["village_gold"] = parent_.params_.village_gold;
		}
		if (!parent_.params_.use_map_settings ||
			res["village_support"].empty()) {
			res["village_support"] =
				lexical_cast<std::string>(parent_.params_.village_support);
		}

	}

	if (parent_.params_.use_map_settings && !parent_.params_.saved_game) {
		config trimmed = cfg_;

		BOOST_FOREACH(const std::string& attribute, attributes_to_trim) {
			trimmed.remove_attribute(attribute);
		}

		if (controller_ != CNTR_COMPUTER) {
			// Only override names for computer controlled players.
			trimmed.remove_attribute("user_description");
		}

		res.merge_with(trimmed);
	}

	return res;
}

bool side_engine::ready_for_start() const
{
	if (!allow_player_) {
		// Sides without players are always ready.
		return true;
	}

	if ((controller_ == mp::CNTR_COMPUTER) ||
		(controller_ == mp::CNTR_EMPTY) ||
		(controller_ == mp::CNTR_LOCAL)) {

		return true;
	}

	if (available_for_user()) return false; //if controller_ == CNTR_NETWORK and player_id_.empty(), this line will return false.

	if (controller_ == CNTR_NETWORK) {
		if (player_id_ == preferences::login() || !waiting_to_choose_faction_ || !allow_changes_) {
			return true;//the host is ready. a network player, who got a chance to choose faction if allowed, is also ready
		}
	}

	return false;
}

bool side_engine::available_for_user(const std::string& name) const
{
	if (controller_ == CNTR_NETWORK && player_id_.empty()) {
		// Side is free and waiting for user.
		return true;
	}
	if (controller_ == CNTR_RESERVED && name.empty()) {
		// Side is still available to someone.
		return true;
	}
	if (controller_ == CNTR_RESERVED && current_player_ == name) {
		// Side is available only for the player with specific name.
		return true;
	}

	return false;
}

bool side_engine::swap_sides_on_drop_target(const unsigned drop_target) {
	assert(drop_target < parent_.side_engines_.size());
	side_engine& target = *parent_.side_engines_[drop_target];

	const std::string target_id = target.player_id_;
	const mp::controller target_controller = target.controller_;
	const std::string target_ai = target.ai_algorithm_;

	if ((controller_lock_ || target.controller_lock_) &&
		(controller_options_ != target.controller_options_)) {

		return false;
	}

	target.ai_algorithm_ = ai_algorithm_;
	if (player_id_.empty()) {
		target.player_id_.clear();
		target.set_controller(controller_);
	} else {
		target.place_user(player_id_);
	}

	ai_algorithm_ = target_ai;
	if (target_id.empty()) {
		player_id_.clear();
		set_controller(target_controller);
	} else {
		place_user(target_id);
	}

	return true;
}

void side_engine::resolve_random()
{
	if (parent_.params_.saved_game) {
		return;
	}

	chose_random_ = flg_.is_random_faction();

	flg_.resolve_random();

	LOG_MP << "side " << (index_ + 1) << ": faction=" <<
		(flg_.current_faction())["name"] << ", leader=" <<
		flg_.current_leader() << ", gender=" << flg_.current_gender() << "\n";
}

void side_engine::reset()
{
	player_id_.clear();
	set_waiting_to_choose_status(false);
	set_controller(parent_.default_controller_);

	if (!parent_.params_.saved_game) {
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

	if (data["change_faction"].to_bool() && contains_selection) {
		// Network user's data carry information about chosen
		// faction, leader and genders.
		flg_.set_current_faction(data["faction"].str());
		flg_.set_current_leader(data["leader"].str());
		flg_.set_current_gender(data["gender"].str());
		waiting_to_choose_faction_ = false;
	}
}

void side_engine::update_controller_options()
{
	controller_options_.clear();

	// Default options.
	if (!parent_.local_players_only_) {
		add_controller_option(CNTR_NETWORK, _("Network Player"), "human");
	}
	add_controller_option(CNTR_LOCAL, _("Local Player"), "human");
	add_controller_option(CNTR_COMPUTER, _("Computer Player"), "ai");
	add_controller_option(CNTR_EMPTY, _("Empty"), "null");

	if (!current_player_.empty()) {
		add_controller_option(CNTR_RESERVED, _("Reserved"), "human");
	}

	// Connected users.
	add_controller_option(CNTR_LAST, _("--give--"), "human");
	BOOST_FOREACH(const std::string& user, parent_.connected_users_) {
		add_controller_option(parent_.default_controller_, user, "human");
	}

	update_current_controller_index();
}

void side_engine::update_current_controller_index()
{
	int i = 0;
	BOOST_FOREACH(const controller_option& option, controller_options_) {
		if (option.first == controller_) {
			current_controller_index_ = i;

			if (player_id_.empty() || player_id_ == option.second) {
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
	const mp::controller selected_cntr = controller_options_[selection].first;
	if (selected_cntr == CNTR_LAST) {
		return false;
	}

	// Check if user was selected. If so assign a side to him/her.
	// If not, make sure that no user is assigned to this side.
	if (selected_cntr == parent_.default_controller_ && selection != 0) {
		player_id_ = controller_options_[selection].second;
		set_waiting_to_choose_status(false);
	} else {
		player_id_.clear();
	}

	set_controller(selected_cntr);

	return true;
}

void side_engine::set_controller(mp::controller controller)
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

	if (controller_name == "ai") {
		set_controller(CNTR_COMPUTER);
	}
	if (controller_name == "null") {
		set_controller(CNTR_EMPTY);
	}

	player_id_.clear();
}

void side_engine::add_controller_option(mp::controller controller,
		const std::string& name, const std::string& controller_value)
{
	if (controller_lock_ && !cfg_["controller"].empty() &&
		cfg_["controller"] != controller_value) {

		return;
	}

	controller_options_.push_back(std::make_pair(controller, name));
}

} // end namespace mp
