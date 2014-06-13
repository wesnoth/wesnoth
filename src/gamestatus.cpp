/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "gamestatus.hpp"

#include "actions/create.hpp"
#include "carryover.hpp"
#include "filesystem.hpp"
#include "formula_string_utils.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "serialization/binary_or_text.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "variable.hpp"
#include "pathfind/pathfind.hpp"
#include "whiteboard/side_actions.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "map_label.hpp"

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

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)


/// The default difficulty setting for campaigns.
const std::string DEFAULT_DIFFICULTY("NORMAL");


class team_builder {
public:
	team_builder(const config& side_cfg,
		     const std::string &save_id, std::vector<team>& teams,
		     const config& level, gamemap& map, unit_map& units,
		     const config &starting_pos)
		: gold_info_ngold_(0)
		, leader_configs_()
		, level_(level)
		, map_(map)
		, player_exists_(false)
		, save_id_(save_id)
		, seen_ids_()
		, side_(0)
		, side_cfg_(side_cfg)
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

	int gold_info_ngold_;
	std::deque<config> leader_configs_;
	const config &level_;
	gamemap &map_;
	bool player_exists_;
	const std::string save_id_;
	std::set<std::string> seen_ids_;
	int side_;
	const config &side_cfg_;
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
		if (side_ == 0) // Otherwise falls into the next error, with a very confusing message
			throw config::error("Side number 0 encountered. Side numbers start at 1");
		if (unsigned(side_ - 1) >= teams_.size()) {
			std::stringstream ss;
			ss << "Side number " << side_ << " higher than number of sides (" << teams_.size() << ")";
			throw config::error(ss.str());
		}
		if (teams_[side_ - 1].side() != 0) {
			std::stringstream ss;
			ss << "Duplicate definition of side " << side_;
			throw config::error(ss.str());
		}
		t_ = &teams_[side_ - 1];

		log_step("init");

		//track whether a [player] tag with persistence information exists (in addition to the [side] tag)
		player_exists_ = false;

		if(map_.empty()) {
			throw game::load_game_failed("Map not found");
		}

		DBG_NG_TC << "save id: "<< save_id_ <<std::endl;
		DBG_NG_TC << "snapshot: "<< (player_exists_ ? "true" : "false") <<std::endl;

