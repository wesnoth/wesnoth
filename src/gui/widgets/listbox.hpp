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

#ifndef GUI_WIDGETS_LISTBOX_HPP_INCLUDED
#define GUI_WIDGETS_LISTBOX_HPP_INCLUDED

#include "tstring.hpp"
#include "gui/widgets/container.hpp"

namespace gui2 {

class tscrollbar_;
class tspacer;

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

	// FIXME this might not the right thing to do.
	void set_active(const bool active) { set_state(active ? ENABLED : DISABLED); };
	bool get_active() const { return state_ != DISABLED; }
	unsigned get_state() const { return state_; }

	/** Inherited from tevent_executor. */
	void mouse_left_button_down(tevent_handler& event);

	/** Inherited from tevent_executor. */
	void key_press(tevent_handler& event, bool& handled, 
		SDLKey key, SDLMod modifier, Uint16 unicode);

	/** Inherited from twidget. */
	bool has_vertical_scrollbar() const { return true; }

	/** 
	 * When an item in the list is selected by the user we need to
	 * update the state. We installed a callback handler which 
	 * calls us.
	 */
	void list_item_selected(twidget* caller);

private:

	/**
	 * Helper for list_item_selected().
	 *
	 * Tries to sets the selected state for the row, but only if it contains the
	 * wanted widget.
	 *
	 * @param row                    The row to test.
	 * @param caller                 The widget to look for.
	 *
	 * @returns                      True if widget was found false otherwise.
	 *                               NOTE this doesn't mean the row select status
	 *                               has been changed.
	 */
	bool list_row_selected(const size_t row, twidget* caller) ;
public:

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

	void set_list_builder(tbuilder_grid* list_builder) 
		{ list_builder_ = list_builder; }

	void set_assume_fixed_row_size(bool assume = true) 
		{ assume_fixed_row_size_ = assume; }

	/** Inherited from tcontainer. */
	tpoint get_best_size() const;

	/** Inherited from tcontainer. */
	void draw(surface& surface,  const bool force = false,
	        const bool invalidate_background = false);
private:
	/** 
	 * Draws the list area if assume_fixed_row_size_ is true. 
	 *
	 * The parameters are the same as draw().
	 */
	void draw_list_area_fixed_row_height(surface& surface, const bool force,
		const bool invalidate_background);

	/** 
	 * Draws the list area if assume_fixed_row_size_ is false.
	 *
	 * The parameters are the same as draw().
	 */
	void draw_list_area_variable_row_height(surface& surface, const bool force,
		const bool invalidate_background);

public:
	/** Inherited from tcontainer. */
	void set_size(const SDL_Rect& rect);

	/** Inherited from tcontainer. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active);

	/** Inherited from tcontainer. */
	const twidget* find_widget(const tpoint& coordinate, 
			const bool must_be_active) const;

	/*
	 * NOTE twidget* find_widget(const std::string& id, const bool must_be_active);
	 * and it's const version are inherited from tcontainer_ but gcc isn't too 
	 * happy with that so we need to call tcontainer_::find_widget() so when
	 * it's required to override those, check that the tcontainer_:: is dropped.
	 */

	/**
	 * Adds a single row to the grid.
	 *
	 * This function expects a row to have only one widget or all widgets need
	 * the same settings.
	 *
	 * @param item                The data to send to set_members of the
	 *                            widget or to all the widgets.
	 */
	void add_row(const std::map<
		std::string /* member id */, t_string /* member value */>& item);

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
	void add_row(const std::map<std::string /* widget id */, std::map<
		std::string /* member id */, t_string /* member value */> >& data);

	/**
	 * Adds multiple rows to the grid.
	 *
	 * Small wrapper to void add_row(const std::map<std::string, t_string>&).
	 * NOTE it's _not_ a wrapper to void add_row(const std::map<std::string,
	 * std::map<std::string, t_string> >&).
	 *
	 * @param data                Vector with the number of rows, for every row
	 *                            it calls add_row(std::map<std::string,
	 *                            t_string>&). 
	 */
	void add_rows(const std::vector<std::map<
		std::string /* member id */, t_string /* member value */> >& data);

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

	void set_state(const tstate /*state*/) {} // FIXME implement
	tstate state_;

	/** It's possible to let the engine build the contents, we need the builder in that case */
	tbuilder_grid* list_builder_;

	/** Returns the scrollbar widget */
	tscrollbar_* scrollbar();

	/** Returns the scrollbar widget */
	const tscrollbar_* scrollbar() const;

	/** Returns the spacer widget which is used to reserve space of the real list. */
	tspacer* list();

	/** Returns the spacer widget which is used to reserve space of the real list. */
	const tspacer* list() const;

	/**
	 * Does every row in the listbox have the same height?
	 *
	 * Normally every row does have the same size but it's no requirement so we
	 * can have a listbox with rows with different height. If all the same
	 * height the units for the scrollbar are rows, otherwise they are pixels.
	 */
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

		trow(const tbuilder_grid& list_builder_, 
			const std::map<std::string /* widget id */, std::map<
			std::string /* member id */, t_string /* member value */> >& data);

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

		void init_in_grid(tgrid* grid, 
			const std::map<std::string /* widget id */, std::map<
			std::string /* member id */, t_string /* member value */> >& data);

		void select_in_grid(tgrid* grid, const bool sel);
	};

	std::vector<trow> rows_;

	/**
	 * Returns the row at the wanted vertical offset.
	 *
	 * @param offset              The offset to look at, the offset in pixels currently on the screen, it will adjust for the scrollbar itself.
	 * @param offset_in_widget    Returns the vertical offset the offset is in the found widget.
	 *
	 * @return                    The row number in which the widget was found.
	 * @retval -1                 If the offset wasn't found.
	 */
	size_t row_at_offset(int offset, int& offset_in_widget) const; 

	/** The builder needs to call us so we can write in the proper callbacks. */
	void finalize_setup();

};

} // namespace gui2

#endif

