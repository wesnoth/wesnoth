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

#ifndef GUI_WIDGETS_LABEL_HPP_INCLUDED
#define GUI_WIDGETS_LABEL_HPP_INCLUDED

#include "gui/widgets/control.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/** Label showing a text. */
class label : public control
{
public:
	label();

	/** See @ref widget::can_wrap. */
	virtual bool can_wrap() const override;

	/** See @ref control::get_characters_per_line. */
	virtual unsigned get_characters_per_line() const override;

	/** See @ref control::get_link_aware. */
	virtual bool get_link_aware() const override;

	/** See @ref control::get_link_aware. */
	virtual std::string get_link_color() const override;

	/** See @ref control::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref control::get_active. */
	virtual bool get_active() const override;

	/** See @ref control::get_state. */
	virtual unsigned get_state() const override;

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_can_wrap(const bool wrap)
	{
		can_wrap_ = wrap;
	}

	void set_characters_per_line(const unsigned set_characters_per_line);

	void set_link_aware(bool l);

	void set_link_color(const std::string & color);

	virtual bool can_mouse_focus() const override { return !tooltip().empty(); }
private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
		COUNT
	};

	void set_state(const state_t state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	/** Holds the label can wrap or not. */
	bool can_wrap_;

	/**
	 * The maximum number of characters per line.
	 *
	 * The maximum is not an exact maximum, it uses the average character width.
	 */
	unsigned characters_per_line_;

	/**
	 * Whether the label is link aware, rendering links with special formatting
	 * and handling click events.
	 */
	bool link_aware_;

	/**
	 * What color links will be rendered in.
	 */
	std::string link_color_;

	/** See @ref control::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/** Inherited from control. */
	void load_config_extra() override;

	/***** ***** ***** signal handlers ***** ****** *****/

	/**
	 * Left click signal handler: checks if we clicked on a hyperlink
	 */
	void signal_handler_left_button_click(const event::event_t event, bool & handled);

	/**
	 * Right click signal handler: checks if we clicked on a hyperlink, copied to clipboard
	 */
	void signal_handler_right_button_click(const event::event_t event, bool & handled);
};

// }---------- DEFINITION ---------{

struct label_definition : public control_definition
{

	explicit label_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		bool link_aware;
		std::string link_color;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_label : public builder_control
{
	builder_label(const config& cfg);

	using builder_control::build;

	widget* build() const;

	bool wrap;

	unsigned characters_per_line;

	PangoAlignment text_alignment;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
