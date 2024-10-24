/*
	Copyright (C) 2008 - 2024
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

#include "gui/core/window_builder.hpp"

#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/core/log.hpp"
#include "gui/core/gui_definition.hpp"
#include "gui/core/static_registry.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/window_builder/instance.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/viewport.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include <functional>

namespace gui2
{
builder_widget::builder_widget(const config& cfg)
	: id(cfg["id"])
	, linked_group(cfg["linked_group"])
	, debug_border_mode(widget::debug_border::none)
	, debug_border_color(decode_color(cfg["debug_border_color"]))
{
	// TODO: move to a `decode` function?
	switch(const int dbm = cfg["debug_border_mode"].to_int(0); dbm) {
	case 0:
		debug_border_mode = widget::debug_border::none;
		break;
	case 1:
		debug_border_mode = widget::debug_border::outline;
		break;
	case 2:
		debug_border_mode = widget::debug_border::fill;
		break;
	default:
		WRN_GUI_P << "Widget builder: unknown debug border mode " << dbm << ".";
	}
}

builder_widget_ptr create_widget_builder(const config& cfg)
{
	VALIDATE(cfg.all_children_count() == 1, "Grid cell does not have exactly 1 child.");
	auto [widget_key, widget_cfg] = *cfg.ordered_begin();

	if(widget_key == "grid") {
		return std::make_shared<builder_grid>(widget_cfg);
	}

	if(widget_key == "instance") {
		return std::make_shared<implementation::builder_instance>(widget_cfg);
	}

	if(widget_key == "pane") {
		return std::make_shared<implementation::builder_pane>(widget_cfg);
	}

	if(widget_key == "viewport") {
		return std::make_shared<implementation::builder_viewport>(widget_cfg);
	}

	for(const auto& [type, builder] : widget_builder_lookup()) {
		if(type == "window" || type == "tooltip") {
			continue;
		}

		if(widget_key == type) {
			return builder(widget_cfg);
		}
	}

	// FAIL() doesn't return
	//
	// To fix this: add your new widget to source-lists/libwesnoth_widgets and rebuild.

	FAIL("Unknown widget type " + widget_key);
}

std::unique_ptr<widget> build_single_widget_instance_helper(const std::string& type, const config& cfg)
{
	const auto& iter = widget_builder_lookup().find(type);
	VALIDATE(iter != widget_builder_lookup().end(), "Invalid widget type '" + type + "'");

	widget_builder_func_t& builder = iter->second;
	return builder(cfg)->build();
}

void builder_window::read(const config& cfg)
{
	VALIDATE(!id_.empty(), missing_mandatory_wml_key("window", "id"));
	VALIDATE(!description_.empty(), missing_mandatory_wml_key("window", "description"));

	DBG_GUI_P << "Window builder: reading data for window " << id_ << ".";

	config::const_child_itors cfgs = cfg.child_range("resolution");
	VALIDATE(!cfgs.empty(), _("No resolution defined for ") + id_);

	for(const auto& i : cfgs) {
		resolutions.emplace_back(i);
	}
}

builder_window::window_resolution::window_resolution(const config& cfg)
	: window_width(cfg["window_width"].to_unsigned())
	, window_height(cfg["window_height"].to_unsigned())
	, automatic_placement(cfg["automatic_placement"].to_bool(true))
	, x(cfg["x"])
	, y(cfg["y"])
	, width(cfg["width"])
	, height(cfg["height"])
	, reevaluate_best_size(cfg["reevaluate_best_size"])
	, functions()
	, vertical_placement(implementation::get_v_align(cfg["vertical_placement"]))
	, horizontal_placement(implementation::get_h_align(cfg["horizontal_placement"]))
	, maximum_width(cfg["maximum_width"], 0u)
	, maximum_height(cfg["maximum_height"], 0u)
	, click_dismiss(cfg["click_dismiss"].to_bool())
	, definition(cfg["definition"])
	, linked_groups()
	, tooltip(cfg.child_or_empty("tooltip"), "tooltip")
	, helptip(cfg.child_or_empty("helptip"), "helptip")
	, grid(nullptr)
{
	if(!cfg["functions"].empty()) {
		wfl::formula(cfg["functions"], &functions).evaluate();
	}

	auto c = cfg.optional_child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = std::make_shared<builder_grid>(*c);

	if(!automatic_placement) {
		VALIDATE(width.has_formula() || width(), missing_mandatory_wml_key("resolution", "width"));
		VALIDATE(height.has_formula() || height(), missing_mandatory_wml_key("resolution", "height"));
	}

	DBG_GUI_P << "Window builder: parsing resolution " << window_width << ',' << window_height;

	if(definition.empty()) {
		definition = "default";
	}

	linked_groups = parse_linked_group_definitions(cfg);
}

builder_window::window_resolution::tooltip_info::tooltip_info(const config& cfg, const std::string& tagname)
	: id(cfg["id"])
{
	VALIDATE(!id.empty(), missing_mandatory_wml_key("[window][resolution][" + tagname + "]", "id"));
}

builder_grid::builder_grid(const config& cfg)
	: builder_widget(cfg)
	, rows(0)
	, cols(0)
	, row_grow_factor()
	, col_grow_factor()
	, flags()
	, border_size()
	, widgets()
{
	log_scope2(log_gui_parse, "Window builder: parsing a grid");

	for(const auto& row : cfg.child_range("row")) {
		unsigned col = 0;

		row_grow_factor.push_back(row["grow_factor"].to_unsigned());

		for(const auto& c : row.child_range("column")) {
			flags.push_back(implementation::read_flags(c));
			border_size.push_back(c["border_size"].to_unsigned());
			if(rows == 0) {
				col_grow_factor.push_back(c["grow_factor"].to_unsigned());
			}

			widgets.push_back(create_widget_builder(c));

			++col;
		}

		if(col == 0) {
			const t_string msg = VGETTEXT("Grid ‘$grid’ row $row must have at least one column.", {
				{"grid", id}, {"row", std::to_string(rows)}
			});

			FAIL(msg);
		}

		++rows;

		if(rows == 1) {
			cols = col;
		} else if(col != cols) {
			const t_string msg = VGETTEXT("Grid ‘$grid’ row $row has a differing number of columns ($found found, $expected expected)", {
				{"grid", id}, {"row", std::to_string(rows)}, {"found", std::to_string(col)}, {"expected", std::to_string(cols)}
			});

			FAIL(msg);
		}
	}

	DBG_GUI_P << "Window builder: grid has " << rows << " rows and " << cols << " columns.";
}

std::unique_ptr<widget> builder_grid::build() const
{
	auto result = std::make_unique<grid>();
	build(*result);
	return result;
}

std::unique_ptr<widget> builder_grid::build(const replacements_map& replacements) const
{
	auto result = std::make_unique<grid>();
	build(*result, replacements);
	return result;
}

void builder_grid::build(grid& grid, optional_replacements replacements) const
{
	grid.set_id(id);
	grid.set_linked_group(linked_group);
	grid.set_rows_cols(rows, cols);

	log_scope2(log_gui_general, "Window builder: building grid");

	DBG_GUI_G << "Window builder: grid '" << id << "' has " << rows << " rows and " << cols << " columns.";

	for(unsigned x = 0; x < rows; ++x) {
		grid.set_row_grow_factor(x, row_grow_factor[x]);

		for(unsigned y = 0; y < cols; ++y) {
			if(x == 0) {
				grid.set_column_grow_factor(y, col_grow_factor[y]);
			}

			DBG_GUI_G << "Window builder: adding child at " << x << ',' << y << ".";

			const unsigned int i = x * cols + y;

			if(replacements) {
				auto widget = widgets[i]->build(replacements.value());
				grid.set_child(std::move(widget), x, y, flags[i], border_size[i]);
			} else {
				auto widget = widgets[i]->build();
				grid.set_child(std::move(widget), x, y, flags[i], border_size[i]);
			}
		}
	}
}

} // namespace gui2
