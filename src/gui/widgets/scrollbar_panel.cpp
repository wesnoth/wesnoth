/*
	Copyright (C) 2009 - 2024
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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/scrollbar_panel.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/core/window_builder/helper.hpp"

#include "gettext.hpp"
#include "wml_exception.hpp"


namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(scrollbar_panel)

scrollbar_panel::scrollbar_panel(const implementation::builder_scrollbar_panel& builder)
	: scrollbar_container(builder, type())
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
	DBG_GUI_P << "Parsing scrollbar panel " << id;

	load_resolutions<resolution>(cfg);
}

scrollbar_panel_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid()
{
	// The panel needs to know the order.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "background", missing_mandatory_wml_tag("scrollbar_panel_definition][resolution", "background")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "foreground", missing_mandatory_wml_tag("scrollbar_panel_definition][resolution", "foreground")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", missing_mandatory_wml_tag("scrollbar_panel][definition", "grid"));
	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_scrollbar_panel::builder_scrollbar_panel(const config& cfg)
	: builder_scrollbar_container(cfg)
	, grid_(nullptr)
{
	auto grid_definition = cfg.optional_child("definition");

	VALIDATE(grid_definition, _("No list defined."));
	grid_ = std::make_shared<builder_grid>(*grid_definition);
	assert(grid_);
}

std::unique_ptr<widget> builder_scrollbar_panel::build() const
{
	auto panel = std::make_unique<scrollbar_panel>(*this);

	DBG_GUI_G << "Window builder: placed scrollbar_panel '" << id
			  << "' with definition '" << definition << "'.";

	const auto conf = panel->cast_config_to<scrollbar_panel_definition>();
	assert(conf);

	panel->init_grid(*conf->grid);
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

			auto widget = grid_->widgets[x * cols + y]->build();
			content_grid->set_child(std::move(widget),
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
