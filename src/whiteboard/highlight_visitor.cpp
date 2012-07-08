/* $Id$ */
/*
 Copyright (C) 2010 - 2012 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "highlight_visitor.hpp"

#include "action.hpp"
#include "attack.hpp"
#include "manager.hpp"
#include "move.hpp"
#include "recall.hpp"
#include "recruit.hpp"
#include "side_actions.hpp"
#include "suppose_dead.hpp"

#include "arrow.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "unit_map.hpp"

#include <boost/foreach.hpp>

namespace wb
{

highlight_visitor::highlight_visitor(unit_map& unit_map, side_actions_ptr side_actions)
	: visitor()
	, mode_(NONE)
	, unit_map_(unit_map)
	, mouseover_hex_()
	, exclusive_display_hexes_()
	, owner_unit_(NULL)
	, selection_candidate_(NULL)
	, selected_action_()
	, main_highlight_()
	, secondary_highlights_()
	, side_actions_(side_actions)
{
}

highlight_visitor::~highlight_visitor()
{
	if (resources::screen && owner_unit_)
		unhighlight();
}

void highlight_visitor::set_mouseover_hex(const map_location& hex)
{
	clear();

	if (!hex.valid())
		return;

	real_map ensure_real_map;
	mouseover_hex_ = hex;
	//if we're right over a unit, just highlight all of this unit's actions
	unit_map::iterator it = unit_map_.find(hex);
	if (it != unit_map_.end())
	{
		selection_candidate_ = &(*it);

		if(resources::teams->at(it->side()-1).get_side_actions()->unit_has_actions(&*it))
		{
			owner_unit_ = &(*it);
		}

		//commented code below is to also select the first action of this unit as
		//the main highlight; it doesn't fit too well in the UI
//		side_actions::iterator action_it = side_actions_->find_first_action_of(*it);
//		if (action_it != side_actions_->end())
//		{
//			main_highlight_ = *action_it;
//		}
	}

	//Set the execution/deletion/bump targets.
	if(owner_unit_)
	{
		side_actions::iterator itor = side_actions_->find_first_action_of(owner_unit_);
		if(itor != side_actions_->end())
			selected_action_ = *itor;
	}

	//Overwrite the above selected_action_ if we find a better one
	if(side_actions_->empty()) {
		return;
	}
	BOOST_REVERSE_FOREACH(action_ptr act, *side_actions_)
	{
		/**@todo "is_numbering_hex" is not the "correct" criterion by which to
		 * select the hightlighted/selected action. It's just convenient for me
		 * to use at the moment since it happens to coincide with the "correct"
		 * criterion, which is to use FIND_MAIN_HIGHLIGHT mode.*/
		if(act->is_numbering_hex(hex))
		{
			selected_action_ = act;
			break;
		}
	}
}

void highlight_visitor::clear()
{
	unhighlight();
	main_highlight_.reset();
	owner_unit_ = NULL;
	secondary_highlights_.clear();
	selected_action_.reset();
}

void highlight_visitor::highlight()
{
	//Find main action to highlight if any, as well as owner unit
	find_main_highlight();

	if (action_ptr main = main_highlight_.lock())
	{
		//Highlight main highlight
		mode_ = HIGHLIGHT_MAIN;
		main->accept(*this);
	}

	if (owner_unit_)
	{
		//Find secondary actions to highlight
		find_secondary_highlights();

		//Make sure owner unit is the only one displayed in its hex
		resources::screen->add_exclusive_draw(owner_unit_->get_location(), *owner_unit_);
		exclusive_display_hexes_.insert(owner_unit_->get_location());

		if (!secondary_highlights_.empty())
		{
			//Highlight secondary highlights
			mode_ = HIGHLIGHT_SECONDARY;
			BOOST_FOREACH(weak_action_ptr weak, secondary_highlights_)
			{
				if (action_ptr action = weak.lock())
				{
					action->accept(*this);
				}
			}
		}
	}
}

