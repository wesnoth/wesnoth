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

#ifndef __GUI_WIDGETS_LISTBOX_HPP_INCLUDED__
#define __GUI_WIDGETS_LISTBOX_HPP_INCLUDED__

#include "gui/widgets/container.hpp"

class t_string;

namespace gui2 {

class tscrollbar_;

//! @todo list
//! header row + footer row same width as client data
//! cell or row select
//! sort at some way
//
//maybe create two types 1 fixed size and one with a builder to add new rows

class tlistbox : public tcontainer_
{
	friend class tbuilder_listbox;
public:
	
	tlistbox();

	void set_active(const bool active) { set_state(active ? ENABLED : DISABLED); };
	bool get_active() const { return state_ != DISABLED; }
	unsigned get_state() const { return state_; }

	/** Inherited from twidget. */
	bool has_vertical_scrollbar() const { return true; }

	/** 
	 * When an item in the list is selected by the user we need to
	 * update the state. We installed a callback handler which 
	 * calls us.
	 */
	void list_item_selected(twidget* caller);

	/**
	 * Callback when the scrollbar moves.
	 */
	void scrollbar_moved(twidget* /*caller*/)
		{ set_scrollbar_button_status(); set_dirty(); }

	/** 
	 * When an item scrollbar control button is clicked we need to move the
	 * scrollbar and update the list. 
	 */
	void scrollbar_click(twidget* caller);

	/** The builder needs to call us so we can write in the proper callbacks. */
	void finalize_setup();

	void set_list_builder(tbuilder_grid* list_builder) 
		{ list_builder_ = list_builder; }

	void set_assume_fixed_row_size(bool assume = true) 
		{ assume_fixed_row_size_ = assume; }

	/** Inherited from tcontainer. */
	tpoint get_best_size() const;

	/** Inherited from tcontainer. */
	void draw(surface& surface);

	/** Inherited from tcontainer. */
	void set_size(const SDL_Rect& rect);

	/** Inherited from tcontainer. */
	twidget* get_widget(const tpoint& coordinate);

	/**
	 * Adds an item to the list, it requires the builder_list to be defined. 
	 * NOTE this is for a listbox with one item per row, for multiple items
	 * there will be a version with gets a vector with values. Probably there
	 * also will be a version which gets the name of an image next to the
	 * label so we can define lists even easier.
	 *
	 * Probably the hardcoded list will disappear as well at some point
	 */
	void add_item(const t_string& label);

	unsigned get_item_count() const { return rows_.size(); }

	/** 
	 * Selects an entire row. 
	 *
	 * @param row                 The row to (de)select.
	 * @param select              true select, false deselect.
	 *
	 * @returns                   false if deselecting wasn't allowed.
	 *                            true otherwise.
	 */
	bool select_row(const unsigned row, const bool select = true);

	unsigned get_selected_row() const { return selected_row_; }

private:

	/** 
	 * Sets the status of the scrollbar buttons.
	 *
	 * This is needed after the scrollbar moves so the status of the buttons
	 * will be active or inactive as needed.
	 */
	void set_scrollbar_button_status();

	//! Note the order of the states must be the same as defined in settings.hpp.
	enum tstate { ENABLED, DISABLED, COUNT };

	void set_state(tstate state) {} // FIXME implement
	tstate state_;

	/** It's possible to let the engine build the contents, we need the builder in that case */
	tbuilder_grid* list_builder_;

	/** Returns the scrollbar widget */
	tscrollbar_* scrollbar();

	bool assume_fixed_row_size_;

	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "listbox"; return type; }

	/** The (lastly) selected row */
	unsigned selected_row_;

	/** Number of items selected */
	unsigned selection_count_;

	/** Select per cell or an entire row */
	bool row_select_;

	/** At least 1 item must be selected */
	bool must_select_;

	/** Multiple items can be selected */
	bool multi_select_; 
	
	/** The sizes of the spacer. */
	SDL_Rect list_rect_;

	/** The background of the list, needed for redrawing. */
	surface list_background_;

	/** The best size for the spacer, if not set it's calculated. */
	tpoint best_spacer_size_;

	class trow {

	public:
		trow(const tbuilder_grid& list_builder_, const t_string& label);

		void select(const bool sel = true);
	
		tgrid* grid() { return grid_; }
		const tgrid* grid() const { return grid_; }

		void set_height(const unsigned height) { height_ = height; }
		unsigned get_height() const { return height_; }

		const surface& canvas() const { return canvas_; }
		surface& canvas() { return canvas_; }

		bool get_selected() const { return selected_; }
	private:

		tgrid* grid_;

		unsigned height_;

		surface canvas_;

		bool selected_;

		void init_in_grid(tgrid* grid, const t_string& label);

		void select_in_grid(tgrid* grid, const bool sel);
	};

	std::vector<trow> rows_;

};

} // namespace gui2

#endif


