/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file gamestatus.cpp
 * Maintain status of a game, load&save games.
 */

#include "global.hpp"
#include "config.hpp"

#include "gamestatus.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "game_preferences.hpp"
#include "replay.hpp"
#include "statistics.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "map.hpp"
#include "pathfind.hpp"

#include <boost/bind.hpp>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

game_classification::game_classification():
	label(),
	parent(),
	version(),
	campaign_type(),
	campaign_define(),
	campaign_xtra_defines(),
	campaign(),
	history(),
	abbrev(),
	scenario(),
	next_scenario(),
	completion(),
	end_text(),
	end_text_duration(),
	difficulty("NORMAL")
	{}

game_classification::game_classification(const config& cfg):
	label(cfg["label"]),
	parent(cfg["parent"]),
	version(cfg["version"]),
	campaign_type(cfg["campaign_type"].empty() ? "scenario" : cfg["campaign_type"]),
	campaign_define(cfg["campaign_define"]),
	campaign_xtra_defines(utils::split(cfg["campaign_extra_defines"])),
	campaign(cfg["campaign"]),
	history(cfg["history"]),
	abbrev(cfg["abbrev"]),
	scenario(cfg["scenario"]),
	next_scenario(cfg["next_scenario"]),
	completion(cfg["completion"]),
	end_text(cfg["end_text"]),
	end_text_duration(lexical_cast_default<unsigned int>(cfg["end_text_duration"])),
	difficulty(cfg["difficulty"].empty() ? "NORMAL" : cfg["difficulty"])
	{}

game_classification::game_classification(const game_classification& gc):
	label(gc.label),
	parent(gc.parent),
	version(gc.version),
	campaign_type(gc.campaign_type),
	campaign_define(gc.campaign_define),
	campaign_xtra_defines(gc.campaign_xtra_defines),
	campaign(gc.campaign),
	history(gc.history),
	abbrev(gc.abbrev),
	scenario(gc.scenario),
	next_scenario(gc.next_scenario),
	completion(gc.completion),
	end_text(gc.end_text),
	end_text_duration(gc.end_text_duration),
	difficulty(gc.difficulty)
{
}

config game_classification::to_config() const
{
	config cfg;

	cfg["label"] = label;
	cfg["parent"] = parent;
	cfg["version"] = game_config::version;
	cfg["campaign_type"] = campaign_type;
	cfg["campaign_define"] = campaign_define;
	cfg["campaign_extra_defines"] = utils::join(campaign_xtra_defines);
	cfg["campaign"] = campaign;
	cfg["history"] = history;
	cfg["abbrev"] = abbrev;
	cfg["scenario"] = scenario;
	cfg["next_scenario"] = next_scenario;
	cfg["completion"] = completion;
	cfg["end_text"] = end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(end_text_duration);
	cfg["difficulty"] = difficulty;

	return cfg;
}

#ifdef __UNUSED__
std::string generate_game_uuid()
{
	struct timeval ts;
	std::stringstream uuid;
	gettimeofday(&ts, NULL);

	uuid << preferences::login() << "@" << ts.tv_sec << "." << ts.tv_usec;

	return uuid.str();
}
#endif

game_state::game_state()  :
		scoped_variables(),
		wml_menu_items(),
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(map_location::null_location),
		rng_(),
		variables(),
		temporaries(),
		generator_setter(&recorder),
		classification_(),
		mp_settings_()
		{}

