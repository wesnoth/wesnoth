/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifdef NEW_DRAW

#ifndef GUI_WIDGETS_GENERATOR_HPP_INCLUDED
#define GUI_WIDGETS_GENERATOR_HPP_INCLUDED

#include <boost/noncopyable.hpp>

#include "widget.hpp"
#include "config.hpp"
#include "gui/widgets/window_builder.hpp"

namespace gui2 {

class tgrid;

/**
 * Abstract base class for the generator.
 *
 * A generator is a class which holds multiple grids and controls their
 * placement on the screen. The final class is policy based, more info about
 * the possible policies is documented in the build() function. This function
 * is the factory to generate the classes as well.
 */
class tgenerator_ 
		: private boost::noncopyable
		, public twidget
{
	friend class tdebug_layout_graph;

public:	
	virtual ~tgenerator_() {}

	/** Determines how the items are placed. */
	enum tplacement 
			{ horizontal_list
			, vertical_list
			, grid
			, independant
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
	static tgenerator_* build(const bool has_minimum, const bool has_maximum,
			const tplacement placement, const bool select);

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
	virtual void select_item(const unsigned index, 
			const bool select = true) = 0;

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

	/** Returns the number of items. */
	virtual unsigned get_item_count() const = 0;

	/** Returns the number of selected items. */
	virtual unsigned get_selected_item_count() const = 0;

	/** Returns the first selected item, -1 if none selected. */
	virtual int get_selected_item() const = 0;

	/** Gets the grid of an item. */
	virtual tgrid& get_item(const unsigned index) = 0;

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
	 */
	virtual void create_item(const int index, 
			tbuilder_grid_const_ptr list_builder, 
			const string_map& item_data,
			void (*callback)(twidget*)) = 0; 

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
	 * @param item_data           The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	virtual void create_item(const int index, 
			tbuilder_grid_const_ptr list_builder, 
			const std::map<std::string /* widget id */, 
			string_map>& data,
			void (*callback)(twidget*)) = 0; 

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
	 * @param item_data           The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	virtual void create_items(const int index, 
			tbuilder_grid_const_ptr list_builder, 
			const std::vector<string_map>& data,
			void (*callback)(twidget*)) = 0; 

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
	 * @param item_data           The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	virtual void create_items(const int index, 
			tbuilder_grid_const_ptr list_builder, 
			const std::vector<std::map<std::string /*widget id*/,
			string_map> >& data,
			void (*callback)(twidget*)) = 0; 

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/* 
	 * These functions must be defined in our child classes so make sure they
	 * become pure virtuals.
	 */

	/** Inherited from twidget. */
	virtual void layout_init() = 0;

	/** Inherited from twidget. */
	virtual tpoint calculate_best_size() const = 0;

	/** Inherited from twidget. */
	virtual void set_size(const tpoint& origin, const tpoint& size) = 0;

	/** Inherited from twidget. */
	virtual void draw_children(surface& frame_buffer) = 0;

	/** Inherited from twidget. */
	virtual void child_populate_dirty_list(twindow& caller, 
			const std::vector<twidget*>& call_stack) = 0;

	/** Inherited from twidget. */
	virtual twidget* find_widget(
			const tpoint& coordinate, const bool must_be_active) = 0;

	/** Inherited from twidget. */
	virtual const twidget* find_widget(
			const tpoint& coordinate, const bool must_be_active) const = 0;

protected:

	/** Gets the grid of an item. */
	virtual const tgrid& get_item(const unsigned index) const = 0;

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
};

} // namespace gui2

#endif
#endif

