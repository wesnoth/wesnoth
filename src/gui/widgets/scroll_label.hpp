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

#ifndef GUI_WIDGETS_SCROLL_LABEL_HPP_INCLUDED
#define GUI_WIDGETS_SCROLL_LABEL_HPP_INCLUDED

#include "gui/widgets/scrollbar_container.hpp"

namespace gui2 {

class tlabel;
class tspacer;

namespace implementation {
	struct tbuilder_scroll_label;
}

/**
 * Label showing a text.
 *
 * This version shows a scrollbar if the text gets too long and has some
 * scrolling features. In general this widget is slower as the normal label so
 * the normal label should be preferred.
 */
class tscroll_label : public tscrollbar_container
{
	friend struct implementation::tbuilder_scroll_label;
public:

	tscroll_label();

	/** Inherited from tcontrol. */
	void set_label(const t_string& label);

	/** Inherited from tcontrol. */
	void set_use_markup(bool use_markup);

	/** Inherited from tcontainer_. */
	void set_self_active(const bool active)
		{ state_ = active ? ENABLED : DISABLED; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	bool get_active() const { return state_ != DISABLED; }

	unsigned get_state() const { return state_; }

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

	void finalize_subclass();

	/***** ***** ***** inherited ****** *****/

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_left_button_down(const event::tevent event);
};

} // namespace gui2

#endif

