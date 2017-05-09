/*
   Copyright (C) 2010 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/styled_widget.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

class progress_bar : public styled_widget
{
public:
	progress_bar() : styled_widget(COUNT), percentage_(static_cast<unsigned>(-1))
	{
		// Force canvas update
		set_percentage(0);
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;


	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_percentage(unsigned percentage);
	unsigned get_percentage() const
	{
		return percentage_;
	}

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		COUNT
	};

	/** The percentage done. */
	unsigned percentage_;

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct progress_bar_definition : public styled_widget_definition
{
	explicit progress_bar_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_progress_bar : public builder_styled_widget
{

	explicit builder_progress_bar(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
