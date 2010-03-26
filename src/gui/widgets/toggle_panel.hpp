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

#ifndef GUI_WIDGETS_TOGGLE_PANEL_HPP_INCLUDED
#define GUI_WIDGETS_TOGGLE_PANEL_HPP_INCLUDED

#include "gui/widgets/panel.hpp"
#include "gui/widgets/selectable.hpp"

namespace gui2 {

/**
 * Class for a toggle button.
 *
 * Quite some code looks like ttoggle_button maybe we should inherit from that but let's test first.
 * the problem is that the toggle_button has an icon we don't want, but maybe look at refactoring later.
 * but maybe we should also ditch the icon, not sure however since it's handy for checkboxes...
 */
class ttoggle_panel : public tpanel, public tselectable_
{
public:
	ttoggle_panel();

	/**
	 * Sets the members of the child controls.
	 *
	 * Sets the members for all controls which have the proper member id. See
	 * tcontrol::set_members for more info.
	 *
	 * @param data                Map with the key value pairs to set the members.
	 */
	void set_child_members(const std::map<std::string /* widget id */, string_map>& data);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from tcontainer_ */
	twidget* find_at(const tpoint& coordinate, const bool must_be_active)
	{
		/**
		 * @todo since there is no mouse event nesting (or event nesting at all)
		 * we need to capture all events. This means items on the panel will
		 * never receive an event, which gives problems with for example the
		 * intended button on the addon panel. So we need to chain mouse events
		 * as well and also add a handled flag for them.
		 */

		twidget* result = tcontainer_::find_at(coordinate, must_be_active);
		return result ? result : tcontrol::find_at(coordinate, must_be_active);
	}

	/** Inherited from tcontainer_ */
	const twidget* find_at(
			const tpoint& coordinate, const bool must_be_active) const
	{
		const twidget* result =
				tcontainer_::find_at(coordinate, must_be_active);
		return result ? result : tcontrol::find_at(coordinate, must_be_active);
	}

	/** Inherited from tpanel. */
	void set_active(const bool active);

	/** Inherited from tpanel. */
	bool get_active() const
		{ return state_ != DISABLED && state_ != DISABLED_SELECTED; }

	/** Inherited from tpanel. */
	unsigned get_state() const { return state_; }

	/**
	 * Inherited from tpanel.
	 *
	 * @todo only due to the fact our definition is slightly different from
	 * tpanel_defintion we need to override this function and do about the same,
	 * look at a way to 'fix' that.
	 */
	SDL_Rect get_client_rect() const;

	/**
	 * Inherited from tpanel.
	 *
	 * @todo only due to the fact our definition is slightly different from
	 * tpanel_defintion we need to override this function and do about the same,
	 * look at a way to 'fix' that.
	 */
	tpoint border_space() const;

	/** Inherited from tselectable_ */
	bool get_value() const { return state_ >= ENABLED_SELECTED; }

	/** Inherited from tselectable_ */
	void set_value(const bool selected);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_retval(const int retval);

	/** Inherited from tselectable_. */
	void set_callback_state_change(boost::function<void (twidget*)> callback)
		{ callback_state_change_ = callback; }

	void set_callback_mouse_left_double_click(boost::function<void (twidget*)> callback)
		{ callback_mouse_left_double_click_ = callback; }
private:

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 * Also note the internals do assume the order for 'up' and 'down' to be the
	 * same and also that 'up' is before 'down'. 'up' has no suffix, 'down' has
	 * the SELECTED suffix.
	 */
	enum tstate {
		ENABLED,          DISABLED,          FOCUSSED,
		ENABLED_SELECTED, DISABLED_SELECTED, FOCUSSED_SELECTED,
		COUNT};

	void set_state(const tstate state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;

	/**
	 * The return value of the button.
	 *
	 * If this value is not 0 and the button is double clicked it sets the
	 * retval of the window and the window closes itself.
	 */
	int retval_;

	/** See tselectable_::set_callback_state_change. */
	boost::function<void (twidget*)> callback_state_change_;

	/** Mouse left double click callback */
	boost::function<void (twidget*)> callback_mouse_left_double_click_;

	/** Inherited from tpanel. */
	void impl_draw_background(surface& frame_buffer)
	{
		// We don't have a fore and background and need to draw depending on
		// our state, like a control. So we use the controls drawing method.
		tcontrol::impl_draw_background(frame_buffer);
	}

	/** Inherited from tpanel. */
	void impl_draw_foreground(surface& frame_buffer)
	{
		// We don't have a fore and background and need to draw depending on
		// our state, like a control. So we use the controls drawing method.
		tcontrol::impl_draw_foreground(frame_buffer);
	}


	/** Inherited from tpanel. */
	const std::string& get_control_type() const;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::tevent event, bool& handled);

	void signal_handler_mouse_leave(const event::tevent event, bool& handled);

	void signal_handler_pre_left_button_click(const event::tevent event);

	void signal_handler_left_button_click(
			const event::tevent event, bool& handled);

	void signal_handler_left_button_double_click(
			const event::tevent event, bool& handled);
};

} // namespace gui2

#endif



