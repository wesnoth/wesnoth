//* $Id$ */
/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Maintain status of a game, load&save games.
 */

#include "global.hpp"
#include "config.hpp"

#include "gamestatus.hpp"

#include "actions.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "game_preferences.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "map.hpp"
#include "pathfind/pathfind.hpp"
#include "whiteboard/side_actions.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_engine_tc("engine/team_construction");
#define ERR_NG_TC LOG_STREAM(err, log_engine_tc)
#define WRN_NG_TC LOG_STREAM(warn, log_engine_tc)
#define LOG_NG_TC LOG_STREAM(info, log_engine_tc)
#define DBG_NG_TC LOG_STREAM(debug, log_engine_tc)

game_classification::game_classification():
	savegame_config(),
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
	savegame_config(),
	label(cfg["label"]),
	parent(cfg["parent"]),
	version(cfg["version"]),
	campaign_type(cfg["campaign_type"].empty() ? "scenario" : cfg["campaign_type"].str()),
	campaign_define(cfg["campaign_define"]),
	campaign_xtra_defines(utils::split(cfg["campaign_extra_defines"])),
	campaign(cfg["campaign"]),
	history(cfg["history"]),
	abbrev(cfg["abbrev"]),
	scenario(cfg["scenario"]),
	next_scenario(cfg["next_scenario"]),
	completion(cfg["completion"]),
	end_text(cfg["end_text"]),
	end_text_duration(cfg["end_text_duration"]),
	difficulty(cfg["difficulty"].empty() ? "NORMAL" : cfg["difficulty"].str())
	{}

game_classification::game_classification(const game_classification& gc):
	savegame_config(),
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

