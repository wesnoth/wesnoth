/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_GENERATOR_HPP_INCLUDED
#define GUI_WIDGETS_GENERATOR_HPP_INCLUDED

#include "widget.hpp"
#include "tstring.hpp"

#include <boost/intrusive_ptr.hpp>

typedef std::map<std::string, t_string> string_map;

namespace gui2
{

struct tbuilder_grid;
typedef boost::intrusive_ptr<const tbuilder_grid> tbuilder_grid_const_ptr;

class tgrid;

/**
 * Abstract base class for the generator.
 *
 * A generator is a class which holds multiple grids and controls their
 * placement on the screen. The final class is policy based, more info about
 * the possible policies is documented in the build() function. This function
 * is the factory to generate the classes as well.
 */
class tgenerator_ : public twidget
{
	friend class tdebug_layout_graph;

public:
	virtual ~tgenerator_()
	{
	}

	/** Determines how the items are placed. */
	enum tplacement {
		horizontal_list,
		vertical_list,
		grid,
		independent
	};

	/**
	 * Create a new generator.
	 *
	 * @param has_minimum         Does one item need to be selected.
	 * @param has_maximum         Is one the maximum number of items that can
	 *                            be selected?
	 * @param placement           The placement of the grids, see tplacement
	 *                            for more info.
	 * @param select              If a grid is selected, what should happen?
	 *                            If true the grid is selected, if false the
	 *                            grid is shown.
	 *
	 * @returns                   A pointer to a new object. The caller gets
	 *                            ownership of the new object.
	 */
	static tgenerator_* build(const bool has_minimum,
							  const bool has_maximum,
							  const tplacement placement,
							  const bool select);

	/**
	 * Deletes an item.
	 */
	virtual void delete_item(const unsigned index) = 0;

	/** Deletes all items. */
	virtual void clear() = 0;

	/**
	 * (De)selects an item.
	 *
	 * @param index               The item to (de)select.
	 * @param select              If true selects, if false deselects.
	 */
	virtual void select_item(const unsigned index, const bool select) = 0;

	/**
	 * Toggles the selection state of an item.
	 *
	 * @param index               The item to toggle.
	 */
	void toggle_item(const unsigned index)
	{
		select_item(index, !is_selected(index));
	}

	/** Returns whether the item is selected. */
	virtual bool is_selected(const unsigned index) const = 0;

	/**
	 * Shows or hides an item.
	 *
	 * The caller is responsible for reformatting the grid.
	 *
	 * @param index               The item to show or hide.
	 * @param show                If true shows the item, else hides it.
	 */
	virtual void set_item_shown(const unsigned index, const bool show) = 0;

	/** Returns whether the item is shown. */
	virtual bool get_item_shown(const unsigned index) const = 0;

	/** Returns the number of items. */
	virtual unsigned get_item_count() const = 0;

	/** Returns the number of selected items. */
	virtual unsigned get_selected_item_count() const = 0;

	/**
	 * Returns the selected item.
	 *
	 * If a list has multiple selected items it looks whether it knows the last
	 * item actually selected, if that item is selected that one is chosen.
	 * Else is goes through all selected items and returns the first one
	 * selected.
	 *
	 * @note tstacked_widget depends on that behavior it always has all items
	 * selected and thus shown and by default the last selected item (the top
	 * one) is active.
	 *
	 * @returns                   The selected item, -1 if none selected.
	 */
	virtual int get_selected_item() const = 0;

	/** Gets the grid of an item. */
	virtual tgrid& item(const unsigned index) = 0;

	/** Gets the grid of an item. */
	virtual const tgrid& item(const unsigned index) const = 0;

	/***** ***** ***** ***** Create items ***** ***** ***** *****/

	/**
	 * Creates a new item.
	 *
	 * The item_data is used for the first widget found, this normally should
	 * be used when there's one widget in an item.
	 *
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 * @param list_builder        A grid builder that's will build the
	 *                            contents of the new item.
	 * @param item_data           The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 *
	 * @returns                   A reference to the newly created grid.
	 */
	virtual tgrid& create_item(const int index,
							   tbuilder_grid_const_ptr list_builder,
							   const string_map& item_data,
							   const boost::function<void(twidget&)>& callback)
			= 0;

	/**
	 * Creates a new item.
	 *
	 * The item_data is used by id, and is meant to set multiple widgets in
	 * an item.
	 *
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 * @param list_builder        A grid builder that's will build the
	 *                            contents of the new item.
	 * @param data                The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 *
	 * @returns                   A reference to the newly created grid.
	 */
	virtual tgrid&
	create_item(const int index,
				tbuilder_grid_const_ptr list_builder,
				const std::map<std::string /* widget id */, string_map>& data,
				const boost::function<void(twidget&)>& callback) = 0;

	/**
	 * Creates one or more new item(s).
	 *
	 * For every item in item_data a new item is generated. This version
	 * expects one widget per item.
	 *
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 * @param list_builder        A grid builder that's will build the
	 *                            contents of the new item.
	 * @param data                The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	virtual void create_items(const int index,
							  tbuilder_grid_const_ptr list_builder,
							  const std::vector<string_map>& data,
							  const boost::function<void(twidget&)>& callback)
			= 0;

	/**
	 * Creates one or more new item(s).
	 *
	 * For every item in item_data a new item is generated. This version
	 * expects multiple widgets per item.
	 *
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 * @param list_builder        A grid builder that's will build the
	 *                            contents of the new item.
	 * @param data                The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	virtual void create_items(
			const int index,
			tbuilder_grid_const_ptr list_builder,
			const std::vector<std::map<std::string /*widget id*/, string_map> >&
					data,
			const boost::function<void(twidget&)>& callback) = 0;

	typedef boost::function<bool (unsigned, unsigned)> torder_func;
	virtual void set_order(const torder_func& order) = 0;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/*
	 * These functions must be defined in our child classes so make sure they
	 * become pure virtuals.
	 */

	/** See @ref twidget::layout_initialise. */
	virtual void layout_initialise(const bool full_initialisation) OVERRIDE = 0;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) OVERRIDE
			= 0;

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) OVERRIDE
			= 0;

	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE = 0;

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE = 0;

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE = 0;

	/** See @ref twidget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) OVERRIDE = 0;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer) OVERRIDE = 0;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) OVERRIDE = 0;

protected:
	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) OVERRIDE
			= 0;

public:
	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE = 0;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE
			= 0;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/**
	 * Up arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_up_arrow(SDLMod modifier, bool& handled) = 0;

	/**
	 * Down arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_down_arrow(SDLMod modifier, bool& handled) = 0;

	/**
	 * Left arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_left_arrow(SDLMod modifier, bool& handled) = 0;

	/**
	 * Right arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_right_arrow(SDLMod modifier, bool& handled) = 0;

protected:
	/**
	 * Selects a not selected item.
	 *
	 * @param index               The index of a not selected item.
	 */
	virtual void do_select_item(const unsigned index) = 0;

	/**
	 * Deselects a selected item.
	 *
	 * @param index               The index of a selected item.
	 */
	virtual void do_deselect_item(const unsigned index) = 0;

	/** Gets the grid of an item. */
	virtual tgrid& item_ordered(const unsigned index) = 0;

	/** Gets the grid of an item. */
	virtual const tgrid& item_ordered(const unsigned index) const = 0;

	virtual unsigned get_ordered_index(unsigned index) const = 0;
	virtual unsigned get_item_at_ordered(unsigned index_ordered) const = 0;
};

} // namespace gui2

#endif