void highlight_visitor::unhighlight()
{
	//unhighlight main highlight
	if (action_ptr main = main_highlight_.lock() )
	{
		mode_ = UNHIGHLIGHT_MAIN;
		main->accept(*this);
	}

	//unhighlight secondary highlights
	mode_ = UNHIGHLIGHT_SECONDARY;
	BOOST_FOREACH(weak_action_ptr weak, secondary_highlights_)
	{
		if (action_ptr action = weak.lock())
		{
			action->accept(*this);
		}
	}

	//unhide other units if needed
	BOOST_FOREACH(map_location hex, exclusive_display_hexes_)
	{
		resources::screen->remove_exclusive_draw(hex);
	}
	exclusive_display_hexes_.clear();
}

action_ptr highlight_visitor::get_execute_target()
{
	if(action_ptr locked = selected_action_.lock())
		return *side_actions_->find_first_action_of(locked->get_unit());
	else
		return action_ptr();
}
action_ptr highlight_visitor::get_delete_target()
{
	if(action_ptr locked = selected_action_.lock())
		return *side_actions_->find_last_action_of(locked->get_unit());
	else
		return action_ptr();
}

action_ptr highlight_visitor::get_bump_target()
{
	return selected_action_.lock();
}

unit* highlight_visitor::get_selection_target()
{
	if (owner_unit_)
		return owner_unit_;
	else
		return selection_candidate_;
}

void highlight_visitor::visit(move_ptr move)
{
	switch (mode_)
	{
	case FIND_MAIN_HIGHLIGHT:
		if (move->get_dest_hex() == mouseover_hex_)
		{
			main_highlight_ = move;
		}
		break;
	case FIND_SECONDARY_HIGHLIGHTS:
		if (move->get_unit() == owner_unit_
				&& move != main_highlight_.lock())
		{
			secondary_highlights_.push_back(move);
		}
		break;
	case HIGHLIGHT_MAIN:
		if (move->arrow_)
		{
			move->set_arrow_brightness(move::ARROW_BRIGHTNESS_FOCUS);
		}
		if (move->fake_unit_)
		{
			///@todo find some highlight animation
			move->fake_unit_->set_ghosted(true);
			//Make sure the fake unit is the only one displayed in its hex
			resources::screen->add_exclusive_draw(move->fake_unit_->get_location(), *move->fake_unit_);
			exclusive_display_hexes_.insert(move->fake_unit_->get_location());
		}
		break;
	case HIGHLIGHT_SECONDARY:
		if (move->arrow_)
		{
			move->set_arrow_brightness(move::ARROW_BRIGHTNESS_HIGHLIGHTED);
		}
		if (move->fake_unit_)
		{
			move->fake_unit_->set_ghosted(true);
			//Make sure the fake unit is the only one displayed in its hex
			resources::screen->add_exclusive_draw(move->fake_unit_->get_location(), *move->fake_unit_);
			exclusive_display_hexes_.insert(move->fake_unit_->get_location());
		}
		break;
	case UNHIGHLIGHT_MAIN: // fall-through
	case UNHIGHLIGHT_SECONDARY:
		if (move->arrow_)
		{
			move->set_arrow_brightness(move::ARROW_BRIGHTNESS_STANDARD);
		}
		if (move->fake_unit_)
		{
			move->fake_unit_->set_disabled_ghosted(false);
		}
		break;
	default:
		assert (false);
		break;
	}

	//Last action with a fake unit always gets normal appearance
	//Override choices above
	if (move->fake_unit_)
	{
		side_actions& sa = *resources::teams->at(move->team_index()).get_side_actions();
		side_actions::iterator last_action = sa.find_last_action_of(move->get_unit());
		side_actions::iterator second_to_last_action =
				last_action != sa.end() && last_action != sa.begin() ? last_action - 1 : sa.end();
		bool this_is_last_action = last_action != sa.end() && move == *last_action;
		bool last_action_has_fake_unit = last_action != sa.end() && (*last_action)->get_fake_unit();
		bool this_is_second_to_last_action = (second_to_last_action != sa.end()
				&& move == *second_to_last_action);

		if (this_is_last_action
				|| (this_is_second_to_last_action && !last_action_has_fake_unit))
		{
			move->fake_unit_->set_standing(true);
		}
	}
}

