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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/progress_bar.hpp"

#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(progress_bar)

void progress_bar::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool progress_bar::get_active() const
{
	return true;
}

unsigned progress_bar::get_state() const
{
	return ENABLED;
}

void progress_bar::set_percentage(unsigned percentage)
{
	percentage = std::min<unsigned>(percentage, 100);

	if(percentage_ != percentage) {
		percentage_ = percentage;

		for(auto & c : get_canvases())
		{
			c.set_variable("percentage", wfl::variant(percentage));
		}

		set_is_dirty(true);
	}
}

bool progress_bar::disable_click_dismiss() const
{
	return false;
}

const std::string& progress_bar::get_control_type() const
{
	static const std::string type = "progress_bar";
	return type;
}

// }---------- DEFINITION ---------{

progress_bar_definition::progress_bar_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing progress bar " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_progress_bar
 *
 * == Progress bar ==
 *
 * @macro = progress_bar_description
 *
 * The definition of a progress bar. This object shows the progress of a certain
 * action, or the value state of a certain item.
 *
 * The following states exist:
 * * state_enabled, the progress bar is enabled.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="progress_bar_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="progress_bar_definition"}
 * @end{parent}{name="gui/"}
 */
progress_bar_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in progress_bar.hpp.
	state.emplace_back(cfg.child("state_enabled"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{progress_bar_description}
 * A progress bar shows the progress of a certain object.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_progress_bar
 *
 * == Image ==
 *
 * @macro = progress_bar_description
 *
 * A progress bar has no extra fields.
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="progress_bar"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @end{tag}{name="progress_bar"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_progress_bar::builder_progress_bar(const config& cfg)
	: builder_styled_widget(cfg)
{
}

widget* builder_progress_bar::build() const
{
	progress_bar* widget = new progress_bar();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed progress bar '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
