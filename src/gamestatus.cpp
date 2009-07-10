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
	campaign_type(cfg["campaign_type"]),
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
	difficulty(cfg["difficulty"])
	{}

game_classification::game_classification(const game_classification& gc):
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
{
	label = gc.label;
	parent = gc.parent;
	version = gc.version;
	campaign_type = gc.campaign_type;
	campaign_define = gc.campaign_define;
	campaign_xtra_defines = gc.campaign_xtra_defines;
	campaign = gc.campaign;
	history = gc.history;
	abbrev = gc.abbrev;
	scenario = gc.scenario;
	completion = gc.completion;
	end_text = gc.end_text;
	end_text_duration = gc.end_text_duration;
	difficulty = gc.difficulty;
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

player_info::player_info() :
	name(),
	gold(-1) ,
	gold_add(false),
	available_units(),
	can_recruit()
{}

player_info* game_state::get_player(const std::string& id) {
	std::map< std::string, player_info >::iterator found = players.find(id);
	if (found == players.end()) {
		WRN_NG << "player " << id << " does not exist.\n";
		return NULL;
	} else
		return &found->second;
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

static player_info read_player(const config &cfg)
{
	player_info res;

	res.name = cfg["name"];

	res.gold = atoi(cfg["gold"].c_str());
	res.gold_add = utils::string_bool(cfg["gold_add"]);

	foreach (const config &u, cfg.child_range("unit")) {
		res.available_units.push_back(unit(u, false));
	}

	res.can_recruit.clear();

	const std::string &can_recruit_str = cfg["can_recruit"];
	if (!can_recruit_str.empty()) {
		const std::vector<std::string> can_recruit = utils::split(can_recruit_str);
		std::copy(can_recruit.begin(),can_recruit.end(),std::inserter(res.can_recruit,res.can_recruit.end()));
	}

	return res;
}

game_state::game_state()  :
		players(),
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
		classification_()
		{}

void game_state::write_player(const std::string& save_id, const player_info& player, config& cfg, const bool use_snapshot) const
{
	cfg["name"] = player.name;
	cfg["save_id"]=save_id;

	 //FIXME: with the current consistency definition of sides by controller, some players may not show up in snapshot.side
	//player_info is used until side consistency is fixed
	bool side_not_found = false;
	//add gold from snapshot
	if (use_snapshot) {
		if (const config &side = snapshot.find_child("side", "save_id", save_id)) {
			cfg["gold"] = side["gold"];
			cfg["gold_add"] = side["gold_add"];
			assert(cfg["gold"] == str_cast<int>(player.gold));
		} else if(const config &snapshot_player = snapshot.find_child("player", "save_id", save_id)) {
			cfg["gold"] = snapshot_player["gold"];
			cfg["gold_add"] = snapshot_player["gold_add"];
			assert(cfg["gold"] == str_cast<int>(player.gold));
		} else {
			WRN_NG << "side " << save_id << " does not exist in snapshot, using player_info\n";
			side_not_found = true;
		}
	}
	//do not store gold if specified by use_snapshot
	 if (!use_snapshot || side_not_found){
		char buf[50];
		snprintf(buf,sizeof(buf),"%d",player.gold);

		cfg["gold"] = buf;
		cfg["gold_add"] = player.gold_add ? "yes" : "no";
	}


	for(std::vector<unit>::const_iterator i = player.available_units.begin();
	    i != player.available_units.end(); ++i) {
		config new_cfg;
		i->write(new_cfg);
		cfg.add_child("unit",new_cfg);
		DBG_NG << "added unit '" << new_cfg["id"] << "' to player '" << player.name << "'\n";
	}

	std::stringstream can_recruit;
	std::copy(player.can_recruit.begin(),player.can_recruit.end(),std::ostream_iterator<std::string>(can_recruit,","));
	std::string can_recruit_str = can_recruit.str();

	// Remove the trailing comma
	if(can_recruit_str.empty() == false) {
		can_recruit_str.resize(can_recruit_str.size()-1);
	}

	cfg["can_recruit"] = can_recruit_str;
}

void write_players(game_state& gamestate, config& cfg, const bool use_snapshot)
{
	// If there is already a player config available it means we are loading
	// from a savegame. Don't do anything then, the information is already there
	config::child_itors player_cfg = cfg.child_range("player");
	if (player_cfg.first != player_cfg.second)
		return;
	
	//take all sides information from the snapshot (assuming it only contains carryover information)
	if (use_snapshot) {
		//take all side tags and add them as players
		foreach(const config* snapshot_side, gamestate.snapshot.get_children("side")) {
			cfg.add_child("player", *snapshot_side);
		}
		//add the remaining player tags
		foreach(const config* snapshot_player, gamestate.snapshot.get_children("player")) {
			cfg.add_child("player", *snapshot_player);
		}
	}
	else {
		for(std::map<std::string, player_info>::const_iterator i=gamestate.players.begin();
			i!=gamestate.players.end(); ++i)
		{
			config new_cfg;
			gamestate.write_player(i->first, i->second, new_cfg, use_snapshot);

			cfg.add_child("player", new_cfg);
		}
	}
}

game_state::game_state(const config& cfg, bool show_replay) :
		players(),
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
		classification_(cfg)
{
	n_unit::id_manager::instance().set_save_id(lexical_cast_default<size_t>(cfg["next_underlying_unit_id"],0));
	log_scope("read_game");

	const config &snapshot = cfg.child("snapshot");
	const config &replay_start = cfg.child("replay_start");

	// We have to load era id for MP games so they can load correct era.


	if (snapshot && !snapshot.empty() && !show_replay) {

		this->snapshot = snapshot;

		rng_.seed_random(lexical_cast_default<unsigned>(snapshot["random_calls"]));

		// Midgame saves have the recall list stored in the snapshot.
		load_recall_list(snapshot.child_range("player"));

	} else {
		assert(replay_start != NULL);

		// The player information should no longer be saved to the root of the config.
		// The game now looks for the info in just the snapshot or the starting position.
		// Check if we find some player information in the starting position
		config::const_child_itors cfg_players = replay_start.child_range("player");

		if (cfg_players.first != cfg_players.second)
			load_recall_list(cfg_players);
	}

	LOG_NG << "scenario: '" << classification_.scenario << "'\n";
	LOG_NG << "next_scenario: '" << classification_.next_scenario << "'\n";

	if(classification_.difficulty.empty()) {
		classification_.difficulty = "NORMAL";
	}

	if(classification_.campaign_type.empty()) {
		classification_.campaign_type = "scenario";
	}

	if (const config &vars = cfg.child("variables")) {
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

void game_state::write_snapshot(config& cfg, const bool use_snapshot) const
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

	//if snapshot is supposed to be used for side persistance information, we add it later
	if(!use_snapshot) {
		for(std::map<std::string, player_info>::const_iterator i=players.begin(); i!=players.end(); ++i) {
			config new_cfg;
			write_player(i->first, i->second, new_cfg, use_snapshot);
			new_cfg["save_id"]=i->first;
			cfg.add_child("player", new_cfg);
		}
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
	if(!to_get.is_valid) return temporaries[key];
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

void game_state::load_recall_list(const config::const_child_itors &players)
{
	if (players.first == players.second) return;

	foreach (const config &p, players)
	{
		const std::string &save_id = p["save_id"];

		if (save_id.empty()) {
			ERR_NG << "Corrupted player entry: NULL save_id" << std::endl;
		} else {
			player_info player = read_player(p);
			this->players.insert(std::make_pair(save_id, player));
		}
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
	players(),
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
	classification_()
{
	*this = state;
}

game_state& game_state::operator=(const game_state& state)
{
	if(this == &state) {
		return *this;
	}

	rng_ = state.rng_;
	players = state.players;
	scoped_variables = state.scoped_variables;
	classification_ = game_classification(state.classification());

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

//build and populate a team from a config
void game_state::get_player_info(const config& side_cfg,
					 std::string save_id, std::vector<team>& teams,
					 const config& level, gamemap& map, unit_map& units,
					 tod_manager& tod_mng, bool snapshot)
{
	const config *player_cfg = NULL;
	//FIXME: temporarily adding this flag to ensure recallable units are added properly without player_info
	bool player_exists = false;
	player_info *player = NULL;

	if(map.empty()) {
		throw game::load_game_failed("Map not found");
	}

	if(side_cfg["controller"] == "human" ||
		side_cfg["controller"] == "network" ||
		side_cfg["controller"] == "network_ai" ||
		side_cfg["controller"] == "human_ai") {
		player = get_player(save_id);
		player_exists = true;
		
		//if we have a snapshot, level contains player tags
		//else, we look for a player tag in starting_pos
		if (snapshot) {
			if (const config &c = level.find_child("player","save_id",save_id))  {
				player_cfg = &c;
			}
		} else {
			//at the start of scenario, get the player tag from starting_pos
			assert(starting_pos != NULL);
			if (const config &c =  starting_pos.find_child("player","save_id",save_id))  {
				player_cfg = &c;
			}
		}

		if(player == NULL && !save_id.empty()) {
			player = &players[save_id];
		}
	}

	//if there is no player tag in either snapshot or replay_start, we have no persisting information for this team
	if (!player_cfg) {
		LOG_NG << save_id << " does not have a corresponding player tag\n";
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
	bool gold_add = false;
	if ( (player_cfg != NULL)  && (!snapshot) ) {
		try {
			int player_gold = lexical_cast_default<int>((*player_cfg)["gold"]);
			if(utils::string_bool((*player_cfg)["gold_add"])) {
				ngold +=  player_gold;
				gold_add = true;
			} else if(player_gold >= ngold) {
				ngold = player_gold;
			}
		} catch (config::error&) {
			ERR_NG << "player tag for " << save_id << " does not have gold information\n";
		}
		
		player->gold = ngold;
	}

	LOG_NG << "set gold to '" << ngold << "'\n";

	team temp_team(side_cfg, map, ngold);
	temp_team.set_gold_add(gold_add);
	teams.push_back(temp_team);

	// Update/fix the recall list for this side,
	// by setting the "side" of each unit in it
	// to be the "side" of the player.
	int side = lexical_cast_default<int>(side_cfg["side"], 1);
	if(player != NULL) {
		for(std::vector<unit>::iterator it = player->available_units.begin();
			it != player->available_units.end(); ++it) {
			it->set_side(side);
			//teams.back().recall_list().push_back(*it); //FIXME: take recall list from snapshot/replay_start once player_info is removed
		}
	}

	//take recall list from [player] tag and update the side number of its units
	if (player_cfg != NULL) {
		foreach(const config &u, (*player_cfg).child_range("unit")) {
			config temp_cfg(u); //copy ctor, as player_cfg is const
			temp_cfg["side"] = str_cast<int>(side);
			unit un(temp_cfg, false);
			teams.back().recall_list().push_back(un);
		}
	}

	// If this team has no objectives, set its objectives
	// to the level-global "objectives"
	if(teams.back().objectives().empty())
		teams.back().set_objectives(level["objectives"]);

	// If this side tag describes the leader of the side
	if(!utils::string_bool(side_cfg["no_leader"]) && side_cfg["controller"] != "null") {
		unit new_unit(&units, &map, &tod_mng, &teams, side_cfg, true);

		// Search the recall list for leader units, and if there is one,
		// use it in place of the config-described unit
		if(player != NULL) {
			for(std::vector<unit>::iterator it = player->available_units.begin();
				it != player->available_units.end(); ++it) {
				if(it->can_recruit()) {
					player->available_units.erase(it);
					break;
				}
			}
		}
		
		if (player_cfg != NULL) {
			for(std::vector<unit>::iterator it = teams.back().recall_list().begin();
				it != teams.back().recall_list().end(); ++it) {
				if(it->can_recruit()) {
					new_unit = *it;
					new_unit.set_game_context(&units, &map, &tod_mng, &teams);
					teams.back().recall_list().erase(it);
					break;
				}
			}
		}

		// See if the side specifies its location.
		// Otherwise start it at the map-given starting position.
		map_location start_pos(side_cfg, this);

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
			t_string(vgettext("Duplicate side definition for side '$side|' found.", symbols)));

		units.add(map.starting_position(new_unit.side()), new_unit);
		LOG_NG << "initializing side '" << side_cfg["side"] << "' at "
			<< start_pos << '\n';
	}

	// If the game state specifies units that
	// can be recruited for the player, add them.
	if(player_cfg != NULL && (*player_cfg).has_attribute("can_recruit")) {
		std::vector<std::string> player_recruits = utils::split((*player_cfg)["can_recruit"]);
		foreach (std::string rec, player_recruits) {
			teams.back().add_recruit(rec);
		}
	}

	if(player != NULL) {
		player->can_recruit = teams.back().recruits();
	}

	// If there are additional starting units on this side
	const config::child_list& starting_units = side_cfg.get_children("unit");
	// available_units has been filled by loading the [player]-section already.
	// However, we need to get the information from the snapshot,
	// so we start from scratch here.
	// This is rather a quick hack, originating from keeping changes
	// as minimal as possible for 1.2.
	// Moving [player] into [replay_start] should be the correct way to go.
	if (player && snapshot){
		player->available_units.clear();
		teams.back().recall_list().clear();
	}
	for(config::child_list::const_iterator su = starting_units.begin(); su != starting_units.end(); ++su) {
		unit new_unit(&units, &map, &tod_mng, &teams,**su,true);

		new_unit.set_side(side);

		const std::string& x = (**su)["x"];
		const std::string& y = (**su)["y"];

		map_location loc(**su, this);
		if(x.empty() && y.empty()) {
			if(player_exists) {
				player->available_units.push_back(new_unit);
				teams.back().recall_list().push_back(new_unit);
				LOG_NG << "inserting unit on recall list for side " << new_unit.side() << "\n";
			} else {
				throw game::load_game_failed(
					"Attempt to create a unit on the recall list for side " +
					lexical_cast<std::string>(side) +
					", which does not have a recall list.");
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
				units.add(loc, new_unit);
				LOG_NG << "inserting unit for side " << new_unit.side() << "\n";
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

void player_info::debug(){
	LOG_NG << "Debugging player\n";
	LOG_NG << "\tName: " << name << "\n";
	LOG_NG << "\tGold: " << gold << "\n";
	LOG_NG << "\tAvailable units:\n";
	for (std::vector<unit>::const_iterator u = available_units.begin(); u != available_units.end(); u++){
		LOG_NG << "\t\t" + u->name() + "\n";
	}
	LOG_NG << "\tEnd available units\n";
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
