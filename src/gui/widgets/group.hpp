/*
   Copyright (C) 2008 - 2016 The Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_GROUP_HPP_INCLUDED
#define GUI_WIDGETS_GROUP_HPP_INCLUDED

#include "gui/auxiliary/event/dispatcher.hpp"
#include "gui/widgets/selectable.hpp"
#include "gui/widgets/widget.hpp"
#include "utils/foreach.tpp"

#include <vector>
#include <boost/bind.hpp>

namespace gui2
{

template <class T>
class tgroup
{
public:
	typedef typename std::pair<tselectable_*, T> group_type;
	typedef typename std::vector<group_type>     group_list;
	typedef typename group_list::iterator        group_iterator;
	typedef typename group_list::const_iterator  group_iterator_const;

	/**
	 * Adds a widget/value pair to the group vector. A callback is set
	 * that sets all members' toggle states to false when clicked. This
	 * happens before individual widget handlers fire, meaning that the
	 * clicked widget will remain the only one selected.
	 */
	void add_member(tselectable_* widget, const T& value)
	{
		members_.push_back(std::make_pair(widget, value));

		dynamic_cast<twidget*>(widget)->connect_signal<event::LEFT_BUTTON_CLICK>(boost::bind(
			&tgroup::group_operator, this), event::tdispatcher::front_child);
	}

	/**
	 * Removes a member from the group vector.
	 */
#ifdef HAVE_CXX11
	void remove_member(tselectable_* widget)
	{
		members_.erase(std::find_if(members_.begin(), members_.end(),
			[&widget](const group_type& member){ return member.first == widget; }));
	}
#endif

	/**
	 * Group member getters
	 */
	std::pair<group_iterator, group_iterator> members()
	{
		return std::make_pair(members_.begin(), members_.end());
	}

	std::pair<group_iterator_const, group_iterator_const> members() const
	{
		return std::make_pair(members_.begin(), members_.end());
	}

	/**
	 * The default actions to take when clicking on one of the widgets
	 * in the group.
	 */
	void group_operator()
	{
		FOREACH(AUTO& member, members())
		{
			member.first->set_value(false);
		}
	}

	/**
	 * Returns the value paired with the currently activiely toggled member
	 * of the group.
	 */
	T get_active_member_value()
	{
		FOREACH(AUTO& member, members())
		{
			if(member.first->get_value_bool()) {
				return member.second;
			}
		}

		return T();
	}

	/**
	 * Sets the toggle values for all widgets besides the one associated
	 * with the specified value to false.
	 */
	void set_member_states(const T& value)
	{
		FOREACH(AUTO& member, members())
		{
			member.first->set_value(member.second == value);
		}
	}

private:
	group_list members_;

};

} // namespace gui2

#endif
