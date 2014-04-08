/*
   Copyright (C) 2008 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "widgets/drop_target.hpp"

#include <boost/bind.hpp>

namespace gui {

	drop_target::drop_groups drop_target::groups_;
	drop_target::target_id drop_target::next_id_;
	drop_target_group drop_target::next_group_ = 0;

	int drop_target::next_free_id(const drop_target_group& group) const
	{
		target_id::iterator group_id = next_id_.insert(std::make_pair(group, 0)).first;
		return group_id->second++;
	}

	drop_target::drop_target(const drop_group_manager_ptr group, const SDL_Rect& loc) : loc_(loc), id_(next_free_id(group->get_group_id())), group_(group)
	{
		groups_.insert(std::make_pair(group_->get_group_id(), this));
	}

	bool drop_target::is_this_id(const int id) const
	{
		return id_ == id;
	}

	drop_target::drop_groups::iterator drop_target::find_this() const
	{
		return std::find_if(groups_.lower_bound(group_->get_group_id()),
				groups_.upper_bound(group_->get_group_id()),
				boost::bind(&drop_target::is_this_id,boost::bind(&drop_groups::value_type::second,_1),id_));
	}

	drop_target::~drop_target()
	{
		groups_.erase(find_this());
	}

	int drop_target::get_id() const
	{
		return id_;
	}

	int drop_target::handle_drop()
	{
		drop_groups::iterator end = groups_.upper_bound(group_->get_group_id());
		drop_target::drop_groups::iterator itor
			= std::find_if(groups_.lower_bound(group_->get_group_id()),
					end,
					boost::bind(&drop_target::hit_rect,
						boost::bind(&drop_groups::value_type::second,_1)
						,boost::cref(loc_), id_));

		if (itor == end)
			return -1;

		return itor->second->get_id();
	}

	void drop_target::delete_group(const drop_target_group id)
	{
		next_id_.erase(id);
		groups_.erase(id);
	}

	drop_target_group drop_target::create_group()
	{
		return next_group_++;
	}

	bool drop_target::hit_rect(const SDL_Rect& hit_loc, const int not_id) const
	{
		if (id_ == not_id)
			return false;
		int this_right = loc_.x + loc_.w;
		int this_lower = loc_.y + loc_.h;

		int hit_right = hit_loc.x + hit_loc.w;
		int hit_lower = hit_loc.y + hit_loc.h;

		// Is it inside in x direction?
		return (this_right > hit_loc.x
				&& loc_.x < hit_right
		// Is it inside in y direction?
				&& this_lower > hit_loc.y
				&& loc_.y < hit_lower);

	}

	drop_group_manager::drop_group_manager() : group_id_(drop_target::create_group())
	{
	}

	drop_group_manager::~drop_group_manager()
	{
		drop_target::delete_group(group_id_);
	}

	drop_target_group drop_group_manager::get_group_id() const
	{
		return group_id_;
	}
}

