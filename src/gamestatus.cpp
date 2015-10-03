//* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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

#include "actions.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "game_preferences.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "map.hpp"
#include "pathfind/pathfind.hpp"

#include <boost/bind.hpp>

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

		BOOST_FOREACH (const std::string& side_tag, tags) {
			BOOST_FOREACH (config* carryover_side, source->get_children(side_tag)) {
				config *scenario_side = NULL;

				//TODO: use the player_id instead of the save_id for that
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
					(*scenario_side)["current_player"] = (*carryover_side)["current_player"];
					(*scenario_side)["colour"] = (*carryover_side)["colour"];
					//add recallable units
					BOOST_FOREACH (const config* u, carryover_side->get_children("unit")) {
						scenario_side->add_child("unit", *u);
					}

				} else {
					//no matching side in the current scenario, we add the persistent information in a [player] tag
					cfg.add_child("player", (*carryover_side));
				}
			}
	}

	} else {
		BOOST_FOREACH(const config* snapshot_side, source->get_children("side")) {
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
			BOOST_FOREACH (const config &p, cfg.child_range("player")) {
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

	//BOOST_FOREACH (const config &p, cfg_save.child_range("player"))
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
			BOOST_FOREACH (const config &side, snapshot.child_range("side"))
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

				BOOST_FOREACH (const config &u, side.child_range("unit"))
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
	variables = state.variables;

	return *this;
}

game_state::~game_state() {
	clear_wmi(wml_menu_items);
}

void game_state::set_variables(const config& vars) {
	variables = vars;
}


class team_builder {
public:
	team_builder(const config& side_cfg,
		     const std::string &save_id, std::vector<team>& teams,
		     const config& level, gamemap& map, unit_map& units,
		     bool snapshot, const config &starting_pos)
		: gold_info_ngold_(0)
		, gold_info_add_(false)
		, leader_cfg_()
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

	void build_team()
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

		//place units
		place_units();

	}

protected:

	static const std::string default_gold_qty_;

	int gold_info_ngold_;
	bool gold_info_add_;
	config leader_cfg_;
	const config &level_;
	gamemap &map_;
	const config *player_cfg_;
	bool player_exists_;
	const std::string &save_id_;
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
		side_ = lexical_cast_default<int>(side_cfg_["side"], 1);

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
		   utils::string_bool(side_cfg_["persistent"])) {
			player_exists_ = true;

			//if we have a snapshot, level contains team information
			//else, we look for [side] or [player] (deprecated) tags in starting_pos
			if (snapshot_) {
				if (const config &c = level_.find_child("player","save_id",save_id_))  {
					player_cfg_ = &c;
				}
			} else {
				//at the start of scenario, get the persistence information from starting_pos
				assert(starting_pos_ != NULL);
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

		DBG_NG_TC << "snapshot: "<< (player_exists_ ? "true" : "false") <<std::endl;
		DBG_NG_TC << "player_cfg: "<< (player_cfg_==NULL ? "is null" : "is not null") <<std::endl;
		DBG_NG_TC << "player_exists: "<< (player_exists_ ? "true" : "false") <<std::endl;

		unit_configs_.clear();
		seen_ids_.clear();
		leader_cfg_ = config();

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
		gold_info_add_ = utils::string_bool((side_cfg_)["gold_add"]);

		if (use_player_cfg()) {
			try {
				int player_gold = lexical_cast_default<int>((*player_cfg_)["gold"]);
				if (!player_exists_) {
					//if we get the persistence information from [side], carryover gold is already sorted
					gold_info_ngold_ = player_gold;
					gold_info_add_ = utils::string_bool((*player_cfg_)["gold_add"]);
				} else if(utils::string_bool((*player_cfg_)["gold_add"])) {
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
		team temp_team(side_cfg_, map_, gold_info_ngold_);
		temp_team.set_gold_add(gold_info_add_);
		teams_.push_back(temp_team);
		t_ = &teams_.back();
	}


	void objectives()
	{
		log_step("objectives");
		// If this team has no objectives, set its objectives
		// to the level-global "objectives"
		if(t_->objectives().empty()){
			const config& child = level_.find_child_recursive("objectives", "side", side_cfg_["side"]);
			bool silent = false;
			if (child && child.has_attribute("silent"))
				silent = utils::string_bool(child["silent"]);
			t_->set_objectives(level_["objectives"], silent);
		}
	}


	void previous_recruits()
	{
		log_step("previous recruits");
		// If the game state specifies units that
		// can be recruited for the player, add them.
		// the can_recruit attribute is checked for backwards compatibility of saves
		if(player_cfg_ != NULL &&
		   ((*player_cfg_).has_attribute("previous_recruits") || (*player_cfg_).has_attribute("can recruit")) ) {
			std::vector<std::string> player_recruits;
			if (!(*player_cfg_)["previous_recruits"].empty()) {
				player_recruits = utils::split((*player_cfg_)["previous_recruits"]);
			}
			else {
				player_recruits = utils::split((*player_cfg_)["can_recruit"]);
			}
			BOOST_FOREACH (std::string rec, player_recruits) {
				DBG_NG_TC << "adding previous recruit: "<< rec << std::endl;
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
				unit new_unit(&units_, u_tmp, true);
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

	void leader()
	{
		log_step("leader");
		// If this side tag describes the leader of the side, we can simply add it to front of unit queue
		// there was a hack: if this side tag describes the leader of the side,
		// we may replace the leader with someone from recall list who can recruit, but take positioning from [side]
		// this hack shall be removed, since it messes up with 'multiple leaders'

		// If this side tag describes the leader of the side
		if(!utils::string_bool(side_cfg_["no_leader"]) && side_cfg_["controller"] != "null") {
			leader_cfg_ = side_cfg_;

			if (!leader_cfg_.has_attribute("canrecruit")){
				leader_cfg_["canrecruit"] = "yes";
			}
			if (!leader_cfg_.has_attribute("placement")){
				leader_cfg_["placement"] = "map,leader";
			}

			handle_unit(leader_cfg_,"leader_cfg");
		} else {
			leader_cfg_ = config();
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
			const config::child_list& starting_units = side_cfg_.get_children("unit");
			for(config::child_list::const_iterator su = starting_units.begin(); su != starting_units.end(); ++su) {
				handle_unit(**su,"side_cfg");
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
			"hidden", "music", "colour", "ai_config", "gold",
			"start_gold", "team_rgb", "village_gold", "controller",
			"persistent", "share_view",
			"share_maps", "recruit", "fog", "shroud", "shroud_data",
			// Multiplayer attributes.
			"income_lock", "gold_lock", "team_lock", "leader",
			"random_leader", "terrain_liked",
			"allow_changes", "faction_name", "user_description" };

		log_step("place units");
		BOOST_FOREACH (const config *u, unit_configs_) {
			unit_creator uc(*t_,map_.starting_position(side_));
			uc
				.allow_add_to_recall(true)
				.allow_discover(true)
				.allow_get_village(true)
				.allow_invalidate(false)
				.allow_rename_side(true)
				.allow_show(false);

			config cfg = *u;
			BOOST_FOREACH (const char *attr, side_attrs) {
				cfg.remove_attribute(attr);
			}
			uc.add_unit(cfg);

		}

		// Find the first leader and use its name as the player name.
		unit_map::iterator u = resources::units->find_first_leader(t_->side());
		if ((u != resources::units->end()) && t_->current_player().empty())
			t_->set_current_player(u->second.name());

	}

};

const std::string team_builder::default_gold_qty_ = "100";


void game_state::build_team(const config& side_cfg,
					 std::string save_id, std::vector<team>& teams,
					 const config& level, gamemap& map, unit_map& units,
					 bool snapshot)
{
	team_builder tb = team_builder(side_cfg,save_id,teams,level,map,units,snapshot,starting_pos);
	tb.build_team();
}


void game_state::set_menu_items(const config::const_child_itors &menu_items)
{
	clear_wmi(wml_menu_items);
	BOOST_FOREACH (const config &item, menu_items)
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
