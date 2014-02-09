/*
   Copyright (C) 2012 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/widget_definition/matrix.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"

namespace gui2
{

tmatrix_definition::tmatrix_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing matrix " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tmatrix_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, content(new tbuilder_grid(cfg.child("content", "[matrix_definition]")))
{
	// Note the order should be the same as the enum tstate in matrix.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
}

} // namespace gui2

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_matrix
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="matrix_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * == Listbox ==
 *
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super=generic/widget_definition/resolution}
 *
 *
 * @begin{tag}{name="state_enabled"}{min=1}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=1}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="content"}{min=1}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="content"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="matrix_definition"}
 * @end{parent}{name="gui/"}
 */
