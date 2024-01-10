/*
	Copyright (C) 2008 - 2023
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

#include "gui/widgets/container_base.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

#include "gui/widgets/text_box.hpp"

#include <iostream>

namespace gui2
{

// ------------ WIDGET -----------{

class label;
class spacer;

namespace implementation
{
struct builder_spinner;
}

/**
 * @ingroup GUIWidgetWML
 *
 * Spinner widget.
 *
 * This version shows a scrollbar if the text gets too long and has some scrolling features.
 * In general this widget is slower as the normal label so the normal label should be preferred.
 *
 * Key          |Type                        |Default  |Description
 * -------------|----------------------------|---------|-----------
 * grid         | @ref guivartype_grid "grid"|mandatory|A grid containing the widgets for main widget.
 *
 * TODO: we need one definition for a vertical scrollbar since this is the second time we use it.
 *
 * ID (return value)|Type                        |Default  |Description
 * -----------------|----------------------------|---------|-----------
 * _content_grid    | @ref guivartype_grid "grid"|mandatory|A grid which should only contain one label widget.
 * The following states exist:
 * * state_enabled - the scroll label is enabled.
 * * state_disabled - the scroll label is disabled.
 */
class spinner : public container_base
{
	friend struct implementation::builder_spinner;

public:
	explicit spinner(const implementation::builder_spinner& builder);

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	bool can_wrap() const override;

	void set_value(const int val);

	int get_value();

	void prev()
	{
		// Allow negatives?
		if (get_value() > 0) {
			set_value(get_value() - step_size_);
		}
	}
//
//	void next()
//	{
//		// No max value
//		set_value(get_value() + step_size_);
//	}

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

//	void finalize_subclass() override;
	/** The grid that holds the content. */
	std::unique_ptr<grid> content_grid_;


	int step_size_;

	text_box* get_internal_text_box();

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/***** ***** ***** inherited ****** *****/

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_left_button_down(const event::ui_event event);
};

// }---------- DEFINITION ---------{

struct spinner_definition : public styled_widget_definition
{
	explicit spinner_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_spinner : public builder_styled_widget
{
	explicit builder_spinner(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
