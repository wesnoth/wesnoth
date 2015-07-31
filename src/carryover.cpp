/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "carryover.hpp"

#include "global.hpp"

#include "config.hpp"
#include "team.hpp"
#include "unit.hpp"
#include <boost/foreach.hpp>
#include <cassert>

carryover::carryover(const config& side)
		: add_(!side["carryover_add"].empty() ? side["carryover_add"].to_bool() : side["add"].to_bool())
		, current_player_(side["current_player"])
		, gold_(!side["carryover_gold"].empty() ? side["carryover_gold"].to_int() : side["gold"].to_int())
		// if we load it from a snapshot we need to read the recruits from "recruits" and not from "previous_recruits".
		, previous_recruits_(side.has_attribute("recruit") ? utils::set_split(side["recruit"]) :utils::set_split(side["previous_recruits"]))
		, recall_list_()
		, save_id_(side["save_id"])
{
	BOOST_FOREACH(const config& u, side.child_range("unit")){
		recall_list_.push_back(u);
		config& u_back = recall_list_.back();
		u_back.remove_attribute("side");
		u_back.remove_attribute("goto_x");
		u_back.remove_attribute("goto_y");
		u_back.remove_attribute("x");
		u_back.remove_attribute("y");
	}
}

carryover::carryover(const team& t, const int gold, const bool add)
		: add_ (add)
		, current_player_(t.current_player())
		, gold_(gold)
		, previous_recruits_(t.recruits())
		, recall_list_()
		, save_id_(t.save_id())
{
	BOOST_FOREACH(const unit_const_ptr & u, t.recall_list()) {
		recall_list_.push_back(config());
		u->write(recall_list_.back());
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
	side["current_player"] = current_player_;
	side["previous_recruits"] = get_recruits(false);
	BOOST_FOREACH(const config & u_cfg, recall_list_)
		side.add_child("unit", u_cfg);
}

carryover_info::carryover_info(const config& cfg, bool from_snpashot)
	: carryover_sides_()
	, variables_(cfg.child_or_empty("variables"))
	, rng_(cfg)
	, wml_menu_items_()
	, next_scenario_(cfg["next_scenario"])
	, next_underlying_unit_id_(cfg["next_underlying_unit_id"].to_int(0))
{
	BOOST_FOREACH(const config& side, cfg.child_range("side"))
	{
		if(side["lost"].to_bool(false) || !side["persistent"].to_bool(true))
		{
			//this shouldnt happen outside a snpshot.
			assert(from_snpashot);
			continue;
		}
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

struct save_id_equals
{
	save_id_equals(std::string val) : value (val) {}
	bool operator () (carryover& v2)
	{
		return value == v2.get_save_id();
	}

	std::string value;
};

void carryover_info::transfer_all_to(config& side_cfg){
	if(side_cfg["save_id"].empty()){
		side_cfg["save_id"] = side_cfg["id"];
	}
	std::vector<carryover>::iterator iside = std::find_if(
		carryover_sides_.begin(),
		carryover_sides_.end(),
		save_id_equals(side_cfg["save_id"])
	);
	if(iside != carryover_sides_.end())
	{
		iside->transfer_all_gold_to(side_cfg);
		iside->transfer_all_recalls_to(side_cfg);
		iside->transfer_all_recruits_to(side_cfg);
		carryover_sides_.erase(iside);
		return;
	}
	else
	{
		//if no carryover was found for this side, check if starting gold is defined
		if(!side_cfg.has_attribute("gold") || side_cfg["gold"].empty()){
			side_cfg["gold"] = default_gold_qty;
		}
	}
}

void carryover_info::transfer_to(config& level){
	if(!level.has_attribute("next_underlying_unit_id"))
	{
		level["next_underlying_unit_id"] = next_underlying_unit_id_;
	}

	//if the game has been loaded from a snapshot, the existing variables will be the current ones
	if(!level.has_child("variables")) {
		level.add_child("variables", variables_);
	}

	config::attribute_value & seed_value = level["random_seed"];
	if ( seed_value.empty() ) {
		seed_value = rng_.get_random_seed_str();
		level["random_calls"] = rng_.get_random_calls();
	}

	if(!level.has_child("menu_item")){
		wml_menu_items_.to_config(level);
	}

	next_scenario_ = "";

}

const config carryover_info::to_config()
{
	config cfg;
	cfg["next_underlying_unit_id"] = next_underlying_unit_id_;
	cfg["next_scenario"] = next_scenario_;

	BOOST_FOREACH(carryover& c, carryover_sides_){
		c.to_config(cfg);
	}

	cfg["random_seed"] = rng_.get_random_seed_str();
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


void carryover_info::merge_old_carryover(const carryover_info& old_carryover)
{
	BOOST_FOREACH(const carryover & old_side, old_carryover.carryover_sides_)
	{
		std::vector<carryover>::iterator iside = std::find_if(
			carryover_sides_.begin(),
			carryover_sides_.end(),
			save_id_equals(old_side.get_save_id())
			);
		//add the side if don't already have it.
		if(iside == carryover_sides_.end())
		{
			this->carryover_sides_.push_back(old_side);
		}
	}
}

