/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "recruit.hpp"

#include "manager.hpp"
#include "side_actions.hpp"
#include "utility.hpp"
#include "visitor.hpp"

#include "game_display.hpp"
#include "menu_events.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "unit_types.hpp"

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
		temp_unit_(create_corresponding_unit()),
		valid_(true),
		fake_unit_(create_corresponding_unit())
{
	this->init();
}

recruit::recruit(config const& cfg, bool hidden)
	: action(cfg,hidden)
	, unit_name_(cfg["unit_name_"])
	, recruit_hex_(cfg.child("recruit_hex_")["x"],cfg.child("recruit_hex_")["y"])
	, temp_unit_()
	, valid_(true)
	, fake_unit_()
{
	// Validate unit_name_
	if(!unit_types.find(unit_name_))
		throw action::ctor_err("recruit: Invalid recruit unit type");

	// Construct temp_unit_ and fake_unit_
	temp_unit_ = create_corresponding_unit();
	fake_unit_.reset(create_corresponding_unit()),

	this->init();
}

void recruit::init()
{
	fake_unit_->set_location(recruit_hex_);
	fake_unit_->set_movement(0);
	fake_unit_->set_attacks(0);
	fake_unit_->set_ghosted(false);
	fake_unit_->place_on_game_display(resources::screen);
}

recruit::~recruit()
{
}

void recruit::accept(visitor& v)
{
	v.visit_recruit(shared_from_this());
}

void recruit::execute(bool& success, bool& complete)
{
	assert(valid_);
	temporary_unit_hider const raii(*fake_unit_);
	int const side_num = team_index() + 1;
	bool const result = resources::controller->get_menu_handler().do_recruit(unit_name_, side_num, recruit_hex_);
	success = complete = result;
}

void recruit::apply_temp_modifier(unit_map& unit_map)
{
	assert(valid_);
	temp_unit_->set_location(recruit_hex_);

	DBG_WB << "Inserting future recruit [" << temp_unit_->id()
			<< "] at position " << temp_unit_->get_location() << ".\n";

	int cost = temp_unit_->type()->cost();
	// Add cost to money spent on recruits.
	resources::teams->at(team_index()).get_side_actions()->change_gold_spent_by(cost);

	// Temporarily insert unit into unit_map
	unit_map.insert(temp_unit_);
	// unit map takes ownership of temp_unit
	temp_unit_ = NULL;

	// Update gold in the top bar
	resources::screen->invalidate_game_status();
}

void recruit::remove_temp_modifier(unit_map& unit_map)
{
	temp_unit_ = unit_map.extract(recruit_hex_);
	assert(temp_unit_);
}

void recruit::draw_hex(map_location const& hex)
{
	if (hex == recruit_hex_)
	{
		const double x_offset = 0.5;
		const double y_offset = 0.7;
		//position 0,0 in the hex is the upper left corner
		std::stringstream number_text;
		number_text << utils::unicode_minus << temp_unit_->type()->cost();
		size_t font_size = 16;
		SDL_Color color; color.r = 255; color.g = 0; color.b = 0; //red
		resources::screen->draw_text_in_hex(hex, display::LAYER_ACTIONS_NUMBERING,
						number_text.str(), font_size, color, x_offset, y_offset);
	}
}

game_display::fake_unit* recruit::create_corresponding_unit()
{
	unit_type const* type = unit_types.find(unit_name_);
	assert(type);
	int side_num = team_index() + 1;
	//real_unit = false needed to avoid generating random traits and causing OOS
	bool real_unit = false;
	game_display::fake_unit* result = new game_display::fake_unit(unit(type, side_num, real_unit));
	result->set_movement(0);
	result->set_attacks(0);
	return result;
}

config recruit::to_config() const
{
	config final_cfg = action::to_config();

	final_cfg["type"] = "recruit";
	final_cfg["unit_name_"] = unit_name_;
//	final_cfg["temp_cost_"] = temp_cost_; //Unnecessary

	config loc_cfg;
	loc_cfg["x"]=recruit_hex_.x;
	loc_cfg["y"]=recruit_hex_.y;
	final_cfg.add_child("recruit_hex_",loc_cfg);

	return final_cfg;
}

void recruit::do_hide() {fake_unit_->set_hidden(true);}
void recruit::do_show() {fake_unit_->set_hidden(false);}

}
