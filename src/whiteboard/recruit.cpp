/*
 Copyright (C) 2010 - 2018 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 */

#include "whiteboard/recruit.hpp"

#include "whiteboard/manager.hpp"
#include "whiteboard/side_actions.hpp"
#include "whiteboard/utility.hpp"
#include "whiteboard/visitor.hpp"

#include "fake_unit_manager.hpp"
#include "fake_unit_ptr.hpp"
#include "menu_events.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "units/map.hpp"
#include "units/types.hpp"

namespace wb
{

std::ostream& operator<<(std::ostream& s, recruit_ptr recruit)
{
	assert(recruit);
	return recruit->print(s);
}
std::ostream& operator<<(std::ostream& s, recruit_const_ptr recruit)
{
	assert(recruit);
	return recruit->print(s);
}

std::ostream& recruit::print(std::ostream &s) const
{
	s << "Recruiting " << unit_name_ << " on hex " << recruit_hex_;
	return s;
}

recruit::recruit(size_t team_index, bool hidden, const std::string& unit_name, const map_location& recruit_hex):
		action(team_index,hidden),
		unit_name_(unit_name),
		recruit_hex_(recruit_hex),
		temp_unit_(create_corresponding_unit()), //auto-ptr ownership transfer
		fake_unit_(unit_ptr(new unit(*temp_unit_))), //temp_unit_ *copied* into new fake unit
		cost_(0)
{
	this->init();
}

recruit::recruit(const config& cfg, bool hidden)
	: action(cfg,hidden)
	, unit_name_(cfg["unit_name_"])
	, recruit_hex_(cfg.child("recruit_hex_")["x"],cfg.child("recruit_hex_")["y"], wml_loc())
	, temp_unit_()
	, fake_unit_()
	, cost_(0)
{
	// Validate unit_name_
	if(!unit_types.find(unit_name_))
		throw action::ctor_err("recruit: Invalid recruit unit type");

	// Construct temp_unit_ and fake_unit_
	temp_unit_ = create_corresponding_unit(); //auto-ptr ownership transfer
	fake_unit_.reset(unit_ptr (new unit(*temp_unit_))), //temp_unit_ copied into new fake_unit

	this->init();
}

void recruit::init()
{
	fake_unit_->set_location(recruit_hex_);
	fake_unit_->set_movement(0, true);
	fake_unit_->set_attacks(0);
	fake_unit_->anim_comp().set_ghosted(false);
	fake_unit_.place_on_fake_unit_manager(resources::fake_units);

	cost_ = fake_unit_->type().cost();
}

recruit::~recruit()
{
}

void recruit::accept(visitor& v)
{
	v.visit(shared_from_this());
}

void recruit::execute(bool& success, bool& complete)
{
	assert(valid());
	temporary_unit_hider const raii(*fake_unit_);
	const int side_num = team_index() + 1;
	//Give back the spent gold so we don't get "not enough gold" message
	resources::gameboard->teams().at(team_index()).get_side_actions()->change_gold_spent_by(-cost_);
	bool const result = resources::controller->get_menu_handler().do_recruit(unit_name_, side_num, recruit_hex_);
	//If it failed, take back the gold
	if (!result) {
		resources::gameboard->teams().at(team_index()).get_side_actions()->change_gold_spent_by(cost_);
	}
	success = complete = result;
}

void recruit::apply_temp_modifier(unit_map& unit_map)
{
	assert(valid());
	temp_unit_->set_location(recruit_hex_);

	DBG_WB << "Inserting future recruit [" << temp_unit_->id()
			<< "] at position " << temp_unit_->get_location() << ".\n";

	// Add cost to money spent on recruits.
	resources::gameboard->teams().at(team_index()).get_side_actions()->change_gold_spent_by(cost_);

	// Temporarily insert unit into unit_map
	// unit map takes ownership of temp_unit
	unit_map.insert(temp_unit_);

	// Update gold in the top bar
	display::get_singleton()->invalidate_game_status();
}

void recruit::remove_temp_modifier(unit_map& unit_map)
{
	//Unit map gives back ownership of temp_unit_
	temp_unit_ = unit_map.extract(recruit_hex_);
	assert(temp_unit_.get());
}

void recruit::draw_hex(const map_location& hex)
{
	if (hex == recruit_hex_)
	{
		const double x_offset = 0.5;
		const double y_offset = 0.7;
		//position 0,0 in the hex is the upper left corner
		std::stringstream number_text;
		number_text << font::unicode_minus << cost_;
		size_t font_size = 16;
		color_t color {255, 0, 0}; //red
		display::get_singleton()->draw_text_in_hex(hex, display::LAYER_ACTIONS_NUMBERING,
						number_text.str(), font_size, color, x_offset, y_offset);
	}
}

void recruit::redraw()
{
	display::get_singleton()->invalidate(recruit_hex_);
}


unit_ptr recruit::create_corresponding_unit()
{
	unit_type const* type = unit_types.find(unit_name_);
	assert(type);
	int side_num = team_index() + 1;
	//real_unit = false needed to avoid generating random traits and causing OOS
	bool real_unit = false;
	unit_ptr result(new unit(*type, side_num, real_unit));
	result->set_movement(0, true);
	result->set_attacks(0);
	return result; //ownership gets transferred to returned unique_ptr copy
}

action::error recruit::check_validity() const
{
	//Check that destination hex is still free
	if(resources::gameboard->units().find(recruit_hex_) != resources::gameboard->units().end()) {
		return LOCATION_OCCUPIED;
	}
	//Check that unit to recruit is still in side's recruit list
	//FIXME: look at leaders extra_recruit too.
	const std::set<std::string>& recruits = resources::gameboard->teams()[team_index()].recruits();
	if(recruits.find(unit_name_) == recruits.end()) {
		return UNIT_UNAVAILABLE;
	}
	//Check that there is still enough gold to recruit this unit
	if(temp_unit_->cost() > resources::gameboard->teams()[team_index()].gold()) {
		return NOT_ENOUGH_GOLD;
	}
	//Check that there is a leader available to recruit this unit
	if(!find_recruiter(team_index(),get_recruit_hex())) {
		return NO_LEADER;
	}

	return OK;
}

config recruit::to_config() const
{
	config final_cfg = action::to_config();

	final_cfg["type"] = "recruit";
	final_cfg["unit_name_"] = unit_name_;
//	final_cfg["temp_cost_"] = temp_cost_; //Unnecessary

	config loc_cfg;
	loc_cfg["x"]=recruit_hex_.wml_x();
	loc_cfg["y"]=recruit_hex_.wml_y();
	final_cfg.add_child("recruit_hex_", std::move(loc_cfg));

	return final_cfg;
}

void recruit::do_hide() {fake_unit_->set_hidden(true);}
void recruit::do_show() {fake_unit_->set_hidden(false);}

}
