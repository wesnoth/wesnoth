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
#include "filesystem.hpp"
#include "formula_string_utils.hpp"
#include "game_config.hpp"
#include "game_events/handlers.hpp"
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


carryover::carryover(const config& side)
		: add_(side["add"].to_bool())
		, color_(side["color"])
		, current_player_(side["current_player"])
		, gold_(side["gold"].to_int())
		, name_(side["name"])
		, previous_recruits_(utils::set_split(side["previous_recruits"]))
		, recall_list_()
		, save_id_(side["save_id"])
{
	BOOST_FOREACH(const config& u, side.child_range("unit")){
		recall_list_.push_back(u);
	}
}

carryover::carryover(const team& t, const int gold, const bool add)
		: add_ (add)
		, color_(t.color())
		, current_player_(t.current_player())
		, gold_(gold)
		, name_(t.name())
		, previous_recruits_(t.recruits())
		, recall_list_()
		, save_id_(t.save_id())
{
	BOOST_FOREACH(const unit& u, t.recall_list()) {
		recall_list_.push_back(config());
		u.write(recall_list_.back());
	}
}

static const int default_gold_qty = 100;

void carryover::transfer_all_gold_to(config& side_cfg){

	int cfg_gold = side_cfg["gold"].to_int();

	if(side_cfg["gold"].empty()) {
		cfg_gold = default_gold_qty;
		side_cfg["gold"] = cfg_gold;
	}

	if(add_ && gold_ > 0){
		side_cfg["gold"] = cfg_gold + gold_;
	}
	else if(gold_ > cfg_gold){
		side_cfg["gold"] = gold_;
	}

	gold_ = 0;
}

void carryover::transfer_all_recruits_to(config& side_cfg){
	std::string can_recruit_str = utils::join(previous_recruits_, ",");
	previous_recruits_.clear();
	side_cfg["previous_recruits"] = can_recruit_str;
}

void carryover::transfer_all_recalls_to(config& side_cfg){
	BOOST_FOREACH(const config & u_cfg, recall_list_) {
		side_cfg.add_child("unit", u_cfg);
	}
	recall_list_.clear();
}

std::string carryover::get_recruits(bool erase){
	// Join the previous recruits into a string.
	std::string can_recruit_str = utils::join(previous_recruits_);
	if ( erase )
		// Clear the previous recruits.
		previous_recruits_.clear();

	return can_recruit_str;
}

void carryover::update_carryover(const team& t, const int gold, const bool add){
	gold_ += gold;
	add_ = add;
	color_ = t.color();
	current_player_ = t.current_player();
	name_ = t.name();
	previous_recruits_.insert(t.recruits().begin(), t.recruits().end());
	BOOST_FOREACH(const unit& u, t.recall_list()) {
		recall_list_.push_back(config());
		u.write(recall_list_.back());
	}
}

void carryover::initialize_team(config& side_cfg){
	transfer_all_gold_to(side_cfg);
}

const std::string carryover::to_string(){
	std::string side = "";
	side.append("Side " + save_id_ + ": gold " + str_cast<int>(gold_) + " recruits " + get_recruits(false) + " units ");
	BOOST_FOREACH(const config & u_cfg, recall_list_) {
		side.append(u_cfg["name"].str() + ", ");
	}
	return side;
}

void carryover::to_config(config& cfg){
	config& side = cfg.add_child("side");
	side["save_id"] = save_id_;
	side["gold"] = gold_;
	side["add"] = add_;
	side["color"] = color_;
	side["current_player"] = current_player_;
	side["name"] = name_;
	side["previous_recruits"] = get_recruits(false);
	BOOST_FOREACH(const config & u_cfg, recall_list_)
		side.add_child("unit", u_cfg);
}

carryover_info::carryover_info(const config& cfg)
	: carryover_sides_()
	, end_level_()
	, variables_(cfg.child_or_empty("variables"))
	, rng_(cfg)
	, wml_menu_items_()
	, difficulty_(cfg["difficulty"].empty() ? DEFAULT_DIFFICULTY : cfg["difficulty"].str())
	, random_mode_(cfg["random_mode"].str())
	, next_scenario_(cfg["next_scenario"])
{
	end_level_.read(cfg.child_or_empty("end_level_data"));
	BOOST_FOREACH(const config& side, cfg.child_range("side")){
		this->carryover_sides_.push_back(carryover(side));
	}

	wml_menu_items_.set_menu_items(cfg);
}

std::vector<carryover>& carryover_info::get_all_sides() {
	return carryover_sides_;
}

