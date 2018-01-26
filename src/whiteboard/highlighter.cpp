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

#include <algorithm>
#include <iterator>
#include <boost/range/adaptor/reversed.hpp>

#include "utils/functional.hpp"

#include "whiteboard/highlighter.hpp"

#include "whiteboard/action.hpp"
#include "whiteboard/attack.hpp"
#include "whiteboard/manager.hpp"
#include "whiteboard/move.hpp"
#include "whiteboard/recall.hpp"
#include "whiteboard/recruit.hpp"
#include "whiteboard/side_actions.hpp"
#include "whiteboard/suppose_dead.hpp"
#include "whiteboard/utility.hpp"

#include "arrow.hpp"
#include "config.hpp"
#include "fake_unit_ptr.hpp"
#include "game_board.hpp"
#include "game_display.hpp"
#include "game_errors.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "units/map.hpp"

namespace wb
{

highlighter::highlighter(side_actions_ptr side_actions)
	: mouseover_hex_()
	, exclusive_display_hexes_()
	, owner_unit_()
	, selection_candidate_()
	, selected_action_()
	, main_highlight_()
	, secondary_highlights_()
	, side_actions_(side_actions)
{
}

highlighter::~highlighter()
{
	try {
	if(game_display::get_singleton() && owner_unit_) {
		unhighlight();
	}
	} catch (...) {}
}

void highlighter::set_mouseover_hex(const map_location& hex)
{
	clear();

	if(!hex.valid()) {
		return;
	}

	real_map ensure_real_map;
	mouseover_hex_ = hex;
	//if we're right over a unit, just highlight all of this unit's actions
	unit_map::iterator it = get_unit_map().find(hex);
	if(it != get_unit_map().end()) {
		selection_candidate_ = it.get_shared_ptr();

		if(resources::gameboard->get_team(it->side()).get_side_actions()->unit_has_actions(*it)) {
			owner_unit_ = it.get_shared_ptr();
		}

		//commented code below is to also select the first action of this unit as
		//the main highlight; it doesn't fit too well in the UI
//		side_actions::iterator action_it = side_actions_->find_first_action_of(*it);
//		if(action_it != side_actions_->end()) {
//			main_highlight_ = *action_it;
//		}
	}

	//Set the execution/deletion/bump targets.
	if(owner_unit_) {
		side_actions::iterator itor = side_actions_->find_first_action_of(*owner_unit_);
		if(itor != side_actions_->end()) {
			selected_action_ = *itor;
		}
	}

	//Overwrite the above selected_action_ if we find a better one
	if(side_actions_->empty()) {
		return;
	}
	for(action_ptr act : boost::adaptors::reverse(*side_actions_)) {
		/**@todo "is_numbering_hex" is not the "correct" criterion by which to
		 * select the hightlighted/selected action. It's just convenient for me
		 * to use at the moment since it happens to coincide with the "correct"
		 * criterion, which is to use find_main_highlight.*/
		if(act->is_numbering_hex(hex)) {
			selected_action_ = act;
			break;
		}
	}
}

void highlighter::clear()
{
	unhighlight();
	main_highlight_.reset();
	owner_unit_.reset();
	secondary_highlights_.clear();
	selected_action_.reset();
}

void highlighter::highlight()
{
	//Find main action to highlight if any, as well as owner unit
	find_main_highlight();

	if(action_ptr main = main_highlight_.lock()) {
		//Highlight main highlight
		highlight_main_visitor hm_visitor(*this);
		main->accept(hm_visitor);
	}

	if(owner_unit_) {
		//Find secondary actions to highlight
		find_secondary_highlights();

		//Make sure owner unit is the only one displayed in its hex
		game_display::get_singleton()->add_exclusive_draw(owner_unit_->get_location(), *owner_unit_);
		exclusive_display_hexes_.insert(owner_unit_->get_location());

		if(!secondary_highlights_.empty()) {
			//Highlight secondary highlights
			highlight_secondary_visitor hs_visitor(*this);
			for(weak_action_ptr weak : secondary_highlights_) {
				if(action_ptr action = weak.lock()) {
					action->accept(hs_visitor);
				}
			}
		}
	}
}

void highlighter::unhighlight()
{
	unhighlight_visitor uh_visitor(*this);

	//unhighlight main highlight
	if(action_ptr main = main_highlight_.lock()) {
		main->accept(uh_visitor);
	}

	//unhighlight secondary highlights
	for(weak_action_ptr weak : secondary_highlights_) {
		if(action_ptr action = weak.lock()) {
			action->accept(uh_visitor);
		}
	}

	//unhide other units if needed
	for(map_location hex : exclusive_display_hexes_) {
		game_display::get_singleton()->remove_exclusive_draw(hex);
	}
	exclusive_display_hexes_.clear();
}

void highlighter::last_action_redraw(move_ptr move)
{
	//Last action with a fake unit always gets normal appearance
	if(move->get_fake_unit()) {
		side_actions& sa = *resources::gameboard->teams().at(move->team_index()).get_side_actions().get();

		// Units with planned actions may have been killed in the previous turn before all actions were completed.
		// In these cases, remove these planned actions for any invalid units and do not redraw anything.
		if (move->get_unit() == nullptr)
		{
			// Note: the planned actions seem to only get removed from the screen when
			// a redraw is triggered by the mouse cursor moving over them.
			for (side_actions::iterator iterator = sa.begin(); iterator < sa.end(); ++iterator)
			{
				if (iterator->get()->get_unit() == nullptr)
					sa.remove_action (iterator);
			}

			return;
		}

		side_actions::iterator last_action = sa.find_last_action_of(*(move->get_unit()));
		side_actions::iterator second_to_last_action = last_action != sa.end() && last_action != sa.begin() ? last_action - 1 : sa.end();

		bool this_is_last_action = last_action != sa.end() && move == *last_action;
		bool last_action_has_fake_unit = last_action != sa.end() && (*last_action)->get_fake_unit();
		bool this_is_second_to_last_action = (second_to_last_action != sa.end() && move == *second_to_last_action);

		if(this_is_last_action || (this_is_second_to_last_action && !last_action_has_fake_unit)) {
			move->get_fake_unit()->anim_comp().set_standing(true);
		}
	}
}

void highlighter::find_main_highlight()
{
	// Even if we already found an owner_unit_ in the mouseover hex,
	// action destination hexes usually take priority over that
	assert(main_highlight_.expired());
	//@todo re-enable the following assert once I find out what happends to
	// viewing side assignments after victory
	//assert(side_actions_->team_index() == game_display::get_singleton()->viewing_team());

	main_highlight_ = find_action_at(mouseover_hex_);
	if(action_ptr main = main_highlight_.lock()) {
		owner_unit_ = main->get_unit();
	}
}

void highlighter::find_secondary_highlights()
{
	assert(owner_unit_);
	assert(secondary_highlights_.empty());

	if(owner_unit_ == nullptr) {
		return;
	}

	// List all the actions of owner_unit_
	std::deque<action_ptr> actions = find_actions_of(*owner_unit_);

	// Remove main_highlight_ if present
	actions.erase(std::remove(actions.begin(), actions.end(), main_highlight_.lock()), actions.end());

	// Copy in secondary_highlights_
	std::copy(actions.begin(), actions.end(), std::back_inserter(secondary_highlights_));
}


action_ptr highlighter::get_execute_target()
{
	if(action_ptr locked = selected_action_.lock()) {
		return *side_actions_->find_first_action_of(*(locked->get_unit()));
	} else {
		return action_ptr();
	}
}
action_ptr highlighter::get_delete_target()
{
	if(action_ptr locked = selected_action_.lock()) {
		return *side_actions_->find_last_action_of(*(locked->get_unit()));
	} else {
		return action_ptr();
	}
}

action_ptr highlighter::get_bump_target()
{
	return selected_action_.lock();
}

unit_ptr highlighter::get_selection_target()
{
	if(owner_unit_) {
		return owner_unit_;
	} else {
		return selection_candidate_;
	}
}

void highlighter::highlight_main_visitor::visit(move_ptr move)
{
	if(move->get_arrow()) {
		move->set_arrow_brightness(move::ARROW_BRIGHTNESS_FOCUS);
	}
	if(move->get_fake_unit()) {
		///@todo find some highlight animation
		move->get_fake_unit()->anim_comp().set_ghosted(true);
		//Make sure the fake unit is the only one displayed in its hex
		game_display::get_singleton()->add_exclusive_draw(move->get_fake_unit()->get_location(), *move->get_fake_unit());
		highlighter_.exclusive_display_hexes_.insert(move->get_fake_unit()->get_location());

		highlighter_.last_action_redraw(move);
	}
}

void highlighter::highlight_main_visitor::visit(attack_ptr attack)
{
	///@todo: highlight the attack indicator
	visit(std::static_pointer_cast<move>(attack));
}

void highlighter::highlight_main_visitor::visit(recruit_ptr recruit)
{
	if(recruit->get_fake_unit()) {
		///@todo: find some suitable effect for mouseover on planned recruit.

		//Make sure the fake unit is the only one displayed in its hex
		game_display::get_singleton()->add_exclusive_draw(recruit->get_fake_unit()->get_location(), *recruit->get_fake_unit());
		highlighter_.exclusive_display_hexes_.insert(recruit->get_fake_unit()->get_location());
	}
}

void highlighter::highlight_secondary_visitor::visit(move_ptr move)
{
	if(move->get_arrow()) {
		move->set_arrow_brightness(move::ARROW_BRIGHTNESS_HIGHLIGHTED);
	}
	if(move->get_fake_unit()) {
		move->get_fake_unit()->anim_comp().set_ghosted(true);
		//Make sure the fake unit is the only one displayed in its hex
		game_display::get_singleton()->add_exclusive_draw(move->get_fake_unit()->get_location(), *move->get_fake_unit());
		highlighter_.exclusive_display_hexes_.insert(move->get_fake_unit()->get_location());

		highlighter_.last_action_redraw(move);
	}
}

void highlighter::highlight_secondary_visitor::visit(attack_ptr attack)
{
	visit(std::static_pointer_cast<move>(attack));
}

void highlighter::unhighlight_visitor::visit(move_ptr move)
{
	if(move->get_arrow()) {
		move->set_arrow_brightness(move::ARROW_BRIGHTNESS_STANDARD);
	}
	if(move->get_fake_unit()) {
		move->get_fake_unit()->anim_comp().set_disabled_ghosted(false);

		highlighter_.last_action_redraw(move);
	}
}

void highlighter::unhighlight_visitor::visit(attack_ptr attack)
{
	visit(std::static_pointer_cast<move>(attack));
}

void highlighter::unhighlight_visitor::visit(recall_ptr recall)
{
	if(recall->get_fake_unit()) {
		//@todo: find some suitable effect for mouseover on planned recall.

		//Make sure the fake unit is the only one displayed in its hex
		game_display::get_singleton()->add_exclusive_draw(recall->get_fake_unit()->get_location(), *recall->get_fake_unit());
		highlighter_.exclusive_display_hexes_.insert(recall->get_fake_unit()->get_location());
	}
}
unit_map& highlighter::get_unit_map()
{
	assert(resources::gameboard);
	return resources::gameboard->units();
}

} // end namespace wb
