/*
   Copyright (C) 2013 - 2015 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "connect_engine.hpp"

#include "ai/configuration.hpp"
#include "config_assign.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "multiplayer_ui.hpp"
#include "mp_game_utils.hpp"
#include "mt_rng.hpp"
#include "playcampaign.hpp"
#include "tod_manager.hpp"
#include "multiplayer_ui.hpp" // For get_color_string
#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <stdlib.h>
#include <ctime>

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_connect_engine("mp/connect/engine");
#define DBG_MP LOG_STREAM(debug, log_mp_connect_engine)
#define LOG_MP LOG_STREAM(info, log_mp_connect_engine)
#define WRN_MP LOG_STREAM(warn, log_mp_connect_engine)
#define ERR_MP LOG_STREAM(err, log_mp_connect_engine)

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)

namespace {

const std::string controller_names[] = {
	"human",
	"human",
	"ai",
	"null",
	"reserved"
};

const std::string attributes_to_trim[] = {
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
	"allow_changes"
};

}

namespace ng {

connect_engine::connect_engine(saved_game& state,
		const bool first_scenario, mp_campaign_info* campaign_info) :
	level_(),
	state_(state),
	params_(state.mp_settings()),
	default_controller_(campaign_info ? CNTR_NETWORK : CNTR_LOCAL),
	campaign_info_(campaign_info),
	first_scenario_(first_scenario),
	force_lock_settings_(),
	side_engines_(),
	era_factions_(),
	team_names_(),
	user_team_names_(),
	player_teams_()
{
	// Initial level config from the mp_game_settings.
	level_ = mp::initial_level_config(state_);
	if (level_.empty()) {
		return;
	}

	force_lock_settings_ = (!state.mp_settings().saved_game) && scenario()["force_lock_settings"].to_bool();

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

				team_name = "Team " +
					lexical_cast<std::string>(original_team_names.size());
			} else {
				team_name = "Team " + lexical_cast<std::string>(
					name_itor - original_team_names.begin() + 1);
			} // Note that the prefix "Team " is untranslatable, as team_name is not meant to be translated. This is needed so that the attribute
			  // is not interpretted as an int when reading from config, which causes bugs later.

			user_team_name = team_prefix + side_str;
		}

		if (add_team) {
			team_names_.push_back(params_.use_map_settings ? team_name :
				"Team " + side_str);
			user_team_names_.push_back(user_team_name.t_str().to_serialized());

			if (side["allow_player"].to_bool(true) || game_config::debug) {
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

	game_config::add_color_info(scenario());
	// Create side engines.
	int index = 0;
	BOOST_FOREACH(const config &s, sides) {
		side_engine_ptr new_side_engine(new side_engine(s, *this, index));
		side_engines_.push_back(new_side_engine);

		index++;
	}

	if(first_scenario_) {
		// Add host to the connected users list.
		import_user(preferences::login(), false);
	}
	else {
		// Add host but don't assign a side to him.
		import_user(preferences::login(), true);
		// Load reserved players information into the sides.
		load_previous_sides_users();
	}

	//actualy only updates the sides in the level.
	update_level();

	// If we are connected, send data to the connected host.
	send_level_data(0);
}

connect_engine::~connect_engine()
{
}

config* connect_engine::current_config() {
	if(config& s = scenario())
		return &s;
	else
		return NULL;
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
	if(campaign_info_) {
		connected_users_rw().insert(username);
	}
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
		if (side->reserved_for() == username && side->player_id().empty() && side->controller() != CNTR_COMPUTER) {
			side->place_user(data);

			side_assigned = true;
		}
	}

	// If no sides were assigned for a user,
	// take a first available side.
	if (side_taken < 0 && !side_assigned) {
		BOOST_FOREACH(side_engine_ptr side, side_engines_) {
			if (side->available_for_user(username) ||
				side->controller() == CNTR_LOCAL) {
					side->place_user(data);

					side_assigned = true;
					break;
			}
		}
	}

	// Check if user has taken any sides, which should get control
	// over any other sides.
	BOOST_FOREACH(side_engine_ptr user_side, side_engines_) {
		if(user_side->player_id() == username && !user_side->previous_save_id().empty()) {
			BOOST_FOREACH(side_engine_ptr side, side_engines_){
				if(side->player_id().empty() && side->previous_save_id() == user_side->previous_save_id()) {
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

	scenario().clear_children("side");

	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		scenario().add_child("side", side->new_config());
	}
}

void connect_engine::update_and_send_diff(bool /*update_time_of_day*/)
{
	config old_level = level_;
	update_level();

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


void connect_engine::start_game()
{
	DBG_MP << "starting a new game" << std::endl;

	// Resolves the "random faction", "random gender" and "random message"
	// Must be done before shuffle sides, or some cases will cause errors
	rand_rng::mt_rng rng; // Make an RNG for all the shuffling and random faction operations
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		std::vector<std::string> avoid_faction_ids;

		// If we aren't resolving random factions independently at random, calculate which factions should not appear for this side.
		if (params_.random_faction_mode != mp_game_settings::RANDOM_FACTION_MODE::DEFAULT) {
			BOOST_FOREACH(side_engine_ptr side2, side_engines_) {
				if (!side2->flg().is_random_faction()) {
					switch(params_.random_faction_mode.v) {
						case mp_game_settings::RANDOM_FACTION_MODE::NO_MIRROR:
							avoid_faction_ids.push_back(side2->flg().current_faction()["id"].str());
							break;
						case mp_game_settings::RANDOM_FACTION_MODE::NO_ALLY_MIRROR:
							if (side2->team() == side->team()) {// TODO: When the connect engines are fixed to allow multiple teams, this should be changed to "if side1 and side2 are allied, i.e. their list of teams has nonempty intersection"
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
	if (state_.mp_settings().shuffle_sides && !force_lock_settings_ && !(level_.child("snapshot") && level_.child("snapshot").child("side"))) {

		// Only playable sides should be shuffled.
		std::vector<int> playable_sides;
		BOOST_FOREACH(side_engine_ptr side, side_engines_) {
			if (side->allow_player() && side->allow_shuffle()) {
				playable_sides.push_back(side->index());
			}
		}

		// Fisher-Yates shuffle.
		for (int i = playable_sides.size(); i > 1; i--)
		{
			int j_side = playable_sides[rng.get_next_random() % i];
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

	update_and_send_diff(true);

	save_reserved_sides_information();

	// Build the gamestate object after updating the level.
	mp::level_to_gamestate(level_, state_);

	network::send_data(config("start_game"), 0);
}

void connect_engine::start_game_commandline(
	const commandline_options& cmdline_opts)
{
	DBG_MP << "starting a new game in commandline mode" << std::endl;

	typedef boost::tuple<unsigned int, std::string> mp_option;

	rand_rng::mt_rng rng;

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
		side->resolve_random(rng);
	} // end top-level loop

	update_and_send_diff(true);

	// Update sides with commandline parameters.
	if (cmdline_opts.multiplayer_turns) {
		DBG_MP << "\tsetting turns: " << *cmdline_opts.multiplayer_turns <<
			std::endl;
		scenario()["turns"] = *cmdline_opts.multiplayer_turns;
	}

	BOOST_FOREACH(config &side, scenario().child_range("side"))
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
	mp::level_to_gamestate(level_, state_);
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
			connected_users_rw().erase(side_to_drop->player_id());
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

			ERR_CF << "ERROR: No username provided with the side." << std::endl;

			return result;
		}

		if (connected_users().find(name) != connected_users().end()) {
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
				connected_users_rw().erase(name);
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

			import_user(data, false, side_taken);
			update_and_send_diff();

			// Wait for them to choose faction if allowed.
			side_engines_[side_taken]->
				set_waiting_to_choose_status(side_engines_[side_taken]->
					allow_changes());
			LOG_MP << "waiting to choose status = " <<
				side_engines_[side_taken]->allow_changes() << std::endl;
			result.second = false;

			LOG_NW << "sent player data\n";
		} else {
			ERR_CF << "tried to take illegal side: " << side_taken << std::endl;

			config response;
			response["failed"] = true;
			network::send_data(response, sock);
		}
	}


	if (const config& change_faction = data.child("change_faction")) {
		int side_taken = find_user_side_index_by_id(change_faction["name"]);
		if (side_taken != -1 || !first_scenario_) {
			import_user(change_faction, false, side_taken);
			update_and_send_diff();
		}
	}

	if (const config& observer = data.child("observer")) {
		import_user(observer, true);
		update_and_send_diff();
	}

	if (const config& observer = data.child("observer_quit")) {
		const t_string& observer_name = observer["name"];
		if (!observer_name.empty()) {
			if ((connected_users().find(observer_name) != connected_users().end()) &&
				(find_user_side_index_by_id(observer_name) != -1)) {

				connected_users_rw().erase(observer_name);
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
	// Send initial information.
	if (first_scenario_) {
		network::send_data(config_of
			("create_game", config_of
				("name", params_.name)
				("password", params_.password)
			)
		);
		network::send_data(level_, sock);
	} else {
		network::send_data(config_of("update_game", config()), 0);
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

void connect_engine::load_previous_sides_users()
{
	std::map<std::string, std::string> side_users =
		utils::map_split(level_.child("multiplayer")["side_users"]);
	std::set<std::string> names;
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		const std::string& save_id = side->previous_save_id();
		if (side_users.find(save_id) != side_users.end()) {
			side->set_reserved_for(side_users[save_id]);

			if (side->controller() != CNTR_COMPUTER) {
				side->set_controller(CNTR_RESERVED);
				names.insert(side_users[save_id]);
			}
			side->update_controller_options();

		}
	}
	//Do this in an extra loop to make sure we import each user only once.
	BOOST_FOREACH(const std::string& name, names)
	{
		if (connected_users().find(name) != connected_users().end() || !campaign_info_) {
			import_user(name, false);
		}
	}
}

void connect_engine::update_side_controller_options()
{
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		side->update_controller_options();
	}
}


const std::set<std::string>& connect_engine::connected_users() const
{
	if(campaign_info_) {
		return campaign_info_->connected_players;
	}
	else {
		static std::set<std::string> empty;
		return empty;
	}
}
std::set<std::string>& connect_engine::connected_users_rw()
{
	assert(campaign_info_);
	return campaign_info_->connected_players;
}

side_engine::side_engine(const config& cfg, connect_engine& parent_engine,
	const int index) :
	cfg_(cfg),
	parent_(parent_engine),
	controller_(CNTR_NETWORK),
	current_controller_index_(0),
	controller_options_(),
	allow_player_(cfg["allow_player"].to_bool(true)),
	controller_lock_(cfg["controller_lock"].to_bool(
		parent_.force_lock_settings_) && parent_.params_.use_map_settings),
	index_(index),
	team_(0),
	color_(std::min(index, gamemap::MAX_PLAYERS - 1)),
	gold_(cfg["gold"].to_int(100)),
	income_(cfg["income"]),
	reserved_for_(cfg["current_player"]),
	player_id_(),
	ai_algorithm_(),
	chose_random_(cfg["chose_random"].to_bool(false)),
	disallow_shuffle_(cfg["disallow_shuffle"].to_bool(false)),
	flg_(parent_.era_factions_, cfg_, parent_.force_lock_settings_,
		parent_.params_.use_map_settings, parent_.params_.saved_game),
	allow_changes_(!parent_.params_.saved_game && !(flg_.choosable_factions().size() == 1 && flg_.choosable_leaders().size() == 1 && flg_.choosable_genders().size() == 1)),
	waiting_to_choose_faction_(allow_changes_),
	custom_color_()
{
	// Check if this side should give its control to some other side.
	const size_t side_cntr_index = cfg_["controller"].to_int(-1) - 1;
	if (side_cntr_index < parent_.side_engines().size()) {
		// Remove this attribute to avoid locking side
		// to non-existing controller type.
		cfg_.remove_attribute("controller");

		cfg_["previous_save_id"] = parent_.side_engines()[side_cntr_index]->previous_save_id();
		ERR_MP << "contoller=<number> is deperecated\n";
	}
	if(!parent_.params_.saved_game && cfg_["save_id"].str().empty()) {
		assert(cfg_["id"].empty()); // we already setted "save_id" to "id" if "id" existed.
		std::ostringstream save_id;
		//generating a save_id that is unique for this campaign playthrough.
		save_id << "save_id_" << time(NULL) << "_" << index;
		cfg_["save_id"] = save_id.str();
	}

	update_controller_options();

	// Tweak the controllers.
	if (cfg_["controller"] == "network_ai" ||
		(parent_.state_.classification().campaign_type == game_classification::CAMPAIGN_TYPE::SCENARIO && cfg_["controller"].blank()))
	{
		cfg_["controller"] = "ai";
	}
	//this is a workaround for bug #21797
	if(cfg_["controller"] == "network" &&  !allow_player_ && parent_.params_.saved_game)
	{
		WRN_MP << "Found a side controlled by a network player with allow_player=no" << std::endl;
		cfg_["controller"] = "ai";
	}

	if(cfg_["controller"] != "human" && cfg_["controller"] != "ai" && cfg_["controller"] != "null") {
		//an invalid contoller type was specified. Remove it to prevent asertion failures later.
		cfg_.remove_attribute("controller");
	}

	if (cfg_["controller"] == "null") {
		set_controller(CNTR_EMPTY);
	} else if (cfg_["controller"] == "ai") {
		set_controller(CNTR_COMPUTER);
	} else if (!reserved_for_.empty()) {
		// Reserve a side for "current_player", unless the side
		// is played by an AI.
		set_controller(CNTR_RESERVED);
	} else if (allow_player_) {
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
		WRN_MP << "In side_engine constructor: Could not find my team_name " << cfg["team_name"] << " among the mp connect engine's list of team names. I am being assigned to the first team. This may indicate a bug!" << std::endl;
	} else {
		team_ = team_name_index;
	}
	if (!cfg["color"].empty()) {
		if(cfg["color"].to_int()) {
			color_ = cfg["color"].to_int() - 1;
		}
		else {
			custom_color_ = cfg["color"].str();
			color_ = 0;
		}
	}

	// Initialize ai algorithm.
	if (const config& ai = cfg.child("ai")) {
		ai_algorithm_ = ai["ai_algorithm"].str();
	}
}

side_engine::~side_engine()
{
}

std::string side_engine::user_description() const
{
	switch(controller_)
	{
	case CNTR_LOCAL:
		return N_("Anonymous player");
	case CNTR_COMPUTER:
		if (allow_player_) {
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

	// Save default "recruit" so that correct faction lists would be
	// initialized by flg_manager when the new side config is sent over network.
	res["default_recruit"] = cfg_["recruit"].str();

	// If the user is allowed to change type, faction, leader etc,
	// then import their new values in the config.
	if (!parent_.params_.saved_game) {
		// Merge the faction data to res.
		config faction = flg_.current_faction();
		res["faction_name"] = faction["name"];
		faction.remove_attribute("id");
		faction.remove_attribute("name");
		faction.remove_attribute("image");
		faction.remove_attribute("flag_rgb");
		res.append(faction);
	}

	if (!cfg_.has_attribute("side") || cfg_["side"].to_int() != index_ + 1) {
		res["side"] = index_ + 1;
	}

	res["controller"] = controller_names[controller_];
	// the hosts recieves the serversided controller tweaks after the start event, but
	// for mp sync it's very important that the controller types are correct
	// during the start/prestart event (otherwse random unit creation during prestart fails).
	res["is_local"] = player_id_ == preferences::login() || controller_ == CNTR_COMPUTER;

	std::string desc = user_description();
	if(!desc.empty()) {
		res["user_description"] = t_string(desc, "wesnoth");
		desc = vgettext(
			"$playername $side",
			boost::assign::map_list_of
				("playername", _(desc.c_str()))
				("side", res["side"].str())
		);
	} else if (!player_id_.empty()) {
		desc = player_id_;
	}
	if(res["name"].str().empty() && !desc.empty()) {
		res["name"] = desc;
	}

	assert(controller_ != CNTR_LAST);
	if(controller_ == CNTR_COMPUTER && allow_player_) {
		// Do not import default ai cfg otherwise - all is set by scenario config.
		res.add_child("ai", ai::configuration::get_ai_config_for(ai_algorithm_));
	}

	if(controller_ == CNTR_EMPTY) {
		res["no_leader"] = true;
	}

	// Side's "current_player" is the player which is currently taken that side
	// or the one which is reserved to it.
	// "player_id" is the id of the client who controlls that side,
	// that always the host for Local players and AIs
	// any always empty for free/reserved sides or null controlled sides.
	// especialy you can use !res["player_id"].empty() to check whether a side is already taken.
	assert(!preferences::login().empty());
	if(controller_ == CNTR_LOCAL) {
		res["player_id"] = preferences::login();
		res["current_player"] = preferences::login();
	} else if(controller_ == CNTR_RESERVED) {
		res.remove_attribute("player_id");
		res["current_player"] = reserved_for_;
	} else if(controller_ == CNTR_COMPUTER) {
		//TODO what is the content of player_id_ here ?
		res["current_player"] = desc;
		res["player_id"] = preferences::login();
	} else if(!player_id_.empty()) {
		res["player_id"] = player_id_;
		res["current_player"] = player_id_;
	}

	res["allow_changes"] = allow_changes_;
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
		res["color"] = get_color(color_);
		res["gold"] = gold_;
		res["income"] = income_;
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

	if ((controller_ == CNTR_COMPUTER) ||
		(controller_ == CNTR_EMPTY) ||
		(controller_ == CNTR_LOCAL)) {

		return true;
	}

	if (available_for_user()) {
		// If controller_ == CNTR_NETWORK and player_id_.empty().
		return false;
	}

	if (controller_ == CNTR_NETWORK) {
		if (player_id_ == preferences::login() || !waiting_to_choose_faction_ || !allow_changes_) {
			// The host is ready. A network player, who got a chance
			// to choose faction if allowed, is also ready.
			return true;
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
	if (controller_ == CNTR_RESERVED && reserved_for_ == name) {
		// Side is available only for the player with specific name.
		return true;
	}

	return false;
}

bool side_engine::swap_sides_on_drop_target(const unsigned drop_target) {
	assert(drop_target < parent_.side_engines_.size());
	side_engine& target = *parent_.side_engines_[drop_target];

	const std::string target_id = target.player_id_;
	const ng::controller target_controller = target.controller_;
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

void side_engine::resolve_random(rand_rng::mt_rng & rng, const std::vector<std::string> & avoid_faction_ids)
{
	if (parent_.params_.saved_game) {
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
	}

	waiting_to_choose_faction_ = false;
}

void side_engine::update_controller_options()
{
	controller_options_.clear();

	// Default options.
	if (parent_.campaign_info_) {
		add_controller_option(CNTR_NETWORK, _("Network Player"), "human");
	}
	add_controller_option(CNTR_LOCAL, _("Local Player"), "human");
	add_controller_option(CNTR_COMPUTER, _("Computer Player"), "ai");
	add_controller_option(CNTR_EMPTY, _("Empty"), "null");

	if (!reserved_for_.empty()) {
		add_controller_option(CNTR_RESERVED, _("Reserved"), "human");
	}

	// Connected users.
	add_controller_option(CNTR_LAST, _("--give--"), "human");
	BOOST_FOREACH(const std::string& user, parent_.connected_users()) {
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
	const ng::controller selected_cntr = controller_options_[selection].first;
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

	if (controller_name == "ai") {
		set_controller(CNTR_COMPUTER);
	}
	if (controller_name == "null") {
		set_controller(CNTR_EMPTY);
	}

	player_id_.clear();
}

void side_engine::add_controller_option(ng::controller controller,
		const std::string& name, const std::string& controller_value)
{
	if (controller_lock_ && !cfg_["controller"].empty() &&
		cfg_["controller"] != controller_value) {

		return;
	}

	controller_options_.push_back(std::make_pair(controller, name));
}

std::vector<std::string> side_engine::get_colors() const
{
	std::vector<std::string> res;
	for (int i = 0; i < num_colors(); ++i) {
		res.push_back(mp::get_color_string(get_color(i)));
	}
	return res;
}

std::string side_engine::get_color(int index) const
{
	if(index == -1) {
		index = color();
	}
	if(!custom_color_.empty()) {
		if(index == 0) {
			return custom_color_;
		}
		index -= 1;
	}
	return lexical_cast<std::string>(index + 1);
}

int side_engine::num_colors() const
{
	return custom_color_.empty() ? gamemap::MAX_PLAYERS : gamemap::MAX_PLAYERS + 1;
}

} // end namespace ng
