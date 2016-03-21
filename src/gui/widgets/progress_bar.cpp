/*
   Copyright (C) 2010 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "utils/foreach.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(progress_bar)

void tprogress_bar::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tprogress_bar::get_active() const
{
	return true;
}

unsigned tprogress_bar::get_state() const
{
	return ENABLED;
}

void tprogress_bar::set_percentage(unsigned percentage)
{
	percentage = std::min<unsigned>(percentage, 100);

	if(percentage_ != percentage) {
		percentage_ = percentage;

		FOREACH(AUTO & c, canvas())
		{
			c.set_variable("percentage", variant(percentage));
		}

		set_is_dirty(true);
	}
}

bool tprogress_bar::disable_click_dismiss() const
{
	return false;
}

const std::string& tprogress_bar::get_control_type() const
{
	static const std::string type = "progress_bar";
	return type;
}

// }---------- DEFINITION ---------{

tprogress_bar_definition::tprogress_bar_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing progress bar " << id << '\n';

	load_resolutions<tresolution>(cfg);
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
tprogress_bar_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
{
	// Note the order should be the same as the enum tstate in progress_bar.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
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

tbuilder_progress_bar::tbuilder_progress_bar(const config& cfg)
	: tbuilder_control(cfg)
{
}

twidget* tbuilder_progress_bar::build() const
{
	tprogress_bar* widget = new tprogress_bar();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed progress bar '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