game_state::game_state()  :
		scoped_variables(),
		wml_menu_items(),
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(map_location::null_location),
		rng_(),
		variables_(),
		temporaries_(),
		generator_setter_(&recorder),
		classification_(),
		mp_settings_(),
		phase_(INITIAL),
		can_end_turn_(true)
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

		BOOST_FOREACH(const std::string& side_tag, tags)
		{
			BOOST_FOREACH(config &carryover_side, source->child_range(side_tag))
			{
				config *scenario_side = NULL;

				//TODO: use the player_id instead of the save_id for that
				if (config& c = cfg.find_child("side", "save_id", carryover_side["save_id"])) {
					scenario_side = &c;
				} else if (config& c = cfg.find_child("side", "id", carryover_side["save_id"])) {
					scenario_side = &c;
				}

				if (scenario_side == NULL) {
					//no matching side in the current scenario, we add the persistent information in a [player] tag
					cfg.add_child("player", carryover_side);
					continue;
				}

				//we have a matching side in the current scenario

				//sort carryover gold
				int ngold = (*scenario_side)["gold"].to_int(100);
				int player_gold = carryover_side["gold"];
				if (carryover_side["gold_add"].to_bool()) {
					ngold += player_gold;
				} else if (player_gold >= ngold) {
					ngold = player_gold;
				}
				carryover_side["gold"] = str_cast(ngold);
				if (const config::attribute_value *v = scenario_side->get("gold_add")) {
					carryover_side["gold_add"] = *v;
				}
				//merge player information into the scenario cfg
				(*scenario_side)["save_id"] = carryover_side["save_id"];
				(*scenario_side)["gold"] = ngold;
				(*scenario_side)["gold_add"] = carryover_side["gold_add"];
				if (const config::attribute_value *v = carryover_side.get("previous_recruits")) {
					(*scenario_side)["previous_recruits"] = *v;
				} else {
					(*scenario_side)["previous_recruits"] = carryover_side["can_recruit"];
				}
				(*scenario_side)["name"] = carryover_side["name"];
				(*scenario_side)["current_player"] = carryover_side["current_player"];

				(*scenario_side)["color"] = carryover_side["color"];

				//add recallable units
				BOOST_FOREACH(const config &u, carryover_side.child_range("unit")) {
					scenario_side->add_child("unit", u);
				}
			}
		}
	} else {
		BOOST_FOREACH(const config &snapshot_side, source->child_range("side")) {
			//take all side tags and add them as players (assuming they only contain carryover information)
			cfg.add_child("player", snapshot_side);
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
		variables_(),
		temporaries_(),
		generator_setter_(&recorder),
		classification_(cfg),
		mp_settings_(cfg),
		phase_(INITIAL),
		can_end_turn_(true)
{
	n_unit::id_manager::instance().set_save_id(cfg["next_underlying_unit_id"]);
	log_scope("read_game");

	const config &snapshot = cfg.child("snapshot");
	const config &replay_start = cfg.child("replay_start");
	// We're loading a snapshot if we have it and the user didn't request a replay.
	bool load_snapshot = !show_replay && snapshot && !snapshot.empty();

	if (load_snapshot) {
		this->snapshot = snapshot;

		rng_.seed_random(snapshot["random_calls"]);
	} else {
		assert(replay_start);
	}

	can_end_turn_ = load_snapshot ? snapshot["can_end_turn"].to_bool(true) : true;

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
			BOOST_FOREACH(const config &p, cfg.child_range("player")) {
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
	cfg["next_underlying_unit_id"] = str_cast(n_unit::id_manager::instance().get_save_id());
	cfg["can_end_turn"] = can_end_turn_;

	cfg["random_seed"] = rng_.get_random_seed();
	cfg["random_calls"] = rng_.get_random_calls();

	cfg["end_text"] = classification_.end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(classification_.end_text_duration);

	cfg.add_child("variables", variables_);

	for(std::map<std::string, wml_menu_item *>::const_iterator j=wml_menu_items.begin();
	    j!=wml_menu_items.end(); ++j) {
		config new_cfg;
		new_cfg["id"]=j->first;
		new_cfg["image"]=j->second->image;
		new_cfg["description"]=j->second->description;
		new_cfg["needs_select"] = j->second->needs_select;
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

	cfg_summary["replay"] = has_replay;
	cfg_summary["snapshot"] = has_snapshot;

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

	//BOOST_FOREACH(const config &p, cfg_save.child_range("player"))
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
			BOOST_FOREACH(const config &side, snapshot.child_range("side"))
			{
				if (side["controller"] != "human") {
					continue;
				}

				if (side["shroud"].to_bool()) {
					shrouded = true;
				}

				if (side["canrecruit"].to_bool())
				{
						leader = side["id"].str();
						leader_image = side["image"].str();
						break;
				}

				BOOST_FOREACH(const config &u, side.child_range("unit"))
				{
					if (u["canrecruit"].to_bool()) {
						leader = u["id"].str();
						leader_image = u["image"].str();
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

config::attribute_value &game_state::get_variable(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_SCALAR).as_scalar();
}

config::attribute_value game_state::get_variable_const(const std::string &key) const
{
	variable_info to_get(key, false, variable_info::TYPE_SCALAR);
	if (!to_get.is_valid)
	{
		config::attribute_value &to_return = temporaries_[key];
		if (key.size() > 7 && key.substr(key.size() - 7) == ".length") {
			// length is a special attribute, so guarantee its correctness
			to_return = 0;
		}
		return to_return;
	}
	return to_get.as_scalar();
}

config& game_state::get_variable_cfg(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_CONTAINER).as_container();
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

static void clear_wmi(std::map<std::string, wml_menu_item *> &gs_wmi)
{
	for (std::map<std::string, wml_menu_item *>::iterator i = gs_wmi.begin(),
	     i_end = gs_wmi.end(); i != i_end; ++i)
	{
		delete i->second;
	}
	gs_wmi.clear();
}

game_state::game_state(const game_state& state) :
	variable_set(), // Not sure why empty, copied from old code
	scoped_variables(state.scoped_variables),
	wml_menu_items(),
	replay_data(state.replay_data),
	starting_pos(state.starting_pos),
	snapshot(state.snapshot),
	last_selected(state.last_selected),
	rng_(state.rng_),
	variables_(state.variables_),
	temporaries_(), // Not sure why empty, copied from old code
	generator_setter_(state.generator_setter_),
	classification_(state.classification_),
	mp_settings_(state.mp_settings_),
	phase_(state.phase_),
	can_end_turn_(state.can_end_turn_)
{
	clear_wmi(wml_menu_items);
	std::map<std::string, wml_menu_item*>::const_iterator itor;
	for (itor = state.wml_menu_items.begin(); itor != state.wml_menu_items.end(); ++itor) {
		wml_menu_item*& mref = wml_menu_items[itor->first];
		mref = new wml_menu_item(*(itor->second));
	}
}

game_state& game_state::operator=(const game_state& state)
{
	// Use copy constructor to make sure we are coherant
	if (this != &state) {
		this->~game_state();
		new (this) game_state(state) ;
	}
	return *this ;
}

game_state::~game_state() {
	clear_wmi(wml_menu_items);
}

void game_state::set_variables(const config& vars) {
	variables_ = vars;
}


class team_builder {
public:
	team_builder(const config& side_cfg,
		     const std::string &save_id, std::vector<team>& teams,
		     const config& level, gamemap& map, unit_map& units,
		     bool snapshot, const config &starting_pos)
		: gold_info_ngold_(0)
		, gold_info_add_(false)
		, leader_configs_()
		, level_(level)
		, map_(map)
		, player_cfg_(NULL)
		, player_exists_(false)
		, save_id_(save_id)
		, seen_ids_()
		, side_(0)
		, side_cfg_(side_cfg)
		, snapshot_(snapshot)
		, starting_pos_(starting_pos)
		, t_(NULL)
		, teams_(teams)
		, unit_configs_()
		, units_(units)
	{
	}

	void build_team_stage_one()
	{
		//initialize the context variables and flags, find relevant tags, set up everything
		init();

		//find out the correct qty of gold and handle gold carryover.
		gold();

		//create a new instance of team and push it to back of resources::teams vector
		new_team();

		assert(t_!=NULL);

		//set team objectives if necessary
		objectives();

		// If the game state specifies additional units that can be recruited by the player, add them.
		previous_recruits();

		//place leader
		leader();

		//prepare units, populate obvious recall lists elements
		prepare_units();

	}


	void build_team_stage_two()
	{
		//place units
		//this is separate stage because we need to place units only after every other team is constructed
		place_units();

	}

protected:

	static const std::string default_gold_qty_;

	int gold_info_ngold_;
	bool gold_info_add_;
	std::deque<config> leader_configs_;
	const config &level_;
	gamemap &map_;
	const config *player_cfg_;
	bool player_exists_;
	const std::string save_id_;
	std::set<std::string> seen_ids_;
	int side_;
	const config &side_cfg_;
	bool snapshot_;
	const config &starting_pos_;
	team *t_;
	std::vector<team> &teams_;
	std::vector<const config*> unit_configs_;
	unit_map &units_;


	void log_step(const char *s) const
	{
		LOG_NG_TC << "team "<<side_<<" construction: "<< s << std::endl;
	}


	void init()
	{
		side_ = side_cfg_["side"].to_int(1);
		if (unsigned(side_ - 1) >= teams_.size() || teams_[side_ - 1].side() != 0)
			throw config::error("Invalid side number.");
		t_ = &teams_[side_ - 1];

		log_step("init");

		player_cfg_ = NULL;
		//track whether a [player] tag with persistence information exists (in addition to the [side] tag)
		player_exists_ = false;

		if(map_.empty()) {
			throw game::load_game_failed("Map not found");
		}

		if(side_cfg_["controller"] == "human" ||
		   side_cfg_["controller"] == "network" ||
		   side_cfg_["controller"] == "network_ai" ||
		   side_cfg_["controller"] == "human_ai" ||
			side_cfg_["persistent"].to_bool())
		{
			player_exists_ = true;

			//if we have a snapshot, level contains team information
			//else, we look for [side] or [player] (deprecated) tags in starting_pos
			///@deprecated r37519 [player] instead of [side] in starting_pos
			if (snapshot_) {
				if (const config &c = level_.find_child("player","save_id",save_id_))  {
					player_cfg_ = &c;
				}
			} else {
				//at the start of scenario, get the persistence information from starting_pos
				if (const config &c =  starting_pos_.find_child("player","save_id",save_id_))  {
					player_cfg_ = &c;
				} else if (const config &c =  starting_pos_.find_child("side","save_id",save_id_))  {
					player_cfg_ = &c;
					player_exists_ = false; //there is only a [side] tag for this save_id in starting_pos
				} else {
					player_cfg_ = NULL;
					player_exists_ = false;
				}
			}
		}

		DBG_NG_TC << "save id: "<< save_id_ <<std::endl;
		DBG_NG_TC << "snapshot: "<< (player_exists_ ? "true" : "false") <<std::endl;
		DBG_NG_TC << "player_cfg: "<< (player_cfg_==NULL ? "is null" : "is not null") <<std::endl;
		DBG_NG_TC << "player_exists: "<< (player_exists_ ? "true" : "false") <<std::endl;

		unit_configs_.clear();
		seen_ids_.clear();

	}

	bool use_player_cfg() const
	{
		return (player_cfg_ != NULL) && (!snapshot_);
	}

	void gold()
	{
		log_step("gold");

		std::string gold = side_cfg_["gold"];
		if(gold.empty()) {
			gold = default_gold_qty_;
		}

		DBG_NG_TC << "found gold: '" << gold << "'\n";

		gold_info_ngold_ = lexical_cast_default<int>(gold);

		/* This is the gold carry-over mechanism for subsequent campaign
		   scenarios. Snapshots and replays are loaded from savegames and
		   got their own gold information, which must not be altered here
		*/

		//true  - carryover gold is added to the start_gold.
		//false - the max of the two is taken as start_gold.
		gold_info_add_ = side_cfg_["gold_add"].to_bool();

		if (use_player_cfg()) {
			try {
				int player_gold = (*player_cfg_)["gold"];
				if (!player_exists_) {
					//if we get the persistence information from [side], carryover gold is already sorted
					gold_info_ngold_ = player_gold;
					gold_info_add_ = (*player_cfg_)["gold_add"].to_bool();
				} else if ((*player_cfg_)["gold_add"].to_bool()) {
					gold_info_ngold_ +=  player_gold;
					gold_info_add_ = true;
				} else if(player_gold >= gold_info_ngold_) {
					gold_info_ngold_ = player_gold;
				}
			} catch (config::error&) {
				ERR_NG_TC << "player tag for " << save_id_ << " does not have gold information\n";
			}
		}

		DBG_NG_TC << "set gold to '" << gold_info_ngold_ << "'\n";
		DBG_NG_TC << "set gold add flag to '" << gold_info_add_ << "'\n";
	}


	void new_team()
	{
		log_step("new team");
		t_->build(side_cfg_, map_, gold_info_ngold_);
		t_->set_gold_add(gold_info_add_);
	}


	void objectives()
	{
		log_step("objectives");
		// If this team has no objectives, set its objectives
		// to the level-global "objectives"
		if (t_->objectives().empty())
			t_->set_objectives(level_["objectives"], false);
	}


	void previous_recruits()
	{
		log_step("previous recruits");
		// If the game state specifies units that
		// can be recruited for the player, add them.
		if (!player_cfg_) return;
		if (const config::attribute_value *v = player_cfg_->get("previous_recruits")) {
			BOOST_FOREACH(const std::string &rec, utils::split(*v)) {
				DBG_NG_TC << "adding previous recruit: " << rec << '\n';
				t_->add_recruit(rec);
			}
		}
	}




	void handle_unit(const config &u, const char *origin)
	{
		DBG_NG_TC
			<< "unit from "<<origin
			<< ": type=["<<u["type"]
			<< "] id=["<<u["id"]
			<< "] placement=["<<u["placement"]
			<< "] x=["<<u["x"]
			<< "] y=["<<u["y"]
			<<"]"<< std::endl;
		const std::string &id = u["id"];
		if (!id.empty()) {
			if ( seen_ids_.find(id)!=seen_ids_.end() ) {
				//seen before
				config u_tmp = u;
				u_tmp["side"] = str_cast(side_);
				unit new_unit(u_tmp, true);
				t_->recall_list().push_back(new_unit);
			} else {
				//not seen before
				unit_configs_.push_back(&u);
				seen_ids_.insert(id);
			}

		} else {
			unit_configs_.push_back(&u);
		}
	}

	void handle_leader(const config &leader)
	{
		leader_configs_.push_back(leader);

		config::attribute_value &a1 = leader_configs_.back()["canrecruit"];
		if (a1.blank()) a1 = true;
		config::attribute_value &a2 = leader_configs_.back()["placement"];
		if (a2.blank()) a2 = "map,leader";

		handle_unit(leader_configs_.back(), "leader_cfg");
	}

	void leader()
	{
		log_step("leader");
		// If this side tag describes the leader of the side, we can simply add it to front of unit queue
		// there was a hack: if this side tag describes the leader of the side,
		// we may replace the leader with someone from recall list who can recruit, but take positioning from [side]
		// this hack shall be removed, since it messes up with 'multiple leaders'

		// If this side tag describes the leader of the side
		if (!side_cfg_["no_leader"].to_bool() && side_cfg_["controller"] != "null") {
			handle_leader(side_cfg_);
		}
		BOOST_FOREACH(const config &l, side_cfg_.child_range("leader")) {
			handle_leader(l);
		}
	}


	void prepare_units()
	{
		log_step("prepare units");
		if (use_player_cfg()) {
			//units in [replay_start][side] merged with [side]
			//only relevant in start-of-scenario saves, that's why !shapshot
			//units that are in '[scenario][side]' are 'first'
			//for create-or-recall semantics to work: for each unit with non-empty id, unconditionally put OTHER, later, units with same id directly to recall list, not including them in unit_configs_
			BOOST_FOREACH(const config &u, (*player_cfg_).child_range("unit")) {
				handle_unit(u,"player_cfg");
			}

		} else {
			//units in [side]
			BOOST_FOREACH(const config &su, side_cfg_.child_range("unit")) {
				handle_unit(su, "side_cfg");
			}
		}
	}


	void place_units()
	{
		static char const *side_attrs[] = {
			"income", "team_name", "user_team_name", "save_id",
			"current_player", "countdown_time", "action_bonus_count",
			"flag", "flag_icon", "objectives", "objectives_changed",
			"disallow_observers", "allow_player", "no_leader",
			"hidden", "music", "color", "colour", "ai_config", "gold",
			"start_gold", "team_rgb", "village_gold", "recall_cost",
			"controller", "persistent", "share_view",
			"share_maps", "recruit", "fog", "shroud", "shroud_data",
			"scroll_to_leader",
			// Multiplayer attributes.
			"income_lock", "gold_lock", "color_lock", "team_lock", "leader",
			"random_leader", "terrain_liked",
			"allow_changes", "faction_name", "user_description", "faction" };

		log_step("place units");
		BOOST_FOREACH(const config *u, unit_configs_) {
			unit_creator uc(*t_,map_.starting_position(side_));
			uc
				.allow_add_to_recall(true)
				.allow_discover(true)
				.allow_get_village(true)
				.allow_invalidate(false)
				.allow_rename_side(true)
				.allow_show(false);

			config cfg = *u;
			BOOST_FOREACH(const char *attr, side_attrs) {
				cfg.remove_attribute(attr);
			}
			uc.add_unit(cfg);

		}

		// Find the first leader and use its name as the player name.
		unit_map::iterator u = resources::units->find_first_leader(t_->side());
		if ((u != resources::units->end()) && t_->current_player().empty())
			t_->set_current_player(u->name());

	}

};

const std::string team_builder::default_gold_qty_ = "100";


team_builder_ptr game_state::create_team_builder(const config& side_cfg,
					 std::string save_id, std::vector<team>& teams,
					 const config& level, gamemap& map, unit_map& units,
					 bool snapshot)
{
	return team_builder_ptr(new team_builder(side_cfg,save_id,teams,level,map,units,snapshot,starting_pos));
}

void game_state::build_team_stage_one(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_one();
}

void game_state::build_team_stage_two(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_two();
}

void game_state::set_menu_items(const config::const_child_itors &menu_items)
{
	clear_wmi(wml_menu_items);
	BOOST_FOREACH(const config &item, menu_items)
	{
		std::string id = item["id"];
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
		out.write_child("variables", variables_);
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
		image = (*cfg)["image"].str();
		description = (*cfg)["description"];
		needs_select = (*cfg)["needs_select"].to_bool();
		if (const config &c = cfg->child("show_if")) show_if = c;
		if (const config &c = cfg->child("filter_location")) filter_location = c;
		if (const config &c = cfg->child("command")) command = c;
	}
}
