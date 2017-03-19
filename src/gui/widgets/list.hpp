/*
   Copyright (C) 2010 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_LIST_HPP_INCLUDED
#define GUI_WIDGETS_LIST_HPP_INCLUDED

#ifdef GUI2_EXPERIMENTAL_LISTBOX

#include "gui/widgets/generator.hpp"
#include "gui/widgets/scrollbar_container.hpp"

#include <boost/dynamic_bitset.hpp>

namespace gui2
{

/**
 * The list class.
 *
 * For now it's a generic for all kind of lists, horizontal, vertical etc.
 * Might be that there will be different types per class, not sure yet.
 */
class list_view : public container_base
{
	friend class debug_layout_graph;

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
	list_view(const bool has_minimum,
		  const bool has_maximum,
		  const generator_base::tplacement placement,
		  const bool select,
		  const builder_grid_const_ptr list_builder);

	/***** ***** ***** ***** Row handling. ***** ***** ****** *****/
	/**
	 * When an item in the list is selected by the user we need to
	 * update the state. We installed a callback handler which
	 * calls us.
	 *
	 * @param item                The data send to the set_members of the
	 *                            widgets.
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 */
	void add_row(const string_map& item, const int index = -1);

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
	 *                            undefined behavior.
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 */
	void add_row(const std::map<std::string /* widget id */, string_map>& data,
				 const int index = -1);

	/**
	 * Appends several rows to the grid.
	 *
	 * @param items               The data to send to the set_members of the
	 *                            widgets.
	 */
	void append_rows(const std::vector<string_map>& items);

	/**
	 * Removes a row in the listbox.
	 *
	 * @param row                 The row to remove, when not in
	 *                            range the function is ignored.
	 * @param count               The number of rows to remove, 0 means all
	 *                            rows (starting from row).
	 */
	void remove_row(const unsigned row, unsigned count = 1);

	/** Removes all the rows in the listbox, clearing it. */
	void clear();

	/** Returns the number of items in the listbox. */
	unsigned get_item_count() const;

	/**
	 * Makes a row active or inactive.
	 *
	 * NOTE this doesn't change the select status of the row.
	 *
	 * @param row                 The row to (de)activate.
	 * @param active              true activate, false deactivate.
	 */
	void set_row_active(const unsigned row, const bool active);

	/**
	 * Makes a row visible or invisible.
	 *
	 * @param row                 The row to show or hide.
	 * @param shown               true visible, false invisible.
	 */
	void set_row_shown(const unsigned row, const bool shown);

	/**
	 * Makes a row visible or invisible.
	 *
	 * Use this version if you want to show hide multiple items since it's
	 * optimized for that purpose, for one it calls the selection changed
	 * callback only once instead of several times.
	 *
	 * @param shown               A vector with the show hide status for every
	 *                            row. The number of items in the vector must
	 *                            be equal to the number of items in the
	 *                            listbox.
	 */
	void set_row_shown(const boost::dynamic_bitset<>& shown);

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
	const grid* get_row_grid(const unsigned row) const;

	/**
	 * The possibly-giving-problems nonconst version of get_row_grid
	 *
	 * @param row                 The row to get the grid from, the caller has
	 *                            to make sure the row is a valid row.
	 * @returns                   The grid of the wanted row.
	 */
	grid* get_row_grid(const unsigned row);

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
#if 0
	/**
	 * Request to update the size of the content after changing the content.
	 *
	 * When a resize is required the container first can try to handle it
	 * itself. If it can't honor the request the function will call @ref
	 * window::invalidate_layout().
	 *
	 * @note Calling this function on a widget with size == (0, 0) results
	 * false but doesn't call invalidate_layout, the engine expects to be in
	 * build up phase with the layout already invalidated.
	 *
	 * @returns                      True if the resizing succeeded, false
	 *                               otherwise.
	 */
	bool update_content_size();
#endif
	/***** ***** ***** ***** inherited ***** ***** ****** *****/

	/** Inherited from tcontrol_. */
	void init();

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
		COUNT
	};

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this class, it's stored in the content_grid_
	 * of the scrollbar_container super class and freed when it's grid is
	 * freed.
	 */
	generator_base* generator_;

	/** Contains the builder for the new items. */
	builder_grid_const_ptr list_builder_;

	bool need_layout_;
#if 0
	/**
	 * Resizes the content.
	 *
	 * The resize either happens due to resizing the content or invalidate the
	 * layout of the window.
	 *
	 * @param width_modification  The wanted modification to the width:
	 *                            * negative values reduce width.
	 *                            * zero leave width as is.
	 *                            * positive values increase width.
	 * @param height_modification The wanted modification to the height:
	 *                            * negative values reduce height.
	 *                            * zero leave height as is.
	 *                            * positive values increase height.
	 */
	void resize_content(
			  const int width_modification
			, const int height_modification);
#endif
	/** Layouts the children if needed. */
	void layout_children(const bool force);
#if 0
	/** Inherited from scrollbar_container. */
	virtual void set_content_size(const point& origin, const point& size);
#endif
	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_left_button_down(const event::ui_event event);

	void signal_handler_pre_child_left_button_click(grid* grid,
													const event::ui_event event,
													bool& handled,
													bool& halt);

	void signal_handler_left_button_click(grid* grid,
										  const event::ui_event event);

	void signal_handler_sdl_key_down(const event::ui_event event,
									 bool& handled,
									 const SDL_Keycode key,
									 SDL_Keymod modifier);
};

typedef list_view listbox;

} // namespace gui2

#endif
#endif
