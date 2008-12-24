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

#ifndef NEW_DRAW
#include "tstring.hpp"
#include "gui/widgets/vertical_scrollbar_container.hpp"
#else
#include "gui/widgets/generator.hpp"
#include "gui/widgets/scrollbar_container.hpp"
#endif

namespace gui2 {

#ifndef NEW_DRAW

class tspacer;

/**
 * @todo list
 * - Header row + footer row same width as client data.
 * - Cell or row select.
 * - Sort at some way.
 * - Test whether footers work properly.
 * - More testing with listboxes with their own background.
 * - client rect is also untested.
 *
 * Maybe create two types 1 fixed size and one with a builder to add new rows.
 */

/** The listbox class. */
class tlistbox : public tvertical_scrollbar_container_
{
public:
	
	tlistbox();

	/** 
	 * When an item in the list is selected by the user we need to
	 * update the state. We installed a callback handler which 
	 * calls us.
	 */
	void list_item_selected(twidget* caller);

	/***** ***** ***** row handling ****** *****/

	/**
	 * Adds a single row to the grid.
	 *
	 * This function expects a row to have only one widget or all widgets need
	 * the same settings.
	 *
	 * @param item                The data to send to set_members of the
	 *                            widget or to all the widgets.
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
	void add_row(const std::map<std::string /* widget id */, string_map>& data);

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
	void add_rows(const std::vector<string_map>& data);

	unsigned get_item_count() const { return rows_.size(); }

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

	/***** ***** ***** inherited ****** *****/

	/** Inherited from tevent_executor. */
	void mouse_left_button_down(tevent_handler& event);

	/** Inherited from tcontainer_. */
	void set_self_active(const bool active) 
		{ state_ = active ? ENABLED : DISABLED; }

	/** Inherited from tvertical_scrollbar_container_. */
	bool select_row(const unsigned row, const bool select = true);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	bool get_active() const { return state_ != DISABLED; }

	unsigned get_state() const { return state_; }

	void set_list_builder(tbuilder_grid_ptr list_builder) 
		{ list_builder_ = list_builder; }

	unsigned get_selected_row() const { return selected_row_; }

	void set_assume_fixed_row_size(bool assume = true) 
		{ assume_fixed_row_size_ = assume; }

private:

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate { ENABLED, DISABLED, COUNT };

//  It's not needed for now so keep it disabled, no definition exists yet.
//	void set_state(const tstate state);

	/** 
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;

	/**
	 * It's possible to let the engine build the contents, we need the builder
	 * in that case 
	 */
	tbuilder_grid_ptr list_builder_;

	/**
	 * Does every row in the listbox have the same height?
	 *
	 * Normally every row does have the same size but it's no requirement so we
	 * can have a listbox with rows with different height. If all the same
	 * height the units for the scrollbar are rows, otherwise they are pixels.
	 */
	bool assume_fixed_row_size_;

	/**
	 * Helper for list_item_selected().
	 *
	 * Tries to sets the selected state for the row, but only if it contains the
	 * wanted widget. NOTE this function assumes the event was triggered by the
	 * user and calls the callback handler.
	 *
	 * @param row                    The row to test.
	 * @param caller                 The widget to look for.
	 *
	 * @returns                      True if widget was found false otherwise.
	 *                               NOTE this doesn't mean the row select status
	 *                               has been changed.
	 */
	bool list_row_selected(const size_t row, twidget* caller) ;

	/** The (lastly) selected row. */
	unsigned selected_row_;

	/** Number of items selected. */
	unsigned selection_count_;

	/** Select per cell or an entire row. */
	bool row_select_;

	/** At least 1 item must be selected. */
	bool must_select_;

	/** Multiple items can be selected. */
	bool multi_select_; 

	/** The background of the list, needed for redrawing. */
	surface list_background_;

	/**
	 * The content grid of a list might contain another grid name _list. This
	 * grid must exist if there is a header or footer. This grid marks the
	 * space for the real scrollable area.
	 */

	/**
	 * @todo evaluate whether the value of the grid needs to be cached as well
	 * as it's size. It would be save since we get notified about a resize.
	 */

