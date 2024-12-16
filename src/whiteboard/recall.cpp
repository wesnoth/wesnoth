/*
	Copyright (C) 2010 - 2024
	by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "whiteboard/recall.hpp"

#include "whiteboard/side_actions.hpp"
#include "whiteboard/utility.hpp"
#include "whiteboard/visitor.hpp"

#include "display.hpp"
#include "fake_unit_ptr.hpp"
#include "game_board.hpp"
#include "recall_list_manager.hpp"
#include "resources.hpp"
#include "replay_helper.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/filter.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"

namespace wb
{

std::ostream& operator<<(std::ostream& s, const recall_ptr& recall)
{
	assert(recall);
	return recall->print(s);
}
std::ostream& operator<<(std::ostream& s, const recall_const_ptr& recall)
{
	assert(recall);
	return recall->print(s);
}

std::ostream& recall::print(std::ostream &s) const
{
	s << "Recalling " << fake_unit_->name() << " [" << fake_unit_->id() << "] on hex " << recall_hex_;
	return s;
}

recall::recall(std::size_t team_index, bool hidden, const unit& u, const map_location& recall_hex)
	: action(team_index,hidden)
	, temp_unit_(u.clone())
	, recall_hex_(recall_hex)
	, fake_unit_(u.clone())
	, original_mp_(0)
	, original_ap_(0)
	, original_recall_pos_(0)
{
	this->init();
}

recall::recall(const config& cfg, bool hidden)
	: action(cfg,hidden)
	, temp_unit_()
	, recall_hex_(cfg.mandatory_child("recall_hex_")["x"],cfg.mandatory_child("recall_hex_")["y"], wml_loc())
	, fake_unit_()
	, original_mp_(0)
	, original_ap_(0)
	, original_recall_pos_(0)
{
	// Construct and validate temp_unit_
	std::size_t underlying_id = cfg["temp_unit_"].to_size_t();
	for(const unit_ptr & recall_unit : resources::gameboard->teams().at(team_index()).recall_list())
	{
		if(recall_unit->underlying_id()==underlying_id)
		{
			temp_unit_ = recall_unit;
			break;
		}
	}
	if(!temp_unit_.get()) {
		throw action::ctor_err("recall: Invalid underlying_id");
	}

	fake_unit_.reset(temp_unit_->clone()); //makes copy of temp_unit_

	this->init();
}

void recall::init()
{
	fake_unit_->set_location(recall_hex_);
	fake_unit_->set_movement(0, true);
	fake_unit_->set_attacks(0);
	fake_unit_->anim_comp().set_ghosted(false);
	fake_unit_.place_on_fake_unit_manager( resources::fake_units);
}

recall::~recall()
{
}

void recall::accept(visitor& v)
{
	v.visit(shared_from_this());
}

void recall::execute(bool& success, bool& complete)
{
	team & current_team = resources::gameboard->teams().at(team_index());

	assert(valid());
	assert(temp_unit_.get());
	temporary_unit_hider const raii(*fake_unit_);
	//Give back the spent gold so we don't get "not enough gold" message
	int cost = current_team.recall_cost();
	if (temp_unit_->recall_cost() > -1) {
		cost=temp_unit_->recall_cost();
	}
	current_team.get_side_actions()->change_gold_spent_by(-cost);
	bool const result = synced_context::run_and_throw("recall",
		replay_helper::get_recall(temp_unit_->id(), recall_hex_, map_location::null_location()));

	if (!result) {
		current_team.get_side_actions()->change_gold_spent_by(cost);
	}
	success = complete = result;
}

void recall::apply_temp_modifier(unit_map& unit_map)
{
	assert(valid());


	DBG_WB << "Inserting future recall " << temp_unit_->name() << " [" << temp_unit_->id()
			<< "] at position " << temp_unit_->get_location() << ".";

	//temporarily remove unit from recall list
	unit_ptr it = resources::gameboard->teams().at(team_index()).recall_list().extract_if_matches_id(temp_unit_->id(), &original_recall_pos_);
	assert(it);

	//Usually (temp_unit_ == it) is true here, but wml might have changed the original unit in which case not doing 'temp_unit_ = it' would result in a gamestate change.
	temp_unit_ = it;
	original_mp_ = temp_unit_->movement_left(true);
	original_ap_ = temp_unit_->attacks_left(true);

	temp_unit_->set_movement(0, true);
	temp_unit_->set_attacks(0);
	temp_unit_->set_location(recall_hex_);

	//Add cost to money spent on recruits.
	int cost = resources::gameboard->teams().at(team_index()).recall_cost();
	if (it->recall_cost() > -1) {
		cost = it->recall_cost();
	}

	// Temporarily insert unit into unit_map
	//unit map takes ownership of temp_unit
	unit_map.insert(temp_unit_);

	resources::gameboard->teams().at(team_index()).get_side_actions()->change_gold_spent_by(cost);
	// Update gold in top bar
	display::get_singleton()->invalidate_game_status();
}

void recall::remove_temp_modifier(unit_map& unit_map)
{
	temp_unit_ = unit_map.extract(recall_hex_);
	assert(temp_unit_.get());

	temp_unit_->set_movement(original_mp_, true);
	temp_unit_->set_attacks(original_ap_);

	original_mp_ = 0;
	original_ap_ = 0;
	//Put unit back into recall list
	resources::gameboard->teams().at(team_index()).recall_list().add(temp_unit_, original_recall_pos_);
}

void recall::draw_hex(const map_location& hex)
{
	if (hex == recall_hex_)
	{
		const double x_offset = 0.5;
		const double y_offset = 0.7;
		//position 0,0 in the hex is the upper left corner
		std::stringstream number_text;
		unit &it = *get_unit();
		int cost = it.recall_cost();
		if (cost < 0) {
			number_text << font::unicode_minus << resources::gameboard->teams().at(team_index()).recall_cost();
		}
		else {
			number_text << font::unicode_minus << cost;
		}
		std::size_t font_size = 16;
		color_t color {255, 0, 0}; //red
		display::get_singleton()->draw_text_in_hex(hex, drawing_layer::actions_numbering,
						number_text.str(), font_size, color, x_offset, y_offset);
	}
}

void recall::redraw()
{
	display::get_singleton()->invalidate(recall_hex_);
}

action::error recall::check_validity() const
{
	//Check that destination hex is still free
	if(resources::gameboard->units().find(recall_hex_) != resources::gameboard->units().end()) {
		return LOCATION_OCCUPIED;
	}
	//Check that unit to recall is still in side's recall list
	if( !resources::gameboard->teams()[team_index()].recall_list().find_if_matches_id(temp_unit_->id()) ) {
		return UNIT_UNAVAILABLE;
	}
	//Check that there is still enough gold to recall this unit
	if(resources::gameboard->teams()[team_index()].recall_cost() > resources::gameboard->teams()[team_index()].gold()) {
		return NOT_ENOUGH_GOLD;
	}
	//Check that there is a leader available to recall this unit
	bool has_recruiter = any_recruiter(team_index() + 1, get_recall_hex(), [&](unit& leader) {
		const unit_filter ufilt(vconfig(leader.recall_filter()));
		return ufilt(*temp_unit_, map_location::null_location());
	});

	if(!has_recruiter) {
		return NO_LEADER;
	}

	return OK;
}

/** @todo Find a better way to serialize unit_ because underlying_id isn't cutting it */
config recall::to_config() const
{
	config final_cfg = action::to_config();

	final_cfg["type"] = "recall";
	final_cfg["temp_unit_"] = static_cast<int>(temp_unit_->underlying_id());
//	final_cfg["temp_cost_"] = temp_cost_; //Unnecessary

	config loc_cfg;
	loc_cfg["x"]=recall_hex_.wml_x();
	loc_cfg["y"]=recall_hex_.wml_y();
	final_cfg.add_child("recall_hex_", std::move(loc_cfg));

	return final_cfg;
}

void recall::do_hide() {fake_unit_->set_hidden(true);}
void recall::do_show() {fake_unit_->set_hidden(false);}

} //end namespace wb
