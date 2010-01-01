/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_MENUBAR_HPP_INCLUDED
#define GUI_WIDGETS_MENUBAR_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2 {

class tselectable_;
namespace implementation {
	struct tbuilder_menubar;
}
/**
 * A menu bar.
 *
 * A menu bar is a generic component which can hold mutiple toggle items of which
 * zero or one are selected. Whether zero is allowed depends on the selection mode.
 * The elements can be order horizontally or vertically which is set in WML.
 */
class tmenubar : public tcontainer_
{
	friend struct implementation::tbuilder_menubar;
public:
	/** The direction is which the items are next to eachother. */
	enum tdirection { HORIZONTAL, VERTICAL };

	tmenubar(const tdirection direction) :
		tcontainer_(COUNT),
		state_(ENABLED),
		callback_selection_change_(0),
		must_select_(false),
		selected_item_(-1),
		direction_(direction)
	{
	}

	/** Returns the number of items in the menu. */
	size_t get_item_count() const;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from tcontrol. */
	bool get_active() const { return state_ != DISABLED; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return state_; }

	/**
	 * Update our state when a widget selected or deselected an item.
	 *
	 * @param widget              The widget that changed its state by a user
	 *                            action.
	 */
	void item_selected(twidget* widget) ;

	/***** ***** ***** setters / getters for members ***** ****** *****/
	void set_callback_selection_change(void (*callback) (twidget*))
		{ callback_selection_change_ = callback; }

	void set_must_select(const bool must_select);

	void set_selected_item(const int item);
	int get_selected_item() const { return selected_item_; }

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate { ENABLED, DISABLED, COUNT };

	void set_state(const tstate state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;

 	/** This callback is used when the selection is changed by the user. */
	void (*callback_selection_change_) (twidget*);

	/** Do we always need to select an item? */
 	bool must_select_;

	/** The selected item -1 for none. */
	int selected_item_;

	/** The direction of the menu bar. */
	tdirection direction_;

	/** Returns an item. */
	const tselectable_* operator[](const size_t index) const;
	tselectable_* operator[](const size_t index);

	/** The builder needs to call us so we can wire in the proper callbacks. */
	void finalize_setup();

	/** Inherited from tcontainer_. */
	void set_self_active(const bool active)
		{ state_ = active ? ENABLED : DISABLED; }

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;
};

} // namespace gui2

#endif