void carryover_info::add_side(const config& cfg) {
	carryover_sides_.push_back(carryover(cfg));
}

void carryover_info::remove_side(const std::string& id) {
	for (std::vector<carryover>::iterator it = carryover_sides_.begin();
		it != carryover_sides_.end(); ++it) {

		if (it->get_save_id() == id) {
			carryover_sides_.erase(it);
			break;
		}
	}
}

const end_level_data& carryover_info::get_end_level() const{
	return end_level_;
}

void carryover_info::transfer_from(const team& t, int carryover_gold){
	BOOST_FOREACH(carryover& side, carryover_sides_){
		if(side.get_save_id() == t.save_id()){
			side.update_carryover(t, carryover_gold, end_level_.carryover_add);
			return;
		}
	}

	carryover_sides_.push_back(carryover(t, carryover_gold, end_level_.carryover_add));
}

void carryover_info::transfer_all_to(config& side_cfg){
	if(side_cfg["save_id"].empty()){
		side_cfg["save_id"] = side_cfg["id"];
	}
	BOOST_FOREACH(carryover& side, carryover_sides_){
		if(side.get_save_id() == side_cfg["save_id"]){
			side.transfer_all_gold_to(side_cfg);
			side.transfer_all_recalls_to(side_cfg);
			side.transfer_all_recruits_to(side_cfg);
			return;
		}
	}

	//if no carryover was found for this side, check if starting gold is defined
	if(!side_cfg.has_attribute("gold") || side_cfg["gold"].empty()){
		side_cfg["gold"] = default_gold_qty;
	}
}

void carryover_info::transfer_from(game_data& gamedata){
	variables_ = gamedata.get_variables();
	wml_menu_items_ = gamedata.get_wml_menu_items();
	rng_ = gamedata.rng();
	difficulty_ = gamedata.difficulty();
	random_mode_ = gamedata.random_mode();
	next_scenario_ = gamedata.next_scenario();
}

void carryover_info::transfer_to(config& level){
	//if the game has been loaded from a snapshot, the existing variables will be the current ones
	if(!level.has_child("variables")) {
		level.add_child("variables", variables_);
	}

	config::attribute_value & seed_value = level["random_seed"];
	if ( seed_value.empty() ) {
		seed_value = rng_.get_random_seed();
		level["random_calls"] = rng_.get_random_calls();
	}

	if(!level.has_child("menu_item")){
		wml_menu_items_.to_config(level);
	}

	if(level["difficulty"].empty()){
		level["difficulty"] = difficulty_;
	}
	if(level["random_mode"].empty()){
		level["random_mode"] = random_mode_;
	}
	difficulty_ = "";
	next_scenario_ = "";

}

const config carryover_info::to_config() 
{
	config cfg;

	cfg["difficulty"] = difficulty_;
	cfg["random_mode"] = random_mode_;
	cfg["next_scenario"] = next_scenario_;

	BOOST_FOREACH(carryover& c, carryover_sides_){
		c.to_config(cfg);
	}
	config& end_level = cfg.add_child("end_level_data");
	end_level_.write(end_level);

	cfg["random_seed"] = rng_.get_random_seed();
	cfg["random_calls"] = rng_.get_random_calls();

	cfg.add_child("variables", variables_);

	wml_menu_items_.to_config(cfg);

	return cfg;
}

carryover* carryover_info::get_side(std::string save_id){
	BOOST_FOREACH(carryover& side, carryover_sides_){
		if(side.get_save_id() == save_id){
			return &side;
		}
	}
	return NULL;
}

class team_builder {
public:
	team_builder(const config& side_cfg,
		     const std::string &save_id, std::vector<team>& teams,
		     const config& level, gamemap& map, unit_map& units,
		     bool snapshot, const config &starting_pos)
		: gold_info_ngold_(0)
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

	int gold_info_ngold_;
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

		player_cfg_ = NULL;
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


