/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/generator.hpp"
#include "gui/widgets/scrollbar_container.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

#include "preferences/preferences.hpp"

#include <boost/dynamic_bitset.hpp>
#include <functional>

namespace gui2
{
// ------------ WIDGET -----------{

class selectable_item;
namespace implementation
{
struct builder_listbox_base;
}

/** The listbox class. */
class listbox : public scrollbar_container
{
	friend struct implementation::builder_listbox_base;

	friend class debug_layout_graph;

public:
	/**
	 * Constructor.
	 *
	 * @param builder             The builder for the appropriate listbox variant.
	 */
	listbox(const implementation::builder_listbox_base& builder);

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
	grid& add_row(const widget_item& item, const int index = -1);

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
	grid& add_row(const widget_data& data, const int index = -1);

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
	 * Hides all rows for which the given predicate returns false.
	 *
	 * @returns                   The number of rows now visible.
	 */
	std::size_t filter_rows_by(const std::function<bool(std::size_t)>& filter);

	/**
	 * Returns a list of visible rows
	 *
	 * @returns                   A mask indicating which rows are visible
	 */
	boost::dynamic_bitset<> get_rows_shown() const;

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
	 * Does exactly as advertised: selects the list's last row.
	 *
	 * @param select              Select or deselect the row.
	 */
	bool select_last_row(const bool select = true)
	{
		return select_row(get_item_count() - 1, select);
	}

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

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void order_by(const generator_base::order_func& func);

private:
	struct sort_helper
	{
		template<typename T>
		static bool less(const T& lhs, const T& rhs) { return lhs < rhs; }

		/** Performs case-insensitive comparison using the current locale. */
		static bool less(const t_string& lhs, const t_string& rhs);

		template<typename T>
		static bool more(const T& lhs, const T& rhs) { return lhs > rhs; }

		/** Performs case-insensitive comparison using the current locale. */
		static bool more(const t_string& lhs, const t_string& rhs);
	};

	/** Implementation detail of @ref set_single_sorter */
	void initialize_sorter(std::string_view id, generator_sort_array&&);

	/** Implementation detail of @ref set_sorters */
	template<std::size_t... Is, typename... Args>
	void set_sorters_impl(std::index_sequence<Is...>, Args&&... fs)
	{
		(set_single_sorter("sort_" + std::to_string(Is), fs), ...);
	}

public:
	/**
	 * Registers a single sorting control by ID.
	 *
	 * @param id           The ID of the selectable_item header widget to bind to.
	 * @param f            Any callable whose result is sortable.
	 */
	template<typename Func>
	void set_single_sorter(std::string_view id, const Func& f)
	{
		initialize_sorter(id, {
			[f](int lhs, int rhs) { return sort_helper::less(f(lhs), f(rhs)); },
			[f](int lhs, int rhs) { return sort_helper::more(f(lhs), f(rhs)); }
		});
	}

	/**
	 * Registers sorting controls using magic index IDs.
	 *
	 * This function accepts any callable whose result is sortable. Each callable passed
	 * will be bound to a corresponding selectable_item widget in the header, if present,
	 * whose ID is sort_N, where N is the index of the callable in the parameter pack.
	 *
	 * @param functors     Zero or more callables with the signature T(std::size_t).
	 */
	template<typename... Args>
	void set_sorters(Args&&... functors)
	{
		set_sorters_impl(std::index_sequence_for<Args...>{}, std::forward<Args>(functors)...);
	}

	/**
	 * Sorts the listbox by a pre-set sorting option. The corresponding header widget
	 * will also be toggled. The sorting option should already have been registered by
	 * @ref listbox::set_sorters().
	 *
	 * @param id              The id of the sorter widget whose value to set.
	 * @param order           The order to sort by (ascending, descending, or none).
	 * @param select_first    If true, the first row post-sort will be selected.
	 *                        If false (default), the selected row will be maintained
	 *                        post-sort as per standard sorting functionality.
	 */
	void set_active_sorter(std::string_view id, sort_order::type order, bool select_first = false);

	/** Returns a widget pointer to the active sorter, along with its corresponding order. */
	std::pair<widget*, sort_order::type> get_active_sorter() const;

	/** Deactivates all sorting toggle buttons at the top, making the list look like it's not sorted. */
	void mark_as_unsorted();

	/** Registers a callback to be called when the active sorting option changes. */
	void set_callback_order_change(std::function<void(unsigned, sort_order::type)> callback)
	{
		callback_order_change_ = callback;
	}

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
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

	enum KEY_SCROLL_DIRECTION { KEY_VERTICAL, KEY_HORIZONTAL };

	/** Helper to update visible area after a key event. */
	void update_visible_area_on_key_event(const KEY_SCROLL_DIRECTION direction);

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
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this class, it's stored in the content_grid_
	 * of the scrollbar_container super class and freed when it's grid is freed.
	 */
	generator_base* generator_;

	generator_base::placement placement_;

	/** Contains the builder for the new items. */
	builder_grid_const_ptr list_builder_;

	std::vector<std::pair<selectable_item*, generator_sort_array>> orders_;

	std::function<void(unsigned, sort_order::type)> callback_order_change_;

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
	 * @param width_modification_pos
	 * @param height_modification_pos
	 */
	void resize_content(const int width_modification,
			const int height_modification,
			const int width_modification_pos = -1,
			const int height_modification_pos = -1);

	/**
	 * Resizes the content.
	 *
	 * The resize happens when a new row is added to the contents.
	 *
	 * @param row                 The new row added to the listbox.
	 */
	void resize_content(const widget& row);

	/** Updates internal layout. */
	void update_layout();

	/** Inherited from scrollbar_container. */
	virtual void set_content_size(const point& origin, const point& size) override;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
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
struct builder_listbox_base : public builder_scrollbar_container
{
	explicit builder_listbox_base(const config& cfg, const generator_base::placement placement);

	using builder_styled_widget::build;

	/** Inherited from builder_widget */
	virtual std::unique_ptr<widget> build() const override;

	/** Flag for vertical, horizontal, or grid placement. */
	generator_base::placement placement;

	builder_grid_ptr header;
	builder_grid_ptr footer;

	builder_grid_ptr list_builder;

	/**
	 * Listbox data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<widget_data> list_data;

	bool has_minimum, has_maximum, allow_selection;
};

struct builder_listbox : public builder_listbox_base
{
	explicit builder_listbox(const config& cfg);
};

struct builder_horizontal_listbox : public builder_listbox_base
{
	explicit builder_horizontal_listbox(const config& cfg)
		: builder_listbox_base(cfg, generator_base::horizontal_list)
	{
	}
};

struct builder_grid_listbox : public builder_listbox_base
{
	explicit builder_grid_listbox(const config& cfg)
		: builder_listbox_base(cfg, generator_base::table)
	{
	}
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