void write_players(game_state& gamestate, config& cfg, const bool use_snapshot, const bool merge_side)
{
	// If there is already a player config available it means we are loading
	// from a savegame. Don't do anything then, the information is already there
	config::child_itors player_cfg = cfg.child_range("player");
	if (player_cfg.first != player_cfg.second)
		return;

	config *source = NULL;
	if (use_snapshot) {
		source = &gamestate.snapshot;
	} else {
		source = &gamestate.starting_pos;
	}

	if (merge_side) {
		//merge sides/players from starting pos with the scenario cfg
		std::vector<std::string> tags;
		tags.push_back("side");
		tags.push_back("player"); //merge [player] tags for backwards compatibility of saves

		foreach (const std::string& side_tag, tags) {
			foreach (config* carryover_side, source->get_children(side_tag)) {
				config *scenario_side = NULL;

				if (config& c = cfg.find_child("side", "save_id", (*carryover_side)["save_id"])) {
					scenario_side = &c;
				} else if (config& c = cfg.find_child("side", "id", (*carryover_side)["save_id"])) {
					scenario_side = &c;
				}

				if (scenario_side != NULL) {
					//we have a matching side in the current scenario

					//sort carryover gold
					std::string gold = (*scenario_side)["gold"];
					if(gold.empty())
						gold = "100";
					int ngold = lexical_cast_default<int>(gold);
					int player_gold = lexical_cast_default<int>((*carryover_side)["gold"]);
					if(utils::string_bool((*carryover_side)["gold_add"])) {
						ngold +=  player_gold;
					} else if(player_gold >= ngold) {
						ngold = player_gold;
					}
					(*carryover_side)["gold"] = str_cast<int>(ngold);
					if ( !(*scenario_side)["gold_add"].empty() ) {
						(*carryover_side)["gold_add"] = (*scenario_side)["gold_add"];
					}
					//merge player information into the scenario cfg
					(*scenario_side)["save_id"] = (*carryover_side)["save_id"];
					(*scenario_side)["gold"] = str_cast<int>(ngold);
					(*scenario_side)["gold_add"] = (*carryover_side)["gold_add"];
					if (!(*carryover_side)["previous_recruits"].empty()) {
						(*scenario_side)["previous_recruits"]	= (*carryover_side)["previous_recruits"];
					} else {
						(*scenario_side)["previous_recruits"]	= (*carryover_side)["can_recruit"];
					}
					(*scenario_side)["name"] = (*carryover_side)["name"];
					//add recallable units
					foreach (const config* u, carryover_side->get_children("unit")) {
						scenario_side->add_child("unit", *u);
					}

				} else {
					//no matching side in the current scenario, we add the persistent information in a [player] tag
					cfg.add_child("player", (*carryover_side));
				}
			}
	}

	} else {
		foreach(const config* snapshot_side, source->get_children("side")) {
			//take all side tags and add them as players (assuming they only contain carryover information)
			cfg.add_child("player", *snapshot_side);
		}
	}
}

game_state::game_state(const config& cfg, bool show_replay) :
		scoped_variables(),
		wml_menu_items(),
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(map_location::null_location),
		rng_(cfg),
		variables(),
		temporaries(),
		generator_setter(&recorder),
		classification_(cfg),
		mp_settings_(cfg)
{
	n_unit::id_manager::instance().set_save_id(lexical_cast_default<size_t>(cfg["next_underlying_unit_id"],0));
	log_scope("read_game");

	const config &snapshot = cfg.child("snapshot");
	const config &replay_start = cfg.child("replay_start");
	// We're loading a snapshot if we have it and the user didn't request a replay.
	bool load_snapshot = !show_replay && snapshot && !snapshot.empty();

	if (load_snapshot) {
		this->snapshot = snapshot;

		rng_.seed_random(lexical_cast_default<unsigned>(snapshot["random_calls"]));
	} else {
		assert(replay_start);
	}

	LOG_NG << "scenario: '" << classification_.scenario << "'\n";
	LOG_NG << "next_scenario: '" << classification_.next_scenario << "'\n";

	//priority of populating wml variables:
	//snapshot -> replay_start -> root
	if (load_snapshot) {
		if (const config &vars = snapshot.child("variables")) {
			set_variables(vars);
		} else if (const config &vars = cfg.child("variables")) {
			set_variables(vars);
		}
	}
	else if (const config &vars = replay_start.child("variables")) {
		set_variables(vars);
	}
	else if (const config &vars = cfg.child("variables")) {
		set_variables(vars);
	}
	set_menu_items(cfg.child_range("menu_item"));

	if (const config &replay = cfg.child("replay")) {
		replay_data = replay;
	}

	if (replay_start) {
		starting_pos = replay_start;
		//This is a quick hack to make replays for campaigns work again:
		//The [player] information needs to be stored somewhere within the gamestate,
		//because we need it later on when creating the replay savegame.
		//We therefore put it inside the starting_pos, so it doesn't get lost.
		//See also playcampaign::play_game, where after finishing the scenario the replay
		//will be saved.
		if(!starting_pos.empty()) {
			foreach (const config &p, cfg.child_range("player")) {
				config& cfg_player = starting_pos.add_child("player");
				cfg_player.merge_with(p);
			}
		}
	}

	if (const config &stats = cfg.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}
}

