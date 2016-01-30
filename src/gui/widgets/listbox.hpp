/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_LISTBOX_HPP_INCLUDED
#define GUI_WIDGETS_LISTBOX_HPP_INCLUDED

#ifndef GUI2_EXPERIMENTAL_LISTBOX

#include "gui/widgets/generator.hpp"
#include "gui/widgets/scrollbar_container.hpp"

namespace gui2
{
class tselectable_;
namespace implementation
{
struct tbuilder_listbox;
struct tbuilder_horizontal_listbox;
}

/** The listbox class. */
class tlistbox : public tscrollbar_container
{
	friend struct implementation::tbuilder_listbox;
	friend struct implementation::tbuilder_horizontal_listbox;
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
	tlistbox(const bool has_minimum,
			 const bool has_maximum,
			 const tgenerator_::tplacement placement,
			 const bool select);

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
	void set_row_shown(const std::vector<bool>& shown);

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
	 * The possibly-giving-problems nonconst version of get_row_grid
	 *
	 * @param row                 The row to get the grid from, the caller has
	 *                            to make sure the row is a valid row.
	 * @returns                   The grid of the wanted row.
	 */
	tgrid* get_row_grid(const unsigned row);

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
	void list_item_clicked(twidget& caller);

	/** See @ref tcontainer_::set_self_active. */
	virtual void set_self_active(const bool active) OVERRIDE;

	/**
	 * Request to update the size of the content after changing the content.
	 *
	 * When a resize is required the container first can try to handle it
	 * itself. If it can't honor the request the function will call @ref
	 * twindow::invalidate_layout().
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

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	/** See @ref twidget::layout_children. */
	virtual void layout_children() OVERRIDE;

	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) OVERRIDE;

	/***** ***** ***** setters / getters for members ***** ****** *****/
	void
	set_callback_item_change(const boost::function<void(size_t)>& callback)
	{
		callback_item_changed_ = callback;
	}

	void
	set_callback_value_change(const boost::function<void(twidget&)>& callback)
	{
		callback_value_changed_ = callback;
	}

	void set_list_builder(tbuilder_grid_ptr list_builder)
	{
		list_builder_ = list_builder;
	}

	void order_by(const tgenerator_::torder_func& func);

	void set_column_order(unsigned col, const std::vector<tgenerator_::torder_func>& func);
protected:
	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from tscrollbar_container. */
	void handle_key_up_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tscrollbar_container. */
	void handle_key_down_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tscrollbar_container. */
	void handle_key_left_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tscrollbar_container. */
	void handle_key_right_arrow(SDLMod modifier, bool& handled);

private:
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
	void finalize(tbuilder_grid_const_ptr header,
				  tbuilder_grid_const_ptr footer,
				  const std::vector<string_map>& list_data);
	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this class, it's stored in the content_grid_
	 * of the tscrollbar_container super class and freed when it's grid is
	 * freed.
	 */
	tgenerator_* generator_;

	/** Contains the builder for the new items. */
	tbuilder_grid_const_ptr list_builder_;

	/**
	 * This callback is called when a list item is clicked (toggled).
	 *
	 * The function is passed the index of the toggled item.
	 */
	boost::function<void(size_t)> callback_item_changed_;

	/**
	 * This callback is called when the value in the listbox changes.
	 *
	 * @todo the implementation of the callback hasn't been tested a lot and
	 * there might be too many calls. That might happen if an arrow up didn't
	 * change the selected item.
	 */
	boost::function<void(twidget&)> callback_value_changed_;

	bool need_layout_;

	typedef std::vector<std::pair<tselectable_*, std::vector<tgenerator_::torder_func> > > torder_list;
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
	void resize_content(const twidget& row);

	/** Layouts the children if needed. */
	void layout_children(const bool force);

	/** Inherited from tscrollbar_container. */
	virtual void set_content_size(const tpoint& origin, const tpoint& size);

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;

	void order_by_column(unsigned column, twidget& widget);
};

} // namespace gui2

#endif
#endif
