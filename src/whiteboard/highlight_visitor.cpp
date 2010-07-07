/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file highlight_visitor.cpp
 */

#include "highlight_visitor.hpp"
#include "manager.hpp"
#include "action.hpp"
#include "move.hpp"
#include "side_actions.hpp"

#include "arrow.hpp"
#include "foreach.hpp"
#include "unit_map.hpp"
#include "resources.hpp"

namespace wb
{

highlight_visitor::highlight_visitor(const unit_map& unit_map, side_actions_ptr side_actions)
	: mode_(NONE)
	, unit_map_(unit_map)
	, side_actions_(side_actions)
	, mouseover_hex_()
	, owner_unit_(NULL)
	, main_highlight_()
	, secondary_highlights_()
	, color_backup_()
{
}

highlight_visitor::~highlight_visitor()
{
	if (resources::screen && owner_unit_)
		unhighlight();
}

void highlight_visitor::set_mouseover_hex(const map_location& hex)
{
	if (mouseover_hex_.valid())
	{
		clear();
	}
	scoped_real_unit_map ensure_real_map;
	mouseover_hex_ = hex;
	//if we're right over a unit, just highlight all of this unit's actions
	unit_map::const_iterator it = unit_map_.find(hex);
	if (it != unit_map_.end())
	{
		owner_unit_ = &(*it);

		//commented code below is to also select the first action of this unit as
		//the main highlight; it doesn't fit too well in the UI
//		side_actions::iterator action_it = side_actions_->find_first_action_of(*it);
//		if (action_it != side_actions_->end())
//		{
//			main_highlight_ = *action_it;
//		}
	}
}

void highlight_visitor::clear()
{
	unhighlight();
	main_highlight_.reset();
	owner_unit_ = NULL;
	secondary_highlights_.clear();
}

void highlight_visitor::highlight()
{
	//Find main action to highlight if any, as well as owner unit
	find_main_highlight();
	if  (owner_unit_)
	{
		//Find secondary actions to highlight
		find_secondary_highlights();


		if (action_ptr main = main_highlight_.lock())
		{
			//Highlight main highlight
			mode_ = HIGHLIGHT_MAIN;
			main->accept(*this);
		}

		if (!secondary_highlights_.empty())
			//Highlight owner unit
			owner_unit_->set_selecting();
			//Highlight secondary highlights
			mode_ = HIGHLIGHT_SECONDARY;
			foreach(weak_action_ptr weak, secondary_highlights_)
			{
				if (action_ptr action = weak.lock())
				{
					action->accept(*this);
				}
			}
	}
}

void highlight_visitor::unhighlight()
{
	//unhighlight owner unit
	if(owner_unit_)
		owner_unit_->set_standing(true);

	mode_ = UNHIGHLIGHT;
	//unhighlight main highlight
	if (action_ptr main = main_highlight_.lock() )
		main->accept(*this);
	//unhighlight secondary highlights
	foreach(weak_action_ptr weak, secondary_highlights_)
	{
		if (action_ptr action = weak.lock())
		{
			action->accept(*this);
		}
	}
}

action_ptr highlight_visitor::get_execute_target()
{
	action_ptr action;
	if (owner_unit_)
	{
		action = *side_actions_->find_first_action_of(*owner_unit_);
	}
	return action;
}
action_ptr highlight_visitor::get_delete_target()
{
	action_ptr action;
	if (owner_unit_)
	{
		action = *side_actions_->find_last_action_of(*owner_unit_);
	}
	return action;
}

action_ptr highlight_visitor::get_bump_target()
{
	return main_highlight_.lock();
}

void highlight_visitor::visit_move(move_ptr move)
{
	switch (mode_)
	{
	case FIND_MAIN_HIGHLIGHT:
		if (move->dest_hex_ == mouseover_hex_)
		{
			main_highlight_ = move;
		}
		break;
	case FIND_SECONDARY_HIGHLIGHTS:
		if (&move->unit_ == owner_unit_
				&& move != main_highlight_.lock())
		{
			secondary_highlights_.push_back(move);
		}
		break;
	case HIGHLIGHT_MAIN:
		color_backup_ = move->arrow_->get_color();
		move->arrow_->set_alpha(move::ALPHA_HIGHLIGHT);
		move->arrow_->set_color("white");
		move->fake_unit_->set_standing(false);
		break;
	case HIGHLIGHT_SECONDARY:
		move->arrow_->set_alpha(move::ALPHA_HIGHLIGHT);
		move->fake_unit_->set_ghosted(false);
		break;
	case UNHIGHLIGHT:
		if (move == main_highlight_.lock())
		{
			move->arrow_->set_color(color_backup_);
		}
		move->arrow_->set_alpha(move::ALPHA_NORMAL);
		move->fake_unit_->set_disabled_ghosted(false);
		break;
	default:
		assert (false);
	}
}

void highlight_visitor::visit_all_actions()
{
	foreach(action_ptr action, *side_actions_)
	{
		action->accept(*this);
	}
}

void highlight_visitor::find_main_highlight()
{
	// Even if we already found an owner_unit_ in the mouseover hex,
	// action destination hexes usually take priority over that
	mode_ = FIND_MAIN_HIGHLIGHT;
	assert(main_highlight_.expired());
	foreach(action_ptr action, *side_actions_)
	{
		action->accept(*this);
		if (action_ptr main = main_highlight_.lock())
		{
			owner_unit_ = &main->get_unit();
			break;
		}
	}
}

void highlight_visitor::find_secondary_highlights()
{
	assert(owner_unit_);
	assert(secondary_highlights_.empty());
	mode_ = FIND_SECONDARY_HIGHLIGHTS;
	visit_all_actions();
}

} // end namespace wb