		unit_configs_.clear();
		seen_ids_.clear();

	}


	void gold()
	{
		log_step("gold");

		gold_info_ngold_ = side_cfg_["gold"];

		DBG_NG_TC << "set gold to '" << gold_info_ngold_ << "'\n";
		//DBG_NG_TC << "set gold add flag to '" << gold_info_add_ << "'\n";
	}


	void new_team()
	{
		log_step("new team");
		t_->build(side_cfg_, map_, gold_info_ngold_);
		//t_->set_gold_add(gold_info_add_);
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
		if (!side_cfg_) return;
		if (const config::attribute_value *v = side_cfg_.get("previous_recruits")) {
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
				t_->recall_list().push_back(UnitPtr(new unit(u_tmp,true)));
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
		// Make a persistent copy of the config.
		leader_configs_.push_back(leader);
		config & stored = leader_configs_.back();

		// Remove the attributes used to define a side.
		BOOST_FOREACH( const std::string & attr , team::attributes) {
			stored.remove_attribute(attr);
		}

		// Provide some default values, if not specified.
		config::attribute_value &a1 = stored["canrecruit"];
		if (a1.blank()) a1 = true;
		config::attribute_value &a2 = stored["placement"];
		if (a2.blank()) a2 = "map,leader";

		// Add the leader to the list of units to create.
		handle_unit(stored, "leader_cfg");
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
			if (side_cfg_["type"] == "random") {
				std::vector<std::string> types = utils::split(side_cfg_["random_leader"]);
				if (types.empty())
					types = utils::split(side_cfg_["leader"]);
				if (types.empty()) {
					utils::string_map i18n_symbols;
					i18n_symbols["faction"] = side_cfg_["name"];
					throw config::error(vgettext("Unable to find a leader type for faction $faction", i18n_symbols));
				}
				const int choice = rand() % types.size();
				config leader = side_cfg_;
				leader["type"] = types[choice];
				handle_leader(leader);
			} else
				handle_leader(side_cfg_);
		}
		BOOST_FOREACH(const config &l, side_cfg_.child_range("leader")) {
			handle_leader(l);
		}
	}


	void prepare_units()
	{
		//if this is a start-of-scenario save then  playcampaign.cpp merged
		//units in [replay_start][side] merged with [side] already
		//units that are in '[scenario][side]' are 'first'

		//for create-or-recall semantics to work: for each unit with non-empty
		//id, unconditionally put OTHER, later, units with same id directly to
		//recall list, not including them in unit_configs_
		BOOST_FOREACH(const config &su, side_cfg_.child_range("unit")) {
			handle_unit(su, "side_cfg");
		}
	}


	void place_units()
	{
		log_step("place units");
		unit_creator uc(*t_,map_.starting_position(side_));
		uc
			.allow_add_to_recall(true)
			.allow_discover(true)
			.allow_get_village(true)
			.allow_invalidate(false)
			.allow_rename_side(true)
			.allow_show(false);

		BOOST_FOREACH(const config *u, unit_configs_) {
			uc.add_unit(*u);
		}

		// Find the first leader and use its name as the player name.
		unit_map::iterator u = resources::units->find_first_leader(t_->side());
		if ((u != resources::units->end()) && t_->current_player().empty())
			t_->set_current_player(u->name());

	}

};

game_data::game_data()
		: scoped_variables()
		, last_selected(map_location::null_location())
		, wml_menu_items_()
		, rng_()
		, variables_()
		, temporaries_()
		, phase_(INITIAL)
		, can_end_turn_(true)
		, scenario_()
		, next_scenario_()
		{}

game_data::game_data(const config& level)
		: scoped_variables()
		, last_selected(map_location::null_location())
		, wml_menu_items_()
		, rng_(level)
		, variables_(level.child_or_empty("variables"))
		, temporaries_()
		, phase_(INITIAL)
		, can_end_turn_(level["can_end_turn"].to_bool(true))
		, scenario_(level["id"])
		, next_scenario_(level["next_scenario"])
{
	wml_menu_items_.set_menu_items(level);
}

game_data::game_data(const game_data& data)
		: variable_set() // Not sure why empty, copied from old code
		, scoped_variables(data.scoped_variables)
		, last_selected(data.last_selected)
		, wml_menu_items_(data.wml_menu_items_)
		, rng_(data.rng_)
		, variables_(data.variables_)
		, temporaries_()
		, phase_(data.phase_)
		, can_end_turn_(data.can_end_turn_)
		, scenario_(data.scenario_)
		, next_scenario_(data.next_scenario_)
{}

config::attribute_value &game_data::get_variable(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_SCALAR).as_scalar();
}

config::attribute_value game_data::get_variable_const(const std::string &key) const
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

config& game_data::get_variable_cfg(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_CONTAINER).as_container();
}

void game_data::set_variable(const std::string& key, const t_string& value)
{
	get_variable(key) = value;
}

config& game_data::add_variable_cfg(const std::string& key, const config& value)
{
	variable_info to_add(key, true, variable_info::TYPE_ARRAY);
	return to_add.vars->add_child(to_add.key, value);
}

void game_data::clear_variable_cfg(const std::string& varname)
{
	variable_info to_clear(varname, false, variable_info::TYPE_CONTAINER);
	if(!to_clear.is_valid) return;
	if(to_clear.explicit_index) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
	}
}

void game_data::clear_variable(const std::string& varname)
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

