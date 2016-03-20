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

#ifndef GUI_WIDGETS_TOGGLE_PANEL_HPP_INCLUDED
#define GUI_WIDGETS_TOGGLE_PANEL_HPP_INCLUDED

#include "gui/widgets/panel.hpp"
#include "gui/widgets/selectable.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * Class for a toggle button.
 *
 * Quite some code looks like ttoggle_button maybe we should inherit from that
 * but let's test first.  the problem is that the toggle_button has an icon we
 * don't want, but maybe look at refactoring later.  but maybe we should also
 * ditch the icon, not sure however since it's handy for checkboxes...
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
	 * @param data                Map with the key value pairs to set the
	 *                            members.
	 */
	void set_child_members(
			const std::map<std::string /* widget id */, string_map>& data);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) OVERRIDE;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/**
	 * See @ref tcontainer_::get_client_rect.
	 *
	 * @todo only due to the fact our definition is slightly different from
	 * tpanel_definition we need to override this function and do about the
	 * same, look at a way to 'fix' that.
	 */
	virtual SDL_Rect get_client_rect() const OVERRIDE;

	/**
	 * See @ref tcontainer_::border_space.
	 *
	 * @todo only due to the fact our definition is slightly different from
	 * tpanel_definition we need to override this function and do about the
	 * same, look at a way to 'fix' that.
	 */
	virtual tpoint border_space() const OVERRIDE;

	/** Inherited from tselectable_ */
	unsigned get_value() const OVERRIDE
	{
		return state_num_;;
	}

	/** Inherited from tselectable_ */
	void set_value(const unsigned selected);

	/** Inherited from tselectable_ */
	unsigned num_states() const OVERRIDE;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_retval(const int retval);

	/** Inherited from tselectable_. */
	void set_callback_state_change(boost::function<void(twidget&)> callback)
	{
		callback_state_change_ = callback;
	}

	void set_callback_mouse_left_double_click(
			boost::function<void(twidget&)> callback)
	{
		callback_mouse_left_double_click_ = callback;
	}

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
		ENABLED,
		DISABLED,
		FOCUSED,
		COUNT
	};

	void set_state(const tstate state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;

	/**
	 *	Usually 1 for selected and 0 for not selected, can also have higher values in tristate buttons.
	 */
	unsigned state_num_;

	/**
	 * The return value of the button.
	 *
	 * If this value is not 0 and the button is double clicked it sets the
	 * retval of the window and the window closes itself.
	 */
	int retval_;

	/** See tselectable_::set_callback_state_change. */
	boost::function<void(twidget&)> callback_state_change_;

	/** Mouse left double click callback */
	boost::function<void(twidget&)> callback_mouse_left_double_click_;

	/** See @ref twidget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer,
									  int x_offset,
									  int y_offset) OVERRIDE;

	/** See @ref twidget::impl_draw_foreground. */
	virtual void impl_draw_foreground(surface& frame_buffer,
									  int x_offset,
									  int y_offset) OVERRIDE;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::tevent event, bool& handled);

	void signal_handler_mouse_leave(const event::tevent event, bool& handled);

	void signal_handler_pre_left_button_click(const event::tevent event);

	void signal_handler_left_button_click(const event::tevent event,
										  bool& handled);

	void signal_handler_left_button_double_click(const event::tevent event,
												 bool& handled);
};

// }---------- DEFINITION ---------{

struct ttoggle_panel_definition : public tcontrol_definition
{
	explicit ttoggle_panel_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		unsigned top_border;
		unsigned bottom_border;

		unsigned left_border;
		unsigned right_border;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_toggle_panel : public tbuilder_control
{
	explicit tbuilder_toggle_panel(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

	tbuilder_grid_ptr grid;

private:
	std::string retval_id_;
	int retval_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
