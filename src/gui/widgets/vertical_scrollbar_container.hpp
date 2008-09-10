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

#ifndef GUI_WIDGETS_VERTICAL_SCROLLBAR_CONTAINER_HPP_INCLUDED
#define GUI_WIDGETS_VERTICAL_SCROLLBAR_CONTAINER_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2 {

class tscrollbar_;



/** Base class for creating containers with a vertical scrollbar. */
class tvertical_scrollbar_container_ : public tcontainer_
{
	// Builders need to be able to finalize the object.
	friend class tbuilder_listbox;


	// Callbacks can call update rountines. Note these are not further declared
	// here only need extrnal linkage to be friends.
	friend void callback_scrollbar_button(twidget*);
	friend void callback_scrollbar(twidget*);
public:
	
	tvertical_scrollbar_container_(const unsigned canvas_count) 
		: tcontainer_(canvas_count),
		callback_value_change_(NULL)
	{
	}

	/***** ***** ***** inherited ****** *****/

	/** Inherited from tevent_executor. */
	void key_press(tevent_handler& event, bool& handled, 
		SDLKey key, SDLMod modifier, Uint16 unicode);

	/** Inherited from twidget. */
	bool has_vertical_scrollbar() const { return true; }

	/** 
	 * Selects an entire row. 
	 *
	 * @param row                 The row to (de)select.
	 * @param select              true select, false deselect.
	 *
	 * @returns                   false if deselecting wasn't allowed.
	 *                            true otherwise.
	 */
	virtual bool select_row(const unsigned row, const bool select = true) = 0;

	/***** ***** ***** setters / getters for members ***** ****** *****/
	void set_callback_value_change(void (*callback) (twidget* caller))
		{ callback_value_change_ = callback; }

protected:

	/**
	 * When the value of the selected item has been changed this function
	 * should be called.
	 */
	void value_changed();


	/** 
	 * Returns the scroll widget.
	 *
	 * This always returns the wdiget, regardless of the mode.
	 * 
	 * @param must_exist          If true the widget must exist and the
	 *                            function will fail if that's not the case. If
	 *                            true the pointer returned is always valid.
	 *
	 * @returns                   A pointer to the widget or NULL.
	 */
    tscrollbar_* find_scrollbar(const bool must_exist = true);

	/** The const version. */
    const tscrollbar_* find_scrollbar(const bool must_exist = true) const;

	/** 
	 * Sets the status of the scrollbar buttons.
	 *
	 * This is needed after the scrollbar moves so the status of the buttons
	 * will be active or inactive as needed.
	 */
	void set_scrollbar_button_status();

private:

	/**
	 * This callback is used when the selection is changed due to a user event.
	 * The name is not fully appropriate for the event but it's choosen to be
	 * generic.
	 */
	void (*callback_value_change_) (twidget* caller);

	/** The builder needs to call us so we can write in the proper callbacks. */
	void finalize_setup();

	/** Callback when the scrollbar moves. */
	void scrollbar_moved(twidget* /*caller*/)
		{ set_scrollbar_button_status(); set_dirty(); }
	/** 
	 * When an item scrollbar control button is clicked we need to move the
	 * scrollbar and update the list. 
	 */
	void scrollbar_click(twidget* caller);

	/**
	 * Is the wanted item active?
	 *
	 * Some subclasses might have items that can be inactive, those shouldn't
	 * be selected. So add a test here. 
	 *
	 * @param item                The item to check.
	 *
	 * @returns                   True active, false inactive.
	 */
	virtual bool get_item_active(const unsigned /*item*/) const { return true; }


	/** Returns the selected row. */
	virtual unsigned get_selected_row() const;

};

} // namespace gui2

#endif


