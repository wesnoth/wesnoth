/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#ifndef GUI2_EXPERIMENTAL_LISTBOX

#include "gui/widgets/generator.hpp"
#include "gui/widgets/scrollbar_container.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

#include <boost/dynamic_bitset.hpp>

namespace gui2
{
// ------------ WIDGET -----------{

class selectable_item;
namespace implementation
{
struct builder_listbox;
struct builder_horizontal_listbox;
struct builder_grid_listbox;
struct builder_styled_widget;
}

/** The listbox class. */
class listbox : public scrollbar_container
{
	friend struct implementation::builder_listbox;
	friend struct implementation::builder_horizontal_listbox;
	friend struct implementation::builder_grid_listbox;
	friend class debug_layout_graph;

public:
	/**
	 * Constructor.
	 *
	 * @param builder             The builder for the appropriate listbox variant.
	 * @param placement           How are the items placed.
	 * @param list_builder        Grid builder for the listbox definition grid.
	 * @param has_minimum         Does the listbox need to have one item selected.
	 * @param has_maximum         Can the listbox only have one item selected.
	 * @param select              Select an item when selected. If false it changes
	 *                            the visible state instead. Default true.
	 */
	listbox(const implementation::builder_styled_widget& builder,
			const generator_base::placement placement,
			builder_grid_ptr list_builder,
			const bool has_minimum,
			const bool has_maximum,
			const bool select = true);

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
	grid& add_row(const string_map& item, const int index = -1);

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
	grid& add_row(const std::map<std::string /* widget id */, string_map>& data, const int index = -1);

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
	 * Returns a list of visible rows
	 *
	 * @returns                   A mask indicating which rows are visible
	 */
	boost::dynamic_bitset<> get_rows_shown() const;

	bool any_rows_shown() const;

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
	 * Selects a row.
	 *
	 * @param row                 The row to select.
	 * @param select              Select or deselect the row.
	 * @returns                   True if the operation succeeded.
	 */
	bool select_row(const unsigned row, const bool select = true);

	/**
	 * Selects a row at the given position, regardless of sorting order.
	 *
	 * When using @ref select_row the relevant row is located by index regardless
	 * of its actual position in the list, which could differ if the list had been
	 * sorted. In that case, `select_row(0)` would not select the list's first row
	 * as displayed.
	 *
	 * This function allows row selection based on position. `select_row_at(0)` will
	 * always select the list's first row, regardless of sorting order.
	 *
	 * @param row                 The row to select.
	 * @param select              Select or deselect the row.
	 *
	 * @returns                   True if the operation succeeded.
	 */
	bool select_row_at(const unsigned row, const bool select = true);

	/**
	 * Check if a row is selected
	 * @param row                 The row to test
	 * @returns                   True if it is selected.
	 */
	bool row_selected(const unsigned row);

	/**
	 * Returns the first selected row
	 *
	 * @returns                   The first selected row, or -1 if no row is selected.
	 */
	int get_selected_row() const;

	/** Function to call after the user clicked on a row. */
	void list_item_clicked(widget& caller);

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;

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

	/***** ***** ***** ***** inherited ***** ***** ****** *****/

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/** See @ref widget::layout_children. */
	virtual void layout_children() override;

	/** See @ref widget::child_populate_dirty_list. */
	virtual void child_populate_dirty_list(window& caller, const std::vector<widget*>& call_stack) override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void order_by(const generator_base::order_func& func);

	void set_column_order(unsigned col, const generator_sort_array& func);

	template<typename Func>
	void register_sorting_option(const int col, const Func& f)
	{
		set_column_order(col, {{
			[f](int lhs, int rhs) { return f(lhs) < f(rhs); },
			[f](int lhs, int rhs) { return f(lhs) > f(rhs); }
		}});
	}

	enum SORT_ORDER {
		SORT_NONE,
		SORT_ASCENDING,
		SORT_DESCENDING,
	};

	using order_pair = std::pair<int, SORT_ORDER>;