	/**
	 * Returns the list area.
	 *
	 * If the listbox has no _list grid the _content_grid grid will be returned
	 * instead.
	 *
	 * @param must_exist          If true the grid must exist and the
	 *                            function will fail if that's not the case. If
	 *                            true the pointer returned is always valid.
	 *
	 * @returns                   A pointer to the grid or NULL.
	 */
	tgrid* find_list(const bool must_exist = true);

	/** The const version. */
	const tgrid* find_list(const bool must_exist = true) const;
#ifndef NEW_DRAW	
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
#endif
	/**
	 * Returns the row at the wanted vertical offset.
	 *
	 * @param offset              The offset to look at, the offset in pixels
	 *                            currently on the screen, it will adjust for
	 *                            the scrollbar itself.
	 * @param offset_in_widget    Returns the vertical offset the offset is in
	 *                            the found widget.
	 *
	 * @returns                   The row number in which the widget was found.
	 * @retval -1                 If the offset wasn't found.
	 */
	size_t row_at_offset(int offset, int& offset_in_widget) const;

	/** Inherited. */
	bool get_item_active(const unsigned item) const;

	/** Contains the info for a row in the listbox. */
	class trow {

	public:

		trow(const tbuilder_grid& list_builder_, 
			const std::map<std::string /* widget id */, string_map>& data);

		/***** ***** ***** setters / getters for members ***** ****** *****/

		tgrid* grid() { return grid_; }
		const tgrid* grid() const { return grid_; }

		void set_height(const unsigned height) { height_ = height; }
		unsigned get_height() const { return height_; }

		const surface& canvas() const { return canvas_; }
		surface& canvas() { return canvas_; }

		void set_selected(const bool selected = true);
		bool get_selected() const { return selected_; }

		void  set_active(const bool active) { active_ = active; }
		bool get_active() const { return active_; }
	private:

		/** The grid containing the widgets in the row. */
		tgrid* grid_;

		/** The height of the row. */
		unsigned height_;

		/** Canvas to draw a row in the widget on. */
		surface canvas_;

		/** Is the row currently selected or not. */
		bool selected_;

		/** 
		 * Is the row active or not?
		 *
		 * This value is used to store the status setting it doesn't change the
		 * status, so the function that changes the active status should also
		 * update this value.
		 */
		bool active_;

		/** Initializes all widgets in the grid. */
		void init_in_grid(tgrid* grid, 
			const std::map<std::string /* widget id */, string_map>& data);

		/** 
		 * Selects all widgets in the grid.
		 *
		 * Some widgets are tselectable_ and thus can be selected for those
		 * widgets the selected value is modified.
		 */
		void select_in_grid(tgrid* grid, const bool selected);
	};

	/** The rows in the listbox. */
	std::vector<trow> rows_;

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** Inherited from tvertical_scrollbar_container_. */
	tpoint content_calculate_best_size() const;

	/** Inherited from tvertical_scrollbar_container_. */
	void content_use_vertical_scrollbar(const unsigned maximum_height);

	/** Inherited from tvertical_scrollbar_container_. */
	void content_set_size(const SDL_Rect& rect);

	/***** ***** ***** inherited ****** *****/

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const 
		{ static const std::string type = "listbox"; return type; }

#ifndef NEW_DRAW
	/** Inherited from tvertical_scrollbar_container_. */
	void draw_content(surface& surface,  const bool force = false,
	        const bool invalidate_background = false);
#else
	/** Inherited from tcontainer_. */
	void content_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack);
#endif	
	/** Inherited from tvertical_scrollbar_container_. */
	twidget* content_find_widget(
		const tpoint& coordinate, const bool must_be_active);

	/** Inherited from tvertical_scrollbar_container_. */
	const twidget* content_find_widget(const tpoint& coordinate, 
			const bool must_be_active) const;
};

#else

/** The listbox class. */
class tlistbox
		: public tscrollbar_container
{
	friend class tbuilder_listbox;
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
	twidget* find_widget(
			const tpoint& coordinate, const bool must_be_active);

	/** Inherited from tscrollbar_container. */
	const twidget* find_widget(
			const tpoint& coordinate, const bool must_be_active) const;

	/** Import overloaded versions. */
	using tscrollbar_container::find_widget;
	
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

#endif

} // namespace gui2

#endif