	bool use_player_cfg() const
	{
		return (player_cfg_ != NULL) && (!snapshot_);
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
		// Make a persistent copy of the config.
		leader_configs_.push_back(leader);
		config & stored = leader_configs_.back();

		// Remove the attributes used to define a side.
		for ( size_t i = 0; team::attributes[i] != NULL; ++i ) {
			stored.remove_attribute(team::attributes[i]);
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
		log_step("prepare units");
		if (use_player_cfg()) {
			//units in [replay_start][side] merged with [side]
			//only relevant in start-of-scenario saves, that's why !shapshot
			//units that are in '[scenario][side]' are 'first'
			//for create-or-recall semantics to work: for each unit with non-empty
			//id, unconditionally put OTHER, later, units with same id directly to
			//recall list, not including them in unit_configs_
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
		, last_selected(map_location::null_location)
		, wml_menu_items_()
		, rng_()
		, variables_()
		, temporaries_()
		, phase_(INITIAL)
		, can_end_turn_(true)
		, difficulty_(DEFAULT_DIFFICULTY)
		, random_mode_("")
		, scenario_()
		, next_scenario_()
		{}

game_data::game_data(const config& level)
		: scoped_variables()
		, last_selected(map_location::null_location)
		, wml_menu_items_()
		, rng_(level)
		, variables_(level.child_or_empty("variables"))
		, temporaries_()
		, phase_(INITIAL)
		, can_end_turn_(level["can_end_turn"].to_bool(true))
		, difficulty_(level["difficulty"].empty() ? DEFAULT_DIFFICULTY : level["difficulty"].str())
		, random_mode_(level["random_mode"].str())
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
		, difficulty_(data.difficulty_)
		, random_mode_(data.random_mode_)
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

void game_data::write_snapshot(config& cfg){
	cfg["difficulty"] = difficulty_;
	cfg["random_mode"] = random_mode_;
	cfg["scenario"] = scenario_;
	cfg["next_scenario"] = next_scenario_;

	cfg["can_end_turn"] = can_end_turn_;

	cfg["random_seed"] = rng_.get_random_seed();
	cfg["random_calls"] = rng_.get_random_calls();

	cfg.add_child("variables", variables_);

	wml_menu_items_.to_config(cfg);
}

void game_data::write_config(config_writer& out){
	out.write_key_val("difficulty", difficulty_);
	out.write_key_val("random_mode", random_mode_);
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
					 bool snapshot, const config& starting_pos)
{
	return team_builder_ptr(new team_builder(side_cfg,save_id,teams,level,map,units,snapshot,starting_pos));
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
	parent(),
	version(),
	campaign_type(),
	campaign_define(),
	campaign_xtra_defines(),
	campaign(),
	history(),
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
	parent(cfg["parent"]),
	version(cfg["version"]),
	campaign_type(cfg["campaign_type"].empty() ? "scenario" : cfg["campaign_type"].str()),
	campaign_define(cfg["campaign_define"]),
	campaign_xtra_defines(utils::split(cfg["campaign_extra_defines"])),
	campaign(cfg["campaign"]),
	history(cfg["history"]),
	abbrev(cfg["abbrev"]),
	completion(cfg["completion"]),
	end_credits(cfg["end_credits"].to_bool(true)),
	end_text(cfg["end_text"]),
	end_text_duration(cfg["end_text_duration"]),
	difficulty(cfg["difficulty"]),
	random_mode(cfg["random_mode"])
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
	cfg["parent"] = parent;
	cfg["version"] = game_config::version;
	cfg["campaign_type"] = campaign_type;
	cfg["campaign_define"] = campaign_define;
	cfg["campaign_extra_defines"] = utils::join(campaign_xtra_defines);
	cfg["campaign"] = campaign;
	cfg["history"] = history;
	cfg["abbrev"] = abbrev;
	cfg["completion"] = completion;
	cfg["end_credits"] = end_credits;
	cfg["end_text"] = end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(end_text_duration);
	cfg["difficulty"] = difficulty;
	cfg["random_mode"] = random_mode;

	return cfg;
}

game_state::game_state()  :
		replay_data(),
		snapshot(),
		carryover_sides(),
		carryover_sides_start(carryover_info().to_config()),
		replay_start_(),
		classification_(),
		mp_settings_()
		{}

game_state::game_state(const config& cfg, bool show_replay) :
		replay_data(),
		snapshot(),
		carryover_sides(),
		carryover_sides_start(),
		replay_start_(),
		classification_(cfg),
		mp_settings_(cfg)
{
	n_unit::id_manager::instance().set_save_id(cfg["next_underlying_unit_id"]);
	log_scope("read_game");

	if(cfg.has_child("carryover_sides")){
		carryover_sides = cfg.child("carryover_sides");
	}
	if(cfg.has_child("carryover_sides_start")){
		carryover_sides_start = cfg.child("carryover_sides_start");
	}

	if(show_replay){
		//If replay_start and replay_data couldn't be loaded
		if(!load_replay(cfg)){
			//TODO: notify user of failure
			ERR_NG<<"Could not load as replay \n";
		}
	} else {
		if(const config& snapshot = cfg.child("snapshot")){
			this->snapshot = snapshot;
			load_replay(cfg);
		} else if(carryover_sides_start.empty() && !carryover_sides.empty()){
			//if we are loading a start of scenario save and don't have carryover_sides_start, use carryover_sides
			carryover_sides_start = carryover_sides;
		}
		//TODO: check if loading fails completely
	}

	LOG_NG << "scenario: '" << carryover_sides_start["next_scenario"] << "'\n";

	if (const config &stats = cfg.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}

}

bool game_state::load_replay(const config& cfg){
	bool replay_loaded = false;

	if(const config& replay_start = cfg.child("replay_start")){
		replay_start_ = replay_start;
		if(const config& replay = cfg.child("replay")){
			this->replay_data = replay;
			replay_loaded = true;
		}
	}

	return replay_loaded;
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

	LOG_RG<<"cfg after conversion "<<cfg<<"\n";
}

void game_state::write_snapshot(config& cfg, game_display* gui) const
{
	log_scope("write_game");
	if(gui != NULL){
		cfg["snapshot"] = true;
		cfg["playing_team"] = str_cast(gui->playing_team());

		game_events::write_events(cfg);

		sound::write_music_play_list(cfg);
	}

	cfg["label"] = classification_.label;
	cfg["history"] = classification_.history;
	cfg["abbrev"] = classification_.abbrev;
	cfg["version"] = game_config::version;

	cfg["completion"] = classification_.completion;

	cfg["campaign"] = classification_.campaign;
	cfg["campaign_type"] = classification_.campaign_type;

	cfg["campaign_define"] = classification_.campaign_define;
	cfg["campaign_extra_defines"] = utils::join(classification_.campaign_xtra_defines);
	cfg["next_underlying_unit_id"] = str_cast(n_unit::id_manager::instance().get_save_id());

	cfg["end_credits"] = classification_.end_credits;
	cfg["end_text"] = classification_.end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(classification_.end_text_duration);

	cfg["difficulty"] = classification_.difficulty;
	cfg["random_mode"] = classification_.random_mode;
	
	if(resources::gamedata != NULL){
		resources::gamedata->write_snapshot(cfg);
	}

	if(gui != NULL){
		gui->labels().write(cfg);
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

	if(cfg_save.has_child("carryover_sides_start")){
		cfg_summary["scenario"] = cfg_save.child("carryover_sides_start")["next_scenario"];
		cfg_summary["difficulty"] = cfg_save.child("carryover_sides_start")["difficulty"];
		cfg_summary["random_mode"] = cfg_save.child("carryover_sides_start")["random_mode"];
	} else {
		cfg_summary["scenario"] = cfg_save["scenario"];
		cfg_summary["difficulty"] = cfg_save["difficulty"];
		cfg_summary["random_mode"] = cfg_save["random_mode"];
	}
	cfg_summary["campaign"] = cfg_save["campaign"];
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
	//	if (p["canrecruit"].to_bool(false))) {
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
	// We need a binary path-independent path to the leader image here
	// so it can be displayed for campaign-specific units in the dialog
	// even when the campaign isn't loaded yet.
	cfg_summary["leader_image"] = get_independent_image_path(leader_image);

	if(!shrouded) {
		if(has_snapshot) {
			if (!cfg_snapshot.find_child("side", "shroud", "yes")) {
				cfg_summary.add_child("map", cfg_snapshot.child_or_empty("map"));
			}
		} else if(has_replay) {
			if (!cfg_replay_start.find_child("side","shroud","yes")) {
				cfg_summary.add_child("map", cfg_replay_start.child_or_empty("map"));
			}
		}
	}
}

game_state::game_state(const game_state& state) :
	replay_data(state.replay_data),
	snapshot(state.snapshot),
	carryover_sides(state.carryover_sides),
	carryover_sides_start(state.carryover_sides_start),
	replay_start_(state.replay_start_),
	classification_(state.classification_),
	mp_settings_(state.mp_settings_)
{}

game_state& game_state::operator=(const game_state& state)
{
	// Use copy constructor to make sure we are coherent
	if (this != &state) {
		this->~game_state();
		new (this) game_state(state) ;
	}
	return *this ;
}


void game_state::write_config(config_writer& out) const
{
	out.write(classification_.to_config());
	if (classification_.campaign_type == "multiplayer")
		out.write_child("multiplayer", mp_settings_.to_config());
}