void game_state::write_snapshot(config& cfg) const
{
	log_scope("write_game");
	cfg["label"] = classification_.label;
	cfg["history"] = classification_.history;
	cfg["abbrev"] = classification_.abbrev;
	cfg["version"] = game_config::version;

	cfg["scenario"] = classification_.scenario;
	cfg["next_scenario"] = classification_.next_scenario;

	cfg["completion"] = classification_.completion;

	cfg["campaign"] = classification_.campaign;
	cfg["campaign_type"] = classification_.campaign_type;
	cfg["difficulty"] = classification_.difficulty;

	cfg["campaign_define"] = classification_.campaign_define;
	cfg["campaign_extra_defines"] = utils::join(classification_.campaign_xtra_defines);
	cfg["next_underlying_unit_id"] = lexical_cast<std::string>(n_unit::id_manager::instance().get_save_id());

	cfg["random_seed"] = lexical_cast<std::string>(rng_.get_random_seed());
	cfg["random_calls"] = lexical_cast<std::string>(rng_.get_random_calls());

	cfg["end_text"] = classification_.end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(classification_.end_text_duration);

	cfg.add_child("variables", variables);

	for(std::map<std::string, wml_menu_item *>::const_iterator j=wml_menu_items.begin();
	    j!=wml_menu_items.end(); ++j) {
		config new_cfg;
		new_cfg["id"]=j->first;
		new_cfg["image"]=j->second->image;
		new_cfg["description"]=j->second->description;
		new_cfg["needs_select"]= (j->second->needs_select) ? "yes" : "no";
		if(!j->second->show_if.empty())
			new_cfg.add_child("show_if", j->second->show_if);
		if(!j->second->filter_location.empty())
			new_cfg.add_child("filter_location", j->second->filter_location);
		if(!j->second->command.empty())
			new_cfg.add_child("command", j->second->command);
		cfg.add_child("menu_item", new_cfg);
	}
}

void extract_summary_from_config(config& cfg_save, config& cfg_summary)
{
	const config &cfg_snapshot = cfg_save.child("snapshot");
	const config &cfg_replay_start = cfg_save.child("replay_start");

	const config &cfg_replay = cfg_save.child("replay");
	const bool has_replay = cfg_replay && !cfg_replay.empty();
	const bool has_snapshot = cfg_snapshot && cfg_snapshot.child("side");

	cfg_summary["replay"] = has_replay ? "yes" : "no";
	cfg_summary["snapshot"] = has_snapshot ? "yes" : "no";

	cfg_summary["label"] = cfg_save["label"];
	cfg_summary["parent"] = cfg_save["parent"];
	cfg_summary["campaign_type"] = cfg_save["campaign_type"];
	cfg_summary["scenario"] = cfg_save["scenario"];
	cfg_summary["campaign"] = cfg_save["campaign"];
	cfg_summary["difficulty"] = cfg_save["difficulty"];
	cfg_summary["version"] = cfg_save["version"];
	cfg_summary["corrupt"] = "";

	if(has_snapshot) {
		cfg_summary["turn"] = cfg_snapshot["turn_at"];
		if (cfg_snapshot["turns"] != "-1") {
			cfg_summary["turn"] = cfg_summary["turn"].str() + "/" + cfg_snapshot["turns"].str();
		}
	}

	// Find the first human leader so we can display their icon in the load menu.

	/** @todo Ideally we should grab all leaders if there's more than 1 human player? */
	std::string leader;
	std::string leader_image;

	//foreach (const config &p, cfg_save.child_range("player"))
	//{
	//	if (utils::string_bool(p["canrecruit"], false)) {
	//		leader = p["save_id"];
	//	}
	//}

	bool shrouded = false;

	//if (!leader.empty())
	//{
		if (const config &snapshot = *(has_snapshot ? &cfg_snapshot : &cfg_replay_start))
		{
			foreach (const config &side, snapshot.child_range("side"))
			{
				if (side["controller"] != "human") {
					continue;
				}

				if (utils::string_bool(side["shroud"])) {
					shrouded = true;
				}

				if (side["canrecruit"] == "yes")
				{
						leader = side["id"];
						leader_image = side["image"];
						break;
				}

				foreach (const config &u, side.child_range("unit"))
				{
					if (utils::string_bool(u["canrecruit"], false)) {
						leader = u["id"];
						leader_image = u["image"];
						break;
					}
				}
			}
		}
	//}

	cfg_summary["leader"] = leader;
	cfg_summary["leader_image"] = leader_image;
	cfg_summary["map_data"] = "";

	if(!shrouded) {
		if(has_snapshot) {
			if (!cfg_snapshot.find_child("side", "shroud", "yes")) {
				cfg_summary["map_data"] = cfg_snapshot["map_data"];
			}
		} else if(has_replay) {
			if (!cfg_replay_start.find_child("side","shroud","yes")) {
				cfg_summary["map_data"] = cfg_replay_start["map_data"];
			}
		}
	}
}

