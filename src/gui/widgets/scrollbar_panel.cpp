/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/scrollbar_panel.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/core/window_builder/helper.hpp"

#include "gettext.hpp"
#include "wml_exception.hpp"

#include "utils/functional.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(scrollbar_panel)

scrollbar_panel::scrollbar_panel(const implementation::builder_scrollbar_panel& builder)
	: scrollbar_container(builder, get_control_type())
{
}

bool scrollbar_panel::get_active() const
{
	return true;
}

unsigned scrollbar_panel::get_state() const
{
	return 0;
}

void scrollbar_panel::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

scrollbar_panel_definition::scrollbar_panel_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing scrollbar panel " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_scrollbar_panel
 *
 * == Scrollbar panel ==
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="scrollbar_panel_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="gui/window_definition/resolution"}
 * The definition of a panel with scrollbars. A panel is a container holding
 * other elements in its grid. A panel is always enabled and can't be
 * disabled. Instead it uses the states as layers to draw on.
 *
 * @begin{table}{config}
 *     grid & grid & &                    A grid containing the widgets for main
 *                                     widget. $
 * @end{table}
 * The following layers exist:
 * * background, the background of the panel.
 * * foreground, the foreground of the panel.
 *
 * @end{tag}{name="resolution"}
 * @end{tag}{name="scrollbar_panel_definition"}
 * @end{parent}{name="gui/"}
 */
scrollbar_panel_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid()
{
	// The panel needs to know the order.
	state.emplace_back(cfg.child("background"));
	state.emplace_back(cfg.child("foreground"));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_scrollbar_panel
 *
 * == Scrollbar panel ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="scrollbar_panel"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * Instance of a scrollbar_panel.
 *
 * List with the scrollbar_panel specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *
 *     definition & section & &        This defines how a scrollbar_panel item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list. $
 *
 * @end{table}
 * @begin{tag}{name="definition"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="definition"}
 * @end{tag}{name="scrollbar_panel"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_scrollbar_panel::builder_scrollbar_panel(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, grid_(nullptr)
{
	const config& grid_definition = cfg.child("definition");

	VALIDATE(grid_definition, _("No list defined."));
	grid_ = std::make_shared<builder_grid>(grid_definition);
	assert(grid_);
}

widget* builder_scrollbar_panel::build() const
{
	scrollbar_panel* panel = new scrollbar_panel(*this);

	panel->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	panel->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed scrollbar_panel '" << id
			  << "' with definition '" << definition << "'.\n";

	const auto conf = panel->cast_config_to<scrollbar_panel_definition>();
	assert(conf);

	panel->init_grid(conf->grid);
	panel->finalize_setup();

	/*** Fill the content grid. ***/
	grid* content_grid = panel->content_grid();
	assert(content_grid);

	const unsigned rows = grid_->rows;
	const unsigned cols = grid_->cols;

	content_grid->set_rows_cols(rows, cols);

	for(unsigned x = 0; x < rows; ++x) {
		content_grid->set_row_grow_factor(x, grid_->row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				content_grid->set_column_grow_factor(y,
													 grid_->col_grow_factor[y]);
			}

			widget* widget = grid_->widgets[x * cols + y]->build();
			content_grid->set_child(widget,
									x,
									y,
									grid_->flags[x * cols + y],
									grid_->border_size[x * cols + y]);
		}
	}

	return panel;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