void highlight_visitor::visit(attack_ptr attack)
{
	visit(boost::static_pointer_cast<move>(attack));
	//@todo: highlight the attack indicator
}

void highlight_visitor::visit(recruit_ptr recruit)
{
	switch (mode_)
	{
	case FIND_MAIN_HIGHLIGHT:
		if (recruit->recruit_hex_ == mouseover_hex_)
		{
			main_highlight_ = recruit;
		}
		break;
	case FIND_SECONDARY_HIGHLIGHTS:
		break;
	case HIGHLIGHT_MAIN:
		if (recruit->fake_unit_)
		{
			//@todo: find some suitable effect for mouseover on planned recruit.

			//Make sure the fake unit is the only one displayed in its hex
			resources::screen->add_exclusive_draw(recruit->fake_unit_->get_location(), *recruit->fake_unit_);
			exclusive_display_hexes_.insert(recruit->fake_unit_->get_location());
		}
		break;
	case HIGHLIGHT_SECONDARY:
		break;
	case UNHIGHLIGHT_MAIN:
	case UNHIGHLIGHT_SECONDARY:
		if (recruit->fake_unit_)
		{
			//@todo: find some suitable effect for mouseover on planned recruit.
		}
		break;
	default:
		assert (false);
	}
}

void highlight_visitor::visit(recall_ptr recall)
{
	switch (mode_)
	{
	case FIND_MAIN_HIGHLIGHT:
		if (recall->recall_hex_ == mouseover_hex_)
		{
			main_highlight_ = recall;
		}
		break;
	case FIND_SECONDARY_HIGHLIGHTS:
		break;
	case HIGHLIGHT_MAIN:
		if (recall->fake_unit_)
		{
			//@todo: find some suitable effect for mouseover on planned recall.
		}
		break;
	case HIGHLIGHT_SECONDARY:
		break;
	case UNHIGHLIGHT_MAIN:
	case UNHIGHLIGHT_SECONDARY:
		if (recall->fake_unit_)
		{
			//@todo: find some suitable effect for mouseover on planned recall.

			//Make sure the fake unit is the only one displayed in its hex
			resources::screen->add_exclusive_draw(recall->fake_unit_->get_location(), *recall->fake_unit_);
			exclusive_display_hexes_.insert(recall->fake_unit_->get_location());
		}
		break;
	default:
		assert (false);
	}
}

void highlight_visitor::visit(suppose_dead_ptr sup_d)
{
	switch(mode_)
	{
	case FIND_MAIN_HIGHLIGHT:
		if (sup_d->get_source_hex() == mouseover_hex_)
		{
			main_highlight_ = sup_d;
		}
		break;
	case FIND_SECONDARY_HIGHLIGHTS:
		break;
	case HIGHLIGHT_MAIN:
		break;
	case HIGHLIGHT_SECONDARY:
		break;
	case UNHIGHLIGHT_MAIN:
		break;
	case UNHIGHLIGHT_SECONDARY:
		break;
	default:
		assert (false);
	}
}

void highlight_visitor::find_main_highlight()
{
	// Even if we already found an owner_unit_ in the mouseover hex,
	// action destination hexes usually take priority over that
	mode_ = FIND_MAIN_HIGHLIGHT;
	assert(main_highlight_.expired());
	//@todo re-enable the following assert once I find out what happends to
	// viewing side assignments after victory
	//assert(side_actions_->team_index() == resources::screen->viewing_team());

	//Find the main highlight -- see visit(), below.
	reverse_visit_all();
}

//Used only by find_main_highlight()
bool highlight_visitor::process(size_t, team&, side_actions&, side_actions::iterator itor)
{
	(*itor)->accept(*this);
	if (action_ptr main = main_highlight_.lock())
	{
		owner_unit_ = main->get_unit();
		return false;
	}
	return true;
}

void highlight_visitor::find_secondary_highlights()
{
	assert(owner_unit_);
	assert(secondary_highlights_.empty());
	mode_ = FIND_SECONDARY_HIGHLIGHTS;
	visit_all_actions(); //< protected fcn from visitor
}

} // end namespace wb