t_string& game_state::get_variable(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_SCALAR).as_scalar();
}

const t_string& game_state::get_variable_const(const std::string& key) const
{
	variable_info to_get(key, false, variable_info::TYPE_SCALAR);
	if(!to_get.is_valid) {
			t_string& to_return = temporaries[key];
			if (key.size() > 7 && key.substr(key.size()-7) == ".length") {
				// length is a special attribute, so guarantee its correctness
				to_return = "0";
			}
			return to_return;
	}
	return to_get.as_scalar();
}

config& game_state::get_variable_cfg(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_CONTAINER).as_container();
}

variable_info::array_range game_state::get_variable_cfgs(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_ARRAY).as_array();
}

void game_state::set_variable(const std::string& key, const t_string& value)
{
	get_variable(key) = value;
}

config& game_state::add_variable_cfg(const std::string& key, const config& value)
{
	variable_info to_add(key, true, variable_info::TYPE_ARRAY);
	return to_add.vars->add_child(to_add.key, value);
}

void game_state::clear_variable_cfg(const std::string& varname)
{
	variable_info to_clear(varname, false, variable_info::TYPE_CONTAINER);
	if(!to_clear.is_valid) return;
	if(to_clear.explicit_index) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
	}
}

void game_state::clear_variable(const std::string& varname)
{
	variable_info to_clear(varname, false);
	if(!to_clear.is_valid) return;
	if(to_clear.explicit_index) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
		to_clear.vars->remove_attribute(to_clear.key);
	}
}

static void clear_wmi(std::map<std::string, wml_menu_item*>& gs_wmi) {
	std::map<std::string, wml_menu_item*>::iterator itor = gs_wmi.begin();
	for(itor = gs_wmi.begin(); itor != gs_wmi.end(); ++itor) {
		delete itor->second;
	}
	gs_wmi.clear();
}

game_state::game_state(const game_state& state) :
	/* default construct everything to silence compiler warnings. */
	variable_set(),
	scoped_variables(),
	wml_menu_items(),
	replay_data(),
	starting_pos(),
	snapshot(),
	last_selected(),
	rng_(),
	variables(),
	temporaries(),
	generator_setter(&recorder),
	classification_(),
	mp_settings_()
{
	*this = state;
}

game_state& game_state::operator=(const game_state& state)
{
	if(this == &state) {
		return *this;
	}

	rng_ = state.rng_;
	scoped_variables = state.scoped_variables;
	classification_ = game_classification(state.classification());
	mp_settings_ = mp_game_settings(state.mp_settings());

	clear_wmi(wml_menu_items);
	std::map<std::string, wml_menu_item*>::const_iterator itor;
	for (itor = state.wml_menu_items.begin(); itor != state.wml_menu_items.end(); ++itor) {
		wml_menu_item*& mref = wml_menu_items[itor->first];
		mref = new wml_menu_item(*(itor->second));
	}

	replay_data = state.replay_data;
	starting_pos = state.starting_pos;
	snapshot = state.snapshot;
	last_selected = state.last_selected;
	set_variables(state.get_variables());

	return *this;
}

