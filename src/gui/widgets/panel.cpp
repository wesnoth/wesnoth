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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/panel.hpp"

#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(panel)

SDL_Rect tpanel::get_client_rect() const
{
	boost::intrusive_ptr<const tpanel_definition::tresolution> conf
			= boost::dynamic_pointer_cast<const tpanel_definition::tresolution>(
					config());
	assert(conf);

	SDL_Rect result = get_rectangle();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

bool tpanel::get_active() const
{
	return true;
}

unsigned tpanel::get_state() const
{
	return 0;
}

void tpanel::impl_draw_background(surface& frame_buffer, int x_offset, int y_offset)
{
	DBG_GUI_D << LOG_HEADER << " size " << get_rectangle() << ".\n";

	canvas(0).blit(frame_buffer,
				   calculate_blitting_rectangle(x_offset, y_offset));
}

void tpanel::impl_draw_foreground(surface& frame_buffer, int x_offset, int y_offset)
{
	canvas(1).blit(frame_buffer,
				   calculate_blitting_rectangle(x_offset, y_offset));
}

tpoint tpanel::border_space() const
{
	boost::intrusive_ptr<const tpanel_definition::tresolution> conf
			= boost::dynamic_pointer_cast<const tpanel_definition::tresolution>(
					config());
	assert(conf);

	return tpoint(conf->left_border + conf->right_border,
				  conf->top_border + conf->bottom_border);
}

const std::string& tpanel::get_control_type() const
{
	static const std::string type = "panel";
	return type;
}

void tpanel::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

tpanel_definition::tpanel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_panel
 *
 * == Panel ==
 *
 * @macro = panel_description
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="panel_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * A panel is always enabled and can't be disabled. Instead it uses the
 * states as layers to draw on.
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * The resolution for a panel also contains the following keys:
 * @begin{table}{config}
 *     top_border & unsigned & 0 &     The size which isn't used for the client
 *                                   area. $
 *     bottom_border & unsigned & 0 &  The size which isn't used for the client
 *                                   area. $
 *     left_border & unsigned & 0 &    The size which isn't used for the client
 *                                   area. $
 *     right_border & unsigned & 0 &   The size which isn't used for the client
 *                                   area. $
 * @end{table}
 *
 * The following layers exist:
 * * background, the background of the panel.
 * * foreground, the foreground of the panel.
 * @begin{tag}{name="foreground"}{min=0}{max=1}
 * @allow{link}{name="generic/state/draw"}
 * @end{tag}{name="foreground"}
 * @begin{tag}{name="background"}{min=0}{max=1}
 * @allow{link}{name="generic/state/draw"}
 * @end{tag}{name="background"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="panel_definition"}
 * @end{parent}{name="gui/"}
 */
tpanel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, top_border(cfg["top_border"])
	, bottom_border(cfg["bottom_border"])
	, left_border(cfg["left_border"])
	, right_border(cfg["right_border"])
{
	// The panel needs to know the order.
	state.push_back(tstate_definition(cfg.child("background")));
	state.push_back(tstate_definition(cfg.child("foreground")));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{panel_description}
 *
 *        A panel is an item which can hold other items. The difference
 *        between a grid and a panel is that it's possible to define how a
 *        panel looks. A grid in an invisible container to just hold the
 *        items.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_panel
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="panel"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Panel ==
 *
 * @macro = panel_description
 *
 * @begin{table}{config}
 *     grid & grid & &                 Defines the grid with the widgets to
 *                                     place on the panel. $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @end{tag}{name="panel"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

tbuilder_panel::tbuilder_panel(const config& cfg)
	: tbuilder_control(cfg), grid(nullptr)
{
	const config& c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = new tbuilder_grid(c);
}

twidget* tbuilder_panel::build() const
{
	tpanel* widget = new tpanel();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed panel '" << id << "' with definition '"
			  << definition << "'.\n";

	widget->init_grid(grid);
	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
