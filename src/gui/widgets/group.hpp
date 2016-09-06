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

#include "gui/core/event/dispatcher.hpp"
#include "gui/widgets/control.hpp"
#include "gui/widgets/selectable.hpp"
#include "gui/widgets/widget.hpp"

#include <map>
#include "utils/functional.hpp"

namespace gui2
{

template <class T>
class tgroup
{
public:
	typedef typename std::map<T, tselectable_*>  group_map;
	typedef typename group_map::iterator         group_iterator;
	typedef typename group_map::const_iterator   group_iterator_const;

	/**
	 * Adds a widget/value pair to the group map. A callback is set
	 * that sets all members' toggle states to false when clicked. This
	 * happens before individual widget handlers fire, meaning that the
	 * clicked widget will remain the only one selected.
	 */
	void add_member(tselectable_* widget, const T& value)
	{
		members_.emplace(value, widget);

		dynamic_cast<twidget*>(widget)->connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&tgroup::group_operator, this), event::tdispatcher::front_child);
	}

	/**
	 * Removes a member from the group map.
	 */
	void remove_member(const T& value)
	{
		members_.erase(value);
	}

	/**
	 * Clears the entire group of members.
	 */
	void clear()
	{
		members_.clear();
	}

	/**
	 * Group member getters
	 */
	group_map& members()
	{
		return members_;
	}

	const group_map& members() const
	{
		return members_;
	}

	template<typename W>
	W& member(const T& value)
	{
		return dynamic_cast<W&>(*members_.at(value));
	}

	/**
	 * Returns the value paired with the currently actively toggled member
	 * of the group.
	 */
	T get_active_member_value()
	{
		for(auto& member : members_) {
			if(member.second->get_value_bool()) {
				return member.first;
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
		for(auto& member : members_) {
			member.second->set_value(member.first == value);
		}
	}

	/**
	 * Sets a common callback function for all members.
	 */
	void set_callback_on_value_change(const std::function<void(twidget&)>& func)
	{
		// Ensure this callback is only called on the member being activated
		const auto callback = [func](twidget& widget)->void {
			if(dynamic_cast<tselectable_*>(&widget)->get_value_bool()) {
				func(widget);
			}
		};

		for(auto& member : members_) {
			member.second->set_callback_state_change(callback);
		}
	}

	/**
	 * Wrapper for enabling or disabling a member widget.
	 * If the selected widget is selected, it is deselected and the first active
	 * member selected instead.
	 */
	void set_member_active(const T& value, const bool active)
	{
		if(members_.find(value) == members_.end()) {
			return;
		}

		tselectable_& w = *members_[value];
		dynamic_cast<tcontrol&>(w).set_active(active);

		// Only select another member this was selected
		// TODO: for some reason, this doesn't work as expected. While the button will be
		// deselected, the get_active call seems to fail, leading to situations where
		// no member is selected. Commenting out for now.
		//if(!w.get_value_bool()) {
		//	return;
		//}

		w.set_value_bool(false);

		// Look for the first active member to select
		for(auto& member : members_) {
			if(dynamic_cast<tcontrol&>(*member.second).get_active()) {
				member.second->set_value_bool(true);
				break;
			}
		}
	}

private:
	group_map members_;

	/**
	 * The default actions to take when clicking on one of the widgets
	 * in the group.
	 */
	void group_operator()
	{
		for(auto& member : members_) {
			member.second->set_value(false);
		}
	}
};

} // namespace gui2

#endif