game_state::~game_state() {
	clear_wmi(wml_menu_items);
}

void game_state::set_variables(const config& vars) {
	if(!variables.empty()) {
		WRN_NG << "clobbering the game_state variables\n";
		WRN_NG << variables;
	}
	variables = vars;
}

void game_state::build_team(const config& side_cfg,
					 std::string save_id, std::vector<team>& teams,
					 const config& level, gamemap& map, unit_map& units,
					 bool snapshot)
{
	const config *player_cfg = NULL;
	//track whether a [player] tag with persistence information exists (in addition to the [side] tag)
	bool player_exists = false;

	if(map.empty()) {
		throw game::load_game_failed("Map not found");
	}

	if(side_cfg["controller"] == "human" ||
		side_cfg["controller"] == "network" ||
		side_cfg["controller"] == "network_ai" ||
		side_cfg["controller"] == "human_ai" ||
		utils::string_bool(side_cfg["persistent"])) {
		player_exists = true;

		//if we have a snapshot, level contains team information
		//else, we look for [side] or [player] (deprecated) tags in starting_pos
		if (snapshot) {
			if (const config &c = level.find_child("player","save_id",save_id))  {
				player_cfg = &c;
			}
		} else {
			//at the start of scenario, get the persistence information from starting_pos
			assert(starting_pos != NULL);
			if (const config &c =  starting_pos.find_child("player","save_id",save_id))  {
				player_cfg = &c;
			} else if (const config &c =  starting_pos.find_child("side","save_id",save_id))  {
				player_cfg = &c;
				player_exists = false; //there is only a [side] tag for this save_id in starting_pos
			}
		}
	}

	LOG_NG << "initializing team...\n";

	std::string gold = side_cfg["gold"];
	if(gold.empty())
		gold = "100";

	LOG_NG << "found gold: '" << gold << "'\n";

	int ngold = lexical_cast_default<int>(gold);

	/* This is the gold carry-over mechanism for subsequent campaign
	scenarios. Snapshots and replays are loaded from savegames and
	got their own gold information, which must not be altered here
	*/
	bool gold_add = utils::string_bool((side_cfg)["gold_add"]);
	if ( (player_cfg != NULL)  && (!snapshot) ) {
		try {
			int player_gold = lexical_cast_default<int>((*player_cfg)["gold"]);
			if (!player_exists) {
				//if we get the persistence information from [side], carryover gold is already sorted
				ngold = player_gold;
				gold_add = utils::string_bool((*player_cfg)["gold_add"]);
			} else if(utils::string_bool((*player_cfg)["gold_add"])) {
				ngold +=  player_gold;
				gold_add = true;
			} else if(player_gold >= ngold) {
				ngold = player_gold;
			}
		} catch (config::error&) {
			ERR_NG << "player tag for " << save_id << " does not have gold information\n";
		}
	}

	LOG_NG << "set gold to '" << ngold << "'\n";

	team temp_team(side_cfg, map, ngold);
	temp_team.set_gold_add(gold_add);
	teams.push_back(temp_team);

	// Update/fix the recall list for this side,
	// by setting the "side" of each unit in it
	// to be the "side" of the player.
	int side = lexical_cast_default<int>(side_cfg["side"], 1);

	//take recall list from [player] tag and update the side number of its units
	if (player_cfg != NULL) {
		foreach(const config &u, (*player_cfg).child_range("unit")) {
			if (u["x"].empty() && u["y"].empty() && !utils::string_bool(u["find_vacant"],false)) {
				config temp_cfg(u); //copy ctor, as player_cfg is const
				temp_cfg["side"] = str_cast<int>(side);
				//FIXME should probably pass &units here
				unit un(NULL, temp_cfg, false);
				teams.back().recall_list().push_back(un);
			}
		}
	}

	// If this team has no objectives, set its objectives
	// to the level-global "objectives"
	if(teams.back().objectives().empty()){
		const config& child = level.find_child_recursive("objectives", "side", side_cfg["side"]);
		bool silent = false;
		if (child && child.has_attribute("silent"))
			silent = utils::string_bool(child["silent"]);
		teams.back().set_objectives(level["objectives"], silent);
	}

	map_location start_pos = map_location::null_location;
	// If this side tag describes the leader of the side
	if(!utils::string_bool(side_cfg["no_leader"]) && side_cfg["controller"] != "null") {
		unit new_unit(&units, side_cfg, true);

		if (player_cfg != NULL) {
			for(std::vector<unit>::iterator it = teams.back().recall_list().begin();
				it != teams.back().recall_list().end(); ++it) {
				if(it->can_recruit()) {
					new_unit = *it;
					new_unit.set_game_context(&units);
					teams.back().recall_list().erase(it);
					break;
				}
			}
		}

		// See if the side specifies its location.
		// Otherwise start it at the map-given starting position.
		start_pos = map_location(side_cfg, this);

		if(side_cfg["x"].empty() && side_cfg["y"].empty()) {
			start_pos = map.starting_position(side);
		}

		if(!start_pos.valid() || !map.on_board(start_pos)) {
			throw game::load_game_failed(
				"Invalid starting position (" +
				lexical_cast<std::string>(start_pos.x+1) +
				"," + lexical_cast<std::string>(start_pos.y+1) +
				") for the leader of side " +
				lexical_cast<std::string>(side) + ".");
		}

		utils::string_map symbols;
		symbols["side"] = lexical_cast<std::string>(side);
		VALIDATE(units.count(start_pos) == 0,
			t_string(vgettext("Leader position not empty - duplicate side definition for side '$side|' found.", symbols)));

		units.add(start_pos, new_unit);
		LOG_NG << "initializing side '" << side_cfg["side"] << "' at "
			<< start_pos << '\n';
	}

	// If the game state specifies units that
	// can be recruited for the player, add them.
	// the can_recruit attribute is checked for backwards compatibility of saves
	if(player_cfg != NULL &&
		((*player_cfg).has_attribute("previous_recruits") || (*player_cfg).has_attribute("can recruit")) ) {
		std::vector<std::string> player_recruits;
			if (!(*player_cfg)["previous_recruits"].empty()) {
				player_recruits = utils::split((*player_cfg)["previous_recruits"]);
			}
			else {
				player_recruits = utils::split((*player_cfg)["can_recruit"]);
			}
		foreach (std::string rec, player_recruits) {
			teams.back().add_recruit(rec);
		}
	}

	// If there are additional starting units on this side
	const config::child_list& starting_units = side_cfg.get_children("unit");
	// the recall list has been filled by loading the [player]-section already.
	// However, we need to get the information from the snapshot,
	// so we start from scratch here.
	// This is rather a quick hack, originating from keeping changes
	// as minimal as possible for 1.2.
	if (player_exists && snapshot){
		teams.back().recall_list().clear();
	}

	//add the units with a specified position to the unit map
	for(config::child_list::const_iterator su = starting_units.begin(); su != starting_units.end(); ++su) {

		config temp_cfg(**su);
		temp_cfg["side"] = str_cast<int>(side); //set the side before unit creation to avoid invalid side errors
		temp_cfg.remove_attribute("find_vacant");

		const std::string& id =(**su)["id"];
		const std::string& x = (**su)["x"];
		const std::string& y = (**su)["y"];
		bool should_find_vacant_hex = utils::string_bool((**su)["find_vacant"],false);
		map_location loc(**su, this);

		if (should_find_vacant_hex) {
			if(!loc.valid() || !map.on_board(loc)) {
				loc = find_vacant_tile(map, units, start_pos, VACANT_ANY);
			} else {
				loc = find_vacant_tile(map, units, loc, VACANT_ANY);
			}
		}


		std::vector<unit>::iterator recall_list_element = std::find_if(teams.back().recall_list().begin(), teams.back().recall_list().end(), boost::bind(&unit::matches_id, _1, id));
		if(x.empty() && y.empty() && !should_find_vacant_hex) {
			//if there is no player tag, this means this team is either not persistent or the units have been added from a [side] tag earlier
			if(player_exists) {
				if (recall_list_element==teams.back().recall_list().end()) {
					unit new_unit(&units, temp_cfg, true);
					teams.back().recall_list().push_back(new_unit);
					LOG_NG << "inserting unit on recall list for side " << new_unit.side() << "\n";
				} else {
					LOG_NG << "wanted to insert unit on recall list, but recall list for side " << (**su)["side"] << "already contains id="<<id<<"\n";
				}
			}
		} else if(!loc.valid() || !map.on_board(loc)) {
			throw game::load_game_failed(
				"Invalid starting position (" +
				lexical_cast<std::string>(loc.x+1) +
				"," + lexical_cast<std::string>(loc.y+1) +
				") for a unit on side " +
				lexical_cast<std::string>(side) + ".");
		} else {
			if (units.find(loc) != units.end()) {
				ERR_NG << "[unit] trying to overwrite existing unit at " << loc << "\n";
			} else {
				if (recall_list_element==teams.back().recall_list().end()) {
					unit new_unit(&units, temp_cfg, true);
					units.add(loc, new_unit);
					LOG_NG << "inserting unit for side " << new_unit.side() << "\n";
				} else {
					//get unit from recall list
					unit u = *recall_list_element;
					u.set_game_context(&units);
					teams.back().recall_list().erase(recall_list_element);
					units.add(loc, u);
					LOG_NG << "inserting unit from recall list for side " << u.side()<< " with id="<< id << "\n";
				}
			}
		}
	}

}

