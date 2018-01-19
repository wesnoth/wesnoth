/*
   Copyright (C) 2008 - 2018 The Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/core/event/dispatcher.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/widget.hpp"
#include "utils/functional.hpp"

#include <map>
#include <vector>

namespace gui2
{

template<class T>
class group
{
	using group_map    = std::map<T, selectable_item*>;
	using order_vector = std::vector<styled_widget*>;

public:
	/**
	 * Adds a widget/value pair to the group map. A callback is set that toggles each members'
	 * state to false when clicked. This happens before individual widget handlers fire, ensuring
	 * that the clicked widget will remain the only one selected.
	 */
	void add_member(selectable_item* w, const T& value)
	{
		bool success;
		std::tie(std::ignore, success) = members_.emplace(value, w);

		if(!success) {
			ERR_GUI_G << "Group member with value " << value << "already exists." << std::endl;
			return;
		}

		dynamic_cast<widget&>(*w).connect_signal<event::LEFT_BUTTON_CLICK>(
			std::bind(&group::group_operator, this), event::dispatcher::front_child);

		member_order_.push_back(dynamic_cast<styled_widget*>(w));
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
	 * Returns the value paired with the currently actively toggled member of the group.
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
	void set_callback_on_value_change(std::function<void(widget&)> func)
	{
		// Ensure this callback is only called on the member being activated
		const auto callback = [func](widget& widget)->void {
			if(dynamic_cast<selectable_item&>(widget).get_value_bool()) {
				func(widget);
			}
		};

		for(auto& member : members_) {
			event::connect_signal_notify_modified(dynamic_cast<widget&>(*member.second), std::bind(callback, _1));
		}
	}

	/**
	 * Wrapper for enabling or disabling member widgets.
	 * Each member widget will be enabled or disabled based on the result of the specified
	 * predicate, which takes its associated value.
	 *
	 * If a selected widget is to be disabled, it is deselected and the first active member
	 * selected instead. The same happens if no members were previously active at all.
	 */
	void set_members_enabled(std::function<bool(const T&)> predicate)
	{
		bool do_reselect = true;

		for(auto& member : members_) {
			const bool res = predicate(member.first);

			selectable_item& w = *member.second;
			dynamic_cast<styled_widget&>(w).set_active(res);

			// Only select another member if this was selected
			if(w.get_value_bool()) {
				do_reselect = !res;

				if(do_reselect) {
					w.set_value_bool(false);
				}
			}
		}

		if(!do_reselect) {
			return;
		}

		// Look for the first active member to select
		for(auto& member : member_order_) {
			if(member->get_active()) {
				dynamic_cast<selectable_item&>(*member).set_value_bool(true);
				break;
			}
		}
	}

private:
	/**
	 * Container map for group members, organized by member value, associated widget.
	 */
	group_map members_;

	/**
	 * Since iterating over std::map is specified by operator< for it's key values, we can't
	 * guarantee the order would line up with the logical order - ie, that which the widgets
	 * appear in in a specific dialog. Keeping a separate vector here allows iterating over
	 * members in the order which they are added to the group.
	 */
	order_vector member_order_;

	/**
	 * The default actions to take when clicking on one of the widgets in the group.
	 */
	void group_operator()
	{
		for(auto& member : members_) {
			member.second->set_value(false);
		}
	}
};

} // namespace gui2
