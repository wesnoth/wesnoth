/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/scrollbar_container.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

class label;
class spacer;

namespace implementation
{
struct builder_scroll_label;
}

/**
 * Label showing a text.
 *
 * This version shows a scrollbar if the text gets too long and has some
 * scrolling features. In general this widget is slower as the normal label so
 * the normal label should be preferred.
 */
class scroll_label : public scrollbar_container
{
	friend struct implementation::builder_scroll_label;

public:
	scroll_label(bool wrap, const PangoAlignment text_alignment);

	/** See @ref styled_widget::set_label. */
	virtual void set_label(const t_string& label) override;

	/** See @ref styled_widget::set_text_alignment. */
	virtual void set_text_alignment(const PangoAlignment text_alignment) override;

	/** See @ref styled_widget::set_use_markup. */
	virtual void set_use_markup(bool use_markup) override;

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	bool can_wrap() const override;
	void set_can_wrap(bool can_wrap);

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

	// It's not needed for now so keep it disabled, no definition exists yet.
	// void set_state(const state_t state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	bool wrap_on_;

	PangoAlignment text_alignment_;

	void finalize_subclass() override;

	label* get_internal_label();

	/***** ***** ***** inherited ****** *****/

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_left_button_down(const event::ui_event event);
};

// }---------- DEFINITION ---------{

struct scroll_label_definition : public styled_widget_definition
{
	explicit scroll_label_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_scroll_label : public builder_styled_widget
{
	explicit builder_scroll_label(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

	scrollbar_container::scrollbar_mode vertical_scrollbar_mode;
	scrollbar_container::scrollbar_mode horizontal_scrollbar_mode;
	bool wrap_on;
	const PangoAlignment text_alignment;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