void game_state::set_menu_items(const config::const_child_itors &menu_items)
{
	clear_wmi(wml_menu_items);
	foreach (const config &item, menu_items)
	{
		const std::string &id = item["id"].base_str();
		wml_menu_item*& mref = wml_menu_items[id];
		if(mref == NULL) {
			mref = new wml_menu_item(id, &item);
		} else {
			WRN_NG << "duplicate menu item (" << id << ") while loading gamestate\n";
		}
	}
}

void game_state::write_config(config_writer& out, bool write_variables) const
{
	out.write(classification_.to_config());
	if (classification_.campaign_type == "multiplayer")
		out.write_child("multiplayer", mp_settings_.to_config());
	out.write_key_val("random_seed", lexical_cast<std::string>(rng_.get_random_seed()));
	out.write_key_val("random_calls", lexical_cast<std::string>(rng_.get_random_calls()));
	if (write_variables) {
		out.write_child("variables", variables);
	}

	for(std::map<std::string, wml_menu_item *>::const_iterator j = wml_menu_items.begin();
	    j != wml_menu_items.end(); ++j) {
		out.open_child("menu_item");
		out.write_key_val("id", j->first);
		out.write_key_val("image", j->second->image);
		out.write_key_val("description", j->second->description);
		out.write_key_val("needs_select", (j->second->needs_select) ? "yes" : "no");
		if(!j->second->show_if.empty())
			out.write_child("show_if", j->second->show_if);
		if(!j->second->filter_location.empty())
			out.write_child("filter_location", j->second->filter_location);
		if(!j->second->command.empty())
			out.write_child("command", j->second->command);
		out.close_child("menu_item");
	}

	if (!replay_data.child("replay")) {
		out.write_child("replay", replay_data);
	}

	out.write_child("replay_start",starting_pos);
}

wml_menu_item::wml_menu_item(const std::string& id, const config* cfg) :
		name(),
		image(),
		description(),
		needs_select(false),
		show_if(),
		filter_location(),
		command()

{
	std::stringstream temp;
	temp << "menu item";
	if(!id.empty()) {
		temp << ' ' << id;
	}
	name = temp.str();
	if(cfg != NULL) {
		image = (*cfg)["image"];
		description = (*cfg)["description"];
		needs_select = utils::string_bool((*cfg)["needs_select"], false);
		if (const config &c = cfg->child("show_if")) show_if = c;
		if (const config &c = cfg->child("filter_location")) filter_location = c;
		if (const config &c = cfg->child("command")) command = c;
	}
}
