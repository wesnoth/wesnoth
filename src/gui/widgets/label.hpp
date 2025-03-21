/*
	Copyright (C) 2008 - 2025
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/styled_widget.hpp"

#include "gui/core/widget_definition.hpp"

namespace gui2
{
namespace implementation
{
	struct builder_label;
}

// ------------ WIDGET -----------{

class label : public styled_widget
{
	friend struct implementation::builder_label;

public:
	explicit label(const implementation::builder_label& builder);

	/** See @ref widget::can_wrap. */
	virtual bool can_wrap() const override
	{
		return can_wrap_ || characters_per_line_ != 0;
	}

	/** See @ref styled_widget::get_characters_per_line. */
	virtual unsigned get_characters_per_line() const override
	{
		return characters_per_line_;
	}

	/** See @ref styled_widget::get_link_aware. */
	virtual bool get_link_aware() const override
	{
		return link_aware_;
	}

	/** See @ref styled_widget::get_link_aware. */
	virtual color_t get_link_color() const override
	{
		return link_color_;
	}

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override
	{
		return state_ != DISABLED;
	}

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override
	{
		return state_;
	}

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override
	{
		return false;
	}

	/** See @ref widget::can_mouse_focus. */
	virtual bool can_mouse_focus() const override
	{
		return !tooltip().empty() || get_link_aware();
	}

	/** See @ref styled_widget::update_canvas. */
	virtual void update_canvas() override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_can_wrap(const bool wrap)
	{
		can_wrap_ = wrap;
	}

	void set_characters_per_line(const unsigned characters_per_line)
	{
		characters_per_line_ = characters_per_line;
	}

	void set_link_aware(bool l);

	void set_link_color(const color_t& color);

	void set_can_shrink(bool can_shrink)
	{
		can_shrink_ = can_shrink;
	}

	void set_text_alpha(unsigned short alpha);

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
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
	color_t link_color_;

	bool can_shrink_;

	unsigned short text_alpha_;

	/** Inherited from styled_widget. */
	virtual bool text_can_shrink() override
	{
		return can_shrink_;
	}

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	/**
	 * Left click signal handler: checks if we clicked on a hyperlink
	 */
	void signal_handler_left_button_click(bool& handled);

	/**
	 * Right click signal handler: checks if we clicked on a hyperlink, copied to clipboard
	 */
	void signal_handler_right_button_click(bool& handled);

	/**
	 * Mouse motion signal handler: checks if the cursor is on a hyperlink
	 */
	void signal_handler_mouse_motion(bool& handled, const point& coordinate);

	/**
	 * Mouse leave signal handler: checks if the cursor left a hyperlink
	 */
	void signal_handler_mouse_leave(bool& handled);

	/**
	 * Implementation detail for (re)setting the hyperlink cursor.
	 */
	void update_mouse_cursor(bool enable);
};

// }---------- DEFINITION ---------{

struct label_definition : public styled_widget_definition
{

	explicit label_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		color_t link_color;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_label : public builder_styled_widget
{
	builder_label(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	bool wrap;

	unsigned characters_per_line;

	PangoAlignment text_alignment;

	bool can_shrink;
	bool link_aware;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
