//* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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
#include "filesystem.hpp"
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

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

wml_menu_item::wml_menu_item(const std::string& id, const config* cfg) :
		name(),
		event_id(id),
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

wmi_container::wmi_container()
	: wml_menu_items_()
{}

wmi_container::wmi_container(std::map<std::string, wml_menu_item*> wml_menu_items)
	: wml_menu_items_(wml_menu_items)
{}

wmi_container::wmi_container(const wmi_container& container)
	: wml_menu_items_()
{
	clear_wmi();
	std::map<std::string, wml_menu_item*>::const_iterator itor;
	for (itor = container.wml_menu_items_.begin(); itor != container.wml_menu_items_.end(); ++itor) {
		wml_menu_item*& mref = wml_menu_items_[itor->first];
		mref = new wml_menu_item(*(itor->second));
	}
}


void wmi_container::clear_wmi()
{
	for (std::map<std::string, wml_menu_item *>::iterator i = wml_menu_items_.begin(),
	     i_end = wml_menu_items_.end(); i != i_end; ++i)
	{
		delete i->second;
	}
	wml_menu_items_.clear();
}

void wmi_container::to_config(config& cfg){
	for(std::map<std::string, wml_menu_item *>::const_iterator j=wml_menu_items_.begin();
		j!=wml_menu_items_.end(); j++) {
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

void wmi_container::set_menu_items(const config& cfg){
	clear_wmi();
	BOOST_FOREACH(const config &item, cfg.child_range("menu_item"))
	{
		if(!item.has_attribute("id")){ continue; }

		std::string id = item["id"];
		wml_menu_item*& mref = wml_menu_items_[id];
		if(mref == NULL) {
			mref = new wml_menu_item(id, &item);
		} else {
			WRN_NG << "duplicate menu item (" << id << ") while loading from config\n";
		}
	}
}

carryover::carryover(const config& side)
		: add_(side["add"].to_bool())
		, color_(side["color"])
		, current_player_(side["current_player"])
		, gold_(side["gold"].to_int())
		, name_(side["name"])
		, previous_recruits_()
		, recall_list_()
		, save_id_(side["save_id"])
{
	std::vector<std::string> temp_recruits = utils::split(side["previous_recruits"], ',');
	previous_recruits_.insert(temp_recruits.begin(), temp_recruits.end());

	BOOST_FOREACH(const config& u, side.child_range("unit")){
		recall_list_.push_back(unit(u));
	}
}

carryover::carryover(const team& t, const int gold, const bool add)
		: add_ (add)
		, color_(t.color())
		, current_player_(t.current_player())
		, gold_(gold)
		, name_(t.name())
		, previous_recruits_(t.recruits())
		, recall_list_(t.recall_list())
		, save_id_(t.save_id())
		{}

static const int default_gold_qty = 100;

void carryover::transfer_all_gold_to(config& side_cfg){

	int cfg_gold = side_cfg["gold"].to_int();

	if(side_cfg["gold"].empty()) {
		cfg_gold = default_gold_qty;
		side_cfg["gold"] = cfg_gold;
	}

	if(add_){
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
	BOOST_FOREACH(unit& u, recall_list_){
		config& new_unit = side_cfg.add_child("unit");
		u.write(new_unit);
	}
	recall_list_.clear();
}

std::string carryover::get_recruits(bool erase){
	std::stringstream can_recruit;
	for(std::set<std::string>::iterator i = previous_recruits_.begin(); i != previous_recruits_.end(); i++){
		can_recruit << *i << ",";
		if(erase){
			previous_recruits_.erase(i);
		}
	}
	std::string can_recruit_str = can_recruit.str();
	// Remove the trailing comma
	if(can_recruit_str.empty() == false) {
		can_recruit_str.resize(can_recruit_str.size()-1);
	}
	return can_recruit_str;
}

void carryover::update_carryover(const team& t, const int gold, const bool add){
	gold_ += gold;
	add_ = add;
	color_ = t.color();
	current_player_ = t.current_player();
	name_ = t.name();
	previous_recruits_.insert(t.recruits().begin(), t.recruits().end());
	recall_list_.insert(recall_list_.end(), t.recall_list().begin(), t.recall_list().end());
}

void carryover::initialize_team(config& side_cfg){
	transfer_all_gold_to(side_cfg);
}

const std::string carryover::to_string(){
	std::string side = "";
	side.append("Side " + save_id_ + ": gold " + str_cast<int>(gold_) + " recruits " + get_recruits(false) + " units ");
	BOOST_FOREACH(const unit& u, recall_list_){
		side.append(u.name() + ", ");
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
	BOOST_FOREACH(unit& u, recall_list_){
		config& unit_cfg = side.add_child("unit");
		u.write(unit_cfg);
	}
}

carryover_info::carryover_info(const config& cfg)
	: carryover_sides_()
	, end_level_()
	, variables_(cfg.child_or_empty("variables"))
	, rng_(cfg)
	, wml_menu_items_()
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
}

void carryover_info::transfer_from(game_data& gamedata){
	variables_ = gamedata.get_variables();
	wml_menu_items_ = gamedata.get_wml_menu_items();
	rng_ = gamedata.rng();
}

void carryover_info::transfer_to(config& level){
	//if the game has been loaded from a snapshot, the existing variables will be the current ones
	if(!level.has_child("variables")) {
		level.add_child("variables", variables_);
	}

	level["random_seed"] = str_cast<int>(rng_.get_random_seed());
	level["random_calls"] = str_cast<int>(rng_.get_random_calls());

	if(!level.has_child("menu_item")){
		wml_menu_items_.to_config(level);
	}

}

const config carryover_info::to_config() {
	config cfg;
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

	static const std::string default_gold_qty_;

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


void init(){
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
			"hidden", "music", "color", "ai_config", "gold",
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

game_data::game_data()
		: scoped_variables()
		, last_selected(map_location::null_location)
		, wml_menu_items_()
		, rng_()
		, variables_()
		, temporaries_()
		, generator_setter_(&recorder)
		, phase_(INITIAL)
		, can_end_turn_(true)
		{}

game_data::game_data(const config& level)
		: scoped_variables()
		, last_selected(map_location::null_location)
		, wml_menu_items_()
		, rng_(level)
		, variables_()
		, temporaries_()
		, generator_setter_(&recorder)
		, phase_(INITIAL)
		, can_end_turn_(true)
{
	wml_menu_items_.set_menu_items(level);
	can_end_turn_ = level["can_end_turn"].to_bool(true);

	if(const config &vars = level.child("variables")){
		set_variables(vars);
	}
}

game_data::game_data(const game_data& data)
		: variable_set() // Not sure why empty, copied from old code
		, scoped_variables(data.scoped_variables)
		, last_selected(data.last_selected)
		, wml_menu_items_(data.wml_menu_items_)
		, rng_(data.rng_)
		, variables_(data.variables_)
		, temporaries_()
		, generator_setter_(data.generator_setter_)
		, phase_(data.phase_)
		, can_end_turn_(data.can_end_turn_)
{}

game_data::~game_data(){
	wml_menu_items_.clear_wmi();
};

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

void game_data::set_variables(const config& vars) {
	variables_ = vars;
}

void game_data::set_vars(const config& cfg){
	if(cfg.has_child("random_calls")){
		rng_.seed_random(cfg["random_calls"]);
	}

	if(const config &vars = cfg.child("variables")){
		set_variables(vars);
	}
}

void game_data::write_snapshot(config& cfg){
	cfg["can_end_turn"] = can_end_turn_;

	cfg["random_seed"] = rng_.get_random_seed();
	cfg["random_calls"] = rng_.get_random_calls();

	cfg.add_child("variables", variables_);

	wml_menu_items_.to_config(cfg);
}

void game_data::write_config(config_writer& out, bool write_variables){
	out.write_key_val("random_seed", lexical_cast<std::string>(rng_.get_random_seed()));
	out.write_key_val("random_calls", lexical_cast<std::string>(rng_.get_random_calls()));
	if (write_variables) {
		out.write_child("variables", variables_);
	}

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
	scenario(),
	next_scenario(),
	completion(),
	end_credits(true),
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
	end_credits(cfg["end_credits"].to_bool(true)),
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
	end_credits(gc.end_credits),
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
	cfg["end_credits"] = end_credits;
	cfg["end_text"] = end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(end_text_duration);
	cfg["difficulty"] = difficulty;

	return cfg;
}

game_state::game_state()  :
		replay_data(),
		starting_pos(),
		snapshot(),
		carryover_sides(carryover_info().to_config()),
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
		replay_data(),
		starting_pos(),
		snapshot(),
	//	carryover_sides(cfg.child_or_empty("carryover_sides")),
		classification_(cfg),
		mp_settings_(cfg)
{
	n_unit::id_manager::instance().set_save_id(cfg["next_underlying_unit_id"]);
	log_scope("read_game");

	carryover_info sides(cfg.child_or_empty("carryover_sides"));

	const config &snapshot = cfg.child("snapshot");
	const config &replay_start = cfg.child("replay_start");
	// We're loading a snapshot if we have it and the user didn't request a replay.
	bool load_snapshot = !show_replay && snapshot && !snapshot.empty();

	if (load_snapshot) {
		this->snapshot = snapshot;
		//for backwards compatibility
		if(snapshot.has_child("variables")){
		sides.set_variables(snapshot.child("variables"));
		}
	} else {
		assert(replay_start);
		//for backwards compatibility
		if(replay_start.has_child("variables")){
			sides.set_variables(replay_start.child("variables"));
		}
	}

	//for backwards compatibility
	if(cfg.has_child("variables")){
		sides.set_variables(cfg.child("variables"));
	}

	carryover_sides = sides.to_config();

	LOG_NG << "scenario: '" << classification_.scenario << "'\n";
	LOG_NG << "next_scenario: '" << classification_.next_scenario << "'\n";

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

	cfg["end_credits"] = classification_.end_credits;
	cfg["end_text"] = classification_.end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(classification_.end_text_duration);

	if(resources::gamedata != NULL){
		resources::gamedata->write_snapshot(cfg);
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
	starting_pos(state.starting_pos),
	snapshot(state.snapshot),
	carryover_sides(state.carryover_sides),
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


void game_state::write_config(config_writer& out, bool write_variables) const
{
	out.write(classification_.to_config());
	if (classification_.campaign_type == "multiplayer")
		out.write_child("multiplayer", mp_settings_.to_config());

	if(resources::gamedata != NULL){
		resources::gamedata->write_config(out, write_variables);
	}

	if (!replay_data.child("replay")) {
		out.write_child("replay", replay_data);
	}

	out.write_child("replay_start",starting_pos);
}

