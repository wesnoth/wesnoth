/*
   Copyright (C) 2007 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/widget_definition/label.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2
{

tlabel_definition::tlabel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing label " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_label
 *
 * == Label ==
 *
 * @macro = label_description
 *
 * Although the label itself has no event interaction it still has two states.
 * The reason is that labels are often used as visual indication of the state
 * of the widget it labels.
 *
 * Note: The above is outdated, if "link_aware" is enabled then there is interaction.
 *
 *
 * The following states exist:
 * * state_enabled, the label is enabled.
 * * state_disabled, the label is disabled.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="label_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{table}{config}
 *     link_aware & f_bool & false & Whether the label is link aware. This means
 *                                     it is rendered with links highlighted,
 *                                     and responds to click events on those
 *                                     links. $
 *     link_color & string & #ffff00 & The color to render links with. This
 *                                     string will be used verbatim in pango
 *                                     markup for each link. $
 * @end{table}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="label_definition"}
 * @end{parent}{name="gui/"}
 */
tlabel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, link_aware(cfg["link_aware"].to_bool(false))
	, link_color(cfg["link_color"].str().size() > 0 ? cfg["link_color"].str() : "#ffff00")
{
	// Note the order should be the same as the enum tstate is label.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
}

} // namespace gui2