	/**
	 * Sorts the listbox by a pre-set sorting option. The corresponding header widget will also be toggled.
	 * The sorting option should already have been registered by @ref listbox::register_sorting_option().
	 *
	 * @param sort_by         Pair of column index and sort direction. The column (first arguemnt)
	 *                        argument will be sorted in the specified direction (second argument)
	 *
	 * @param select_first    If true, the first row post-sort will be selected. If false (default),
	 *                        the selected row will be maintained post-sort  as per standard sorting
	 *                        functionality.
	 */
	void set_active_sorting_option(const order_pair& sort_by, const bool select_first = false);

	const order_pair get_active_sorting_option();

protected:
	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from scrollbar_container. */
	void handle_key_up_arrow(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from scrollbar_container. */
	void handle_key_down_arrow(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from scrollbar_container. */
	void handle_key_left_arrow(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from scrollbar_container. */
	void handle_key_right_arrow(SDL_Keymod modifier, bool& handled) override;

private:
	/** Helper to update visible area after a key event. */
	void update_visible_area_on_key_event(const bool key_direction_vertical);

	/**
	 * @todo A listbox must have the following config parameters in the
	 * instantiation:
	 * - fixed row height?
	 * - fixed column width?
	 * and if so the following ways to set them
	 * - fixed depending on header ids
	 * - fixed depending on footer ids
	 * - fixed depending on first row ids
	 * - fixed depending on list (the user has to enter a list of ids)
	 *
	 * For now it's always fixed width depending on the first row.
	 */

	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param header              Builder for the header.
	 * @param footer              Builder for the footer.
	 * @param list_data           The initial data to fill the listbox with.
	 */
	void finalize(builder_grid_const_ptr header,
			builder_grid_const_ptr footer,
			const std::vector<std::map<std::string, string_map>>& list_data);
	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this class, it's stored in the content_grid_
	 * of the scrollbar_container super class and freed when it's grid is
	 * freed.
	 */
	generator_base* generator_;

	const bool is_horizontal_;

	/** Contains the builder for the new items. */
	builder_grid_const_ptr list_builder_;

	bool need_layout_;

	typedef std::vector<std::pair<selectable_item*, generator_sort_array>> torder_list;
	torder_list orders_;
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
	void resize_content(const int width_modification,
			const int height_modification,
			const int width__modification_pos = -1,
			const int height_modification_pos = -1);

	/**
	 * Resizes the content.
	 *
	 * The resize happens when a new row is added to the contents.
	 *
	 * @param row                 The new row added to the listbox.
	 */
	void resize_content(const widget& row);

	/** Layouts the children if needed. */
	void layout_children(const bool force);

	/** Inherited from scrollbar_container. */
	virtual void set_content_size(const point& origin, const point& size) override;

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	void order_by_column(unsigned column, widget& widget);
};

// }---------- DEFINITION ---------{

struct listbox_definition : public styled_widget_definition
{
	explicit listbox_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{
struct builder_listbox : public builder_styled_widget
{
	explicit builder_listbox(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

	scrollbar_container::scrollbar_mode vertical_scrollbar_mode;
	scrollbar_container::scrollbar_mode horizontal_scrollbar_mode;

	builder_grid_ptr header;
	builder_grid_ptr footer;

	builder_grid_ptr list_builder;

	/**
	 * Listbox data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<std::map<std::string, string_map>> list_data;

	bool has_minimum_, has_maximum_;
};

struct builder_horizontal_listbox : public builder_styled_widget
{
	explicit builder_horizontal_listbox(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

	scrollbar_container::scrollbar_mode vertical_scrollbar_mode;
	scrollbar_container::scrollbar_mode horizontal_scrollbar_mode;

	builder_grid_ptr list_builder;

	/**
	 * Listbox data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<std::map<std::string, string_map>> list_data;

	bool has_minimum_, has_maximum_;
};

struct builder_grid_listbox : public builder_styled_widget
{
	explicit builder_grid_listbox(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

	scrollbar_container::scrollbar_mode vertical_scrollbar_mode;
	scrollbar_container::scrollbar_mode horizontal_scrollbar_mode;

	builder_grid_ptr list_builder;

	/**
	 * Listbox data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<std::map<std::string, string_map>> list_data;

	bool has_minimum_, has_maximum_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
