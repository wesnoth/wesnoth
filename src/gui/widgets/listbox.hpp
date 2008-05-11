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

namespace gui2 {

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
	
	tlistbox() :
		tcontainer_(COUNT),
		state_(ENABLED),
		list_builder_(0),
		assume_fixed_row_size_(true),
		selected_row_(-1)
	{
		load_config();
	}

	void set_active(const bool active) { set_state(active ? ENABLED : DISABLED); };
	bool get_active() const { return state_ != DISABLED; }
	unsigned get_state() const { return state_; }

	/** 
	 * When an item in the list is selected by the user we need to
	 * update the state. We installed a callback handler which 
	 * calls us.
	 */
	void list_item_selected(twidget* caller);

	/** The builder needs to call us so we can write in the proper callbacks. */
	void finalize_setup();

	void set_list_builder(tbuilder_grid* list_builder) 
		{ list_builder_ = list_builder; }

	void set_assume_fixed_row_size(bool assume = true) 
		{ assume_fixed_row_size_ = assume; }

	/**
	 * Adds an item to the list, it requires the builder_list to be defined. 
	 * NOTE this is for a listbox with one item per row, for multiple items
	 * there will be a version with gets a vector with values. Probably there
	 * also will be a version which gets the name of an image next to the
	 * label so we can define lists even easier.
	 *
	 * Probably the hardcoded list will disappear as well at some point
	 */
	void add_item(const std::string& label);

	unsigned get_item_count() /*const*/;

	/** Selects an entire row. */
	void select_row(const unsigned row, const bool select = true);

	/** Selects a single cell */
	void select_cell(const unsigned row, const unsigned column, const bool select = true);

	unsigned get_selected_row() const { return selected_row_; }

private:
	//! Note the order of the states must be the same as defined in settings.hpp.
	enum tstate { ENABLED, DISABLED, COUNT };

	void set_state(tstate state) {} // FIXME implement
	tstate state_;

	/** It's possible to let the engine build the contents, we need the builder in that case */
	tbuilder_grid* list_builder_;

	bool assume_fixed_row_size_;

	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "listbox"; return type; }

	void select_in_grid(tgrid* grid, const bool select);

	unsigned selected_row_;
};

} // namespace gui2

#endif


