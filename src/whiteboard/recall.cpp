/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "recall.hpp"

#include "manager.hpp"
#include "side_actions.hpp"
#include "utility.hpp"
#include "visitor.hpp"

#include "game_display.hpp"
#include "menu_events.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit.hpp"

namespace wb
{

std::ostream& operator<<(std::ostream& s, recall_ptr recall)
{
	assert(recall);
	return recall->print(s);
}
std::ostream& operator<<(std::ostream& s, recall_const_ptr recall)
{
	assert(recall);
	return recall->print(s);
}

std::ostream& recall::print(std::ostream &s) const
{
	s << "Recalling " << fake_unit_->name() << " [" << fake_unit_->id() << "] on hex " << recall_hex_;
	return s;
}

recall::recall(size_t team_index, const unit& unit, const map_location& recall_hex):
		action(team_index),
		temp_unit_(new class unit(unit)),
		recall_hex_(recall_hex),
		valid_(true),
		fake_unit_(new class unit(unit), wb::fake_unit_deleter()),
		temp_cost_(0)
{
	fake_unit_->set_location(recall_hex_);
	fake_unit_->set_movement(0);
	fake_unit_->set_attacks(0);
	fake_unit_->set_ghosted(false);
	resources::screen->place_temporary_unit(fake_unit_.get());
}

recall::~recall()
{
	delete temp_unit_;
}

void recall::accept(visitor& v)
{
	v.visit_recall(shared_from_this());
}

bool recall::execute()
{
	assert(valid_);
	assert(temp_unit_);
	fake_unit_->set_hidden(true);
	resources::controller->get_menu_handler().do_recall(*temp_unit_, team_index() + 1, recall_hex_);
	delete temp_unit_;
	temp_unit_ = NULL;
	return true;
}

void recall::apply_temp_modifier(unit_map& unit_map)
{
	assert(valid_);
	temp_unit_->set_location(recall_hex_);

	DBG_WB << "Inserting future recall " << temp_unit_->name() << " [" << temp_unit_->id()
			<< "] at position " << temp_unit_->get_location() << ".\n";

	//temporarily remove unit from recall list
	std::vector<unit>& recalls = resources::teams->at(team_index()).recall_list();
	std::vector<unit>::iterator it = std::find_if(recalls.begin(), recalls.end(),
					unit_comparator_predicate(*temp_unit_));
	assert(it != recalls.end());
	recalls.erase(it);

	// Temporarily insert unit into unit_map
	unit_map.insert(temp_unit_);
	//unit map takes ownership of temp_unit
	temp_unit_ = NULL;

	//Add cost to money spent on recruits.
	temp_cost_ = resources::teams->at(team_index()).recall_cost();
	resources::teams->at(team_index()).get_side_actions()->change_gold_spent_by(temp_cost_);

	// Update gold in top bar
	resources::screen->invalidate_game_status();
}

void recall::remove_temp_modifier(unit_map& unit_map)
{
	temp_unit_ = unit_map.extract(recall_hex_);
	assert(temp_unit_);

	//Put unit back into recall list
	resources::teams->at(team_index()).recall_list().push_back(*temp_unit_);

	/*
	 * Remove cost from money spent on recruits.
	 */
	resources::teams->at(team_index()).get_side_actions()->change_gold_spent_by(-temp_cost_);
	temp_cost_ = 0;
	resources::screen->invalidate_game_status();
}

void recall::draw_hex(map_location const& hex)
{
	if (hex == recall_hex_)
	{
		const double x_offset = 0.5;
		const double y_offset = 0.7;
		//position 0,0 in the hex is the upper left corner
		std::stringstream number_text;
		number_text << "-" << resources::teams->at(team_index()).recall_cost();
		size_t font_size = 16;
		SDL_Color color; color.r = 255; color.g = 0; color.b = 0; //red
		resources::screen->draw_text_in_hex(hex, display::LAYER_ACTIONS_NUMBERING,
						number_text.str(), font_size, color, x_offset, y_offset);
	}
}

} //end namespace wb