void game_data::write_snapshot(config& cfg) const {
	cfg["scenario"] = scenario_;
	cfg["next_scenario"] = next_scenario_;

	cfg["can_end_turn"] = can_end_turn_;

	cfg["random_seed"] = rng_.get_random_seed();
	cfg["random_calls"] = rng_.get_random_calls();

	cfg.add_child("variables", variables_);

	wml_menu_items_.to_config(cfg);
}

void game_data::write_config(config_writer& out){
	out.write_key_val("scenario", scenario_);
	out.write_key_val("next_scenario", next_scenario_);

	out.write_key_val("random_seed", lexical_cast<std::string>(rng_.get_random_seed()));
	out.write_key_val("random_calls", lexical_cast<std::string>(rng_.get_random_calls()));
	out.write_child("variables", variables_);

	config cfg;
	wml_menu_items_.to_config(cfg);
	out.write_child("menu_item", cfg);
}

team_builder_ptr game_data::create_team_builder(const config& side_cfg,
					 std::string save_id, std::vector<team>& teams,
					 const config& level, gamemap& map, unit_map& units,
					 const config& starting_pos)
{
	return team_builder_ptr(new team_builder(side_cfg, save_id, teams, level, map, units, starting_pos));
}

void game_data::build_team_stage_one(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_one();
}

void game_data::build_team_stage_two(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_two();
}

game_data& game_data::operator=(const game_data& info)
{
	// Use copy constructor to make sure we are coherent
	if (this != &info) {
		this->~game_data();
		new (this) game_data(info) ;
	}
	return *this ;
}

game_data* game_data::operator=(const game_data* info)
{
	// Use copy constructor to make sure we are coherent
	if (this != info) {
		this->~game_data();
		new (this) game_data(*info) ;
	}
	return this ;
}

game_classification::game_classification():
	savegame_config(),
	label(),
	version(),
	campaign_type(),
	campaign_define(),
	campaign_xtra_defines(),
	campaign(),
	abbrev(),
	completion(),
	end_credits(true),
	end_text(),
	end_text_duration(),
	difficulty(DEFAULT_DIFFICULTY),
	random_mode("")
	{}

game_classification::game_classification(const config& cfg):
	savegame_config(),
	label(cfg["label"]),
	version(cfg["version"]),
	campaign_type(lexical_cast_default<game_classification::CAMPAIGN_TYPE> (cfg["campaign_type"].str(), game_classification::SCENARIO)),
	campaign_define(cfg["campaign_define"]),
	campaign_xtra_defines(utils::split(cfg["campaign_extra_defines"])),
	campaign(cfg["campaign"]),
	abbrev(cfg["abbrev"]),
	completion(cfg["completion"]),
	end_credits(cfg["end_credits"].to_bool(true)),
	end_text(cfg["end_text"]),
	end_text_duration(cfg["end_text_duration"]),
	difficulty(cfg["difficulty"].empty() ? DEFAULT_DIFFICULTY : cfg["difficulty"].str()),
	random_mode(cfg["random_mode"])
	{}

game_classification::game_classification(const game_classification& gc):
	savegame_config(),
	label(gc.label),
	version(gc.version),
	campaign_type(gc.campaign_type),
	campaign_define(gc.campaign_define),
	campaign_xtra_defines(gc.campaign_xtra_defines),
	campaign(gc.campaign),
	abbrev(gc.abbrev),
	completion(gc.completion),
	end_credits(gc.end_credits),
	end_text(gc.end_text),
	end_text_duration(gc.end_text_duration),
	difficulty(gc.difficulty),
	random_mode(gc.random_mode)
{
}

config game_classification::to_config() const
{
	config cfg;

	cfg["label"] = label;
	cfg["version"] = game_config::version;
	cfg["campaign_type"] = lexical_cast<std::string> (campaign_type);
	cfg["campaign_define"] = campaign_define;
	cfg["campaign_extra_defines"] = utils::join(campaign_xtra_defines);
	cfg["campaign"] = campaign;
	cfg["abbrev"] = abbrev;
	cfg["completion"] = completion;
	cfg["end_credits"] = end_credits;
	cfg["end_text"] = end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(end_text_duration);
	cfg["difficulty"] = difficulty;
	cfg["random_mode"] = random_mode;

	return cfg;
}

