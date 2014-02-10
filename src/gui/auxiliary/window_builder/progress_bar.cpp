/*
   Copyright (C) 2010 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/progress_bar.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/progress_bar.hpp"

namespace gui2
{

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

} // namespace gui2

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
