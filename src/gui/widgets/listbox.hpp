/* $Id$ */
/*
   copyright (C) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/


#ifndef GUI_WIDGETS_LISTBOX_HPP_INCLUDED
#define GUI_WIDGETS_LISTBOX_HPP_INCLUDED

#include "gui/widgets/generator.hpp"
#include "gui/widgets/scrollbar_container.hpp"

namespace gui2 {

/** The listbox class. */
class tlistbox
		: public tscrollbar_container
{
	friend struct tbuilder_listbox;
	friend class tdebug_layout_graph;

public:
	/**
	 * Constructor.
	 *
	 * @param has_minimum         Does the listbox need to have one item
	 *                            selected.
	 * @param has_maximum         Can the listbox only have one item
	 *                            selected.
	 * @param placement           How are the items placed.
	 * @param select              Select an item when selected, if false it
	 *                            changes the visible state instead.
	 */
	tlistbox(const bool has_minimum, const bool has_maximum,
			const tgenerator_::tplacement placement, const bool select);

	/***** ***** ***** ***** Row handling. ***** ***** ****** *****/
	/**
	 * When an item in the list is selected by the user we need to
	 * update the state. We installed a callback handler which
	 * calls us.
	 */
	void add_row(const string_map& item);

	/**
	 * Adds single row to the grid.
	 *
	 * This function expect a row to have multiple widgets (either multiple
	 * columns or one column with multiple widgets).
	 *
	 *
	 * @param data                The data to send to the set_members of the
	 *                            widgets. If the member id is not an empty
	 *                            string it is only send to the widget that has
	 *                            the wanted id (if any). If the member id is an
	 *                            empty string, it is send to all members.
	 *                            Having both empty and non-empty id's gives
	 *                            undefined behaviour.
	 */
	void add_row(const std::map<std::string /* widget id */,
			string_map>& data);

	/** Returns the number of items in the listbox. */
	unsigned get_item_count() const;

	/**
	 * Makes a row active or inactive.
	 *
	 * NOTE this doesn't change the select status of the row.
	 *
	 * @param row                 The row to (de)activate.
	 * @param select              true activate, false deactivate.
	 */
	void set_row_active(const unsigned row, const bool active);

	/**
	 * Returns the grid of the wanted row.
	 *
	 * There's only a const version since allowing callers to modify the grid
	 * behind our backs might give problems. We return a pointer instead of a
	 * reference since dynamic casting of pointers is easier (no try catch
	 * needed).
	 *
	 * @param row                 The row to get the grid from, the caller has
	 *                            to make sure the row is a valid row.
	 * @returns                   The grid of the wanted row.
	 */
	const tgrid* get_row_grid(const unsigned row) const;

	/**
	 * Selectes a row.
	 *
	 * @param row                 The row to select.
	 * @param select              Select or deselect the row.
	 */
	bool select_row(const unsigned row, const bool select = true);

	/**
	 * Returns the first selected row
	 *
	 * @returns                   The first selected row.
	 * @retval -1                 No row selected.
	 */
	int get_selected_row() const;

	/** Function to call after the user clicked on a row. */
	void list_item_clicked(twidget* caller);

	/** Inherited from tcontainer_. */
	void set_self_active(const bool /*active*/)  {}
//		{ state_ = active ? ENABLED : DISABLED; }
//
	/***** ***** ***** ***** inherited ***** ***** ****** *****/

	/** Inherited from tscrollbar_container. */
	void child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	// FIXME implement
	void set_callback_value_change(void (*) (twidget* caller)) {}

	void set_list_builder(tbuilder_grid_ptr list_builder)
		{ list_builder_ = list_builder; }

private:

	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param header              Builder for the header.
	 * @param footer              Builder for the footer.
	 * @param list_data           The initial data to fill the listbox with.
	 */
	void finalize(
			tbuilder_grid_const_ptr header,
			tbuilder_grid_const_ptr footer,
			const std::vector<string_map>& list_data);
	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this variable.
	 */
	tgenerator_* generator_;

	/** Contains the builder for the new items. */
	tbuilder_grid_const_ptr list_builder_;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const
		{ static const std::string type = "listbox"; return type; }
};

} // namespace gui2

#endif