void convert_old_saves(config& cfg){
	if(!cfg.has_child("snapshot")){
		return;
	}

	const config& snapshot = cfg.child("snapshot");
	const config& replay_start = cfg.child("replay_start");
	const config& replay = cfg.child("replay");

	if(!cfg.has_child("carryover_sides") && !cfg.has_child("carryover_sides_start")){
		config carryover;
		//copy rng and menu items from toplevel to new carryover_sides
		carryover["random_seed"] = cfg["random_seed"];
		carryover["random_calls"] = cfg["random_calls"];
		BOOST_FOREACH(const config& menu_item, cfg.child_range("menu_item")){
			carryover.add_child("menu_item", menu_item);
		}
		carryover["difficulty"] = cfg["difficulty"];
		carryover["random_mode"] = cfg["random_mode"];
		//the scenario to be played is always stored as next_scenario in carryover_sides_start
		carryover["next_scenario"] = cfg["scenario"];

		config carryover_start = carryover;

		//copy sides from either snapshot or replay_start to new carryover_sides
		if(!snapshot.empty()){
			BOOST_FOREACH(const config& side, snapshot.child_range("side")){
				carryover.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, snapshot.child_range("player")){
				carryover.add_child("side", side);
			}
			//save the sides from replay_start in carryover_sides_start
			BOOST_FOREACH(const config& side, replay_start.child_range("side")){
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, replay_start.child_range("player")){
				carryover_start.add_child("side", side);
			}
		} else if (!replay_start.empty()){
			BOOST_FOREACH(const config& side, replay_start.child_range("side")){
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, replay_start.child_range("player")){
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
		}

		//get variables according to old hierarchy and copy them to new carryover_sides
		if(!snapshot.empty()){
			if(const config& variables = snapshot.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", replay_start.child_or_empty("variables"));
			} else if (const config& variables = cfg.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", variables);
			}
		} else if (!replay_start.empty()){
			if(const config& variables = replay_start.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", variables);
			}
		} else {
			carryover.add_child("variables", cfg.child("variables"));
			carryover_start.add_child("variables", cfg.child("variables"));
		}

		cfg.add_child("carryover_sides", carryover);
		cfg.add_child("carryover_sides_start", carryover_start);
	}

	//if replay and snapshot are empty we've got a start of scenario save and don't want replay_start either
	if(replay.empty() && snapshot.empty()){
		LOG_RG<<"removing replay_start \n";
		cfg.remove_child("replay_start", 0);
	}

	//remove empty replay or snapshot so type of save can be detected more easily
	if(replay.empty()){
		LOG_RG<<"removing replay \n";
		cfg.remove_child("replay", 0);
	}

	if(snapshot.empty()){
		LOG_RG<<"removing snapshot \n";
		cfg.remove_child("snapshot", 0);
	}
	//?-1.11.? end
	//1.12-1.13 begin

	if(config& carryover_sides_start = cfg.child("carryover_sides_start"))
	{
		if(!carryover_sides_start.has_attribute("next_underlying_unit_id"))
		{
			carryover_sides_start["next_underlying_unit_id"] = cfg["next_underlying_unit_id"];
		}
	}

	if(config& snapshot = cfg.child("snapshot"))
	{
		//make [end_level] -> [end_level_data] since its alo called [end_level_data] in the carryover.
		if(config& end_level = cfg.child("end_level") )
		{
			snapshot.add_child("end_level_data", end_level);
			snapshot.remove_child("end_level",0);
		}
	}

	//1.12-1.13 end
	LOG_RG<<"cfg after conversion "<<cfg<<"\n";
}
