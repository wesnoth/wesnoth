/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_STACKED_WIDGET_HPP_INCLUDED
#define GUI_WIDGETS_STACKED_WIDGET_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2 {

namespace implementation {
	struct tbuilder_stacked_widget;
}

class tgenerator_;

class tstacked_widget
		: public tcontainer_
{
	friend struct implementation::tbuilder_stacked_widget;
	friend class tdebug_layout_graph;

public:

	tstacked_widget();

	/***** ***** ***** ***** Stack handling. ***** ***** ****** *****/

	/**
	 * Adds single item to the grid.
	 *
	 * This function expect a item to one multiple widget.
	 *
	 * @param item                The data to send to the set_members of the
	 *                            widget.
	 */
	void add_item(const string_map& item);

	/**
	 * Adds single item to the grid.
	 *
	 * This function expect a item to have multiple widgets (either multiple
	 * columns or one column with multiple widgets).
	 *
	 *
	 * @param data                The data to send to the set_members of the
	 *                            widgets. If the member id is not an empty
	 *                            string it is only send to the widget that
	 *                            has the wanted id (if any). If the member
	 *                            id is an empty string, it is send to all
	 *                            members. Having both empty and non-empty
	 *                            id's gives undefined behaviour.
	 */
	void add_item(const std::map<std::string /* widget id */,
			string_map>& data);

	/** Returns the number of items. */
	unsigned get_item_count() const;

	/***** ***** ***** inherited ***** ****** *****/

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return 0; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_item_builder(tbuilder_grid_ptr item_builder)
		{ item_builder_ = item_builder; }

private:

	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param item_data           The initial data to fill the widget with.
	 */
	void finalize(const std::vector<string_map>& item_data);

	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this variable.
	 */
	tgenerator_* generator_;

	/** Contains the builder for the new items. */
	tbuilder_grid_const_ptr item_builder_;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const
		{ static const std::string type = "stacked_widget"; return type; }

	/** Inherited from tcontainer_. */
	void set_self_active(const bool /*active*/) {}
};

} // namespace gui2

#endif

