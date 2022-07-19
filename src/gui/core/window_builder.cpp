/*
	Copyright (C) 2008 - 2022
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
#include "gui/widgets/settings.hpp"
#include "gui/widgets/viewport.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include <functional>

namespace gui2
{

std::unique_ptr<window> build(const builder_window::window_resolution& definition)
{
	// We set the values from the definition since we can only determine the
	// best size (if needed) after all widgets have been placed.
	auto win = std::make_unique<window>(definition);
	assert(win);

	for(const auto& lg : definition.linked_groups) {
		if(win->has_linked_size_group(lg.id)) {
			t_string msg = VGETTEXT("Linked '$id' group has multiple definitions.", {{"id", lg.id}});

			FAIL(msg);
		}

		win->init_linked_size_group(lg.id, lg.fixed_width, lg.fixed_height);
	}

	win->set_click_dismiss(definition.click_dismiss);

	const auto conf = win->cast_config_to<window_definition>();
	assert(conf);

	if(conf->grid) {
		win->init_grid(*conf->grid);
		win->finalize(*definition.grid);
	} else {
		win->init_grid(*definition.grid);
	}

	win->add_to_keyboard_chain(win.get());

	return win;
}

std::unique_ptr<window> build(const std::string& type)
{
	const builder_window::window_resolution& definition = get_window_builder(type);
	auto window = build(definition);
	window->set_id(type);
	return window;
}

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
	config::const_all_children_itors children = cfg.all_children_range();
	VALIDATE(children.size() == 1, "Grid cell does not have exactly 1 child.");

	if(const auto grid = cfg.optional_child("grid")) {
		return std::make_shared<builder_grid>(grid.value());
	}

	if(const auto instance = cfg.optional_child("instance")) {
		return std::make_shared<implementation::builder_instance>(instance.value());
	}

	if(const auto pane = cfg.optional_child("pane")) {
		return std::make_shared<implementation::builder_pane>(pane.value());
	}

	if(const auto viewport = cfg.optional_child("viewport")) {
		return std::make_shared<implementation::builder_viewport>(viewport.value());
	}

	for(const auto& [type, builder] : widget_builder_lookup()) {
		if(type == "window" || type == "tooltip") {
			continue;
		}

		if(const auto c = cfg.optional_child(type)) {
			return builder(c.value());
		}
	}

	// FAIL() doesn't return
	//
	// To fix this: add your new widget to source-lists/libwesnoth_widgets and rebuild.

	FAIL("Unknown widget type " + cfg.ordered_begin()->key);
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
	VALIDATE(!cfgs.empty(), _("No resolution defined."));

	for(const auto& i : cfgs) {
		resolutions.emplace_back(i);
	}
}

builder_window::window_resolution::window_resolution(const config& cfg)
	: window_width(cfg["window_width"])
	, window_height(cfg["window_height"])
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

	const config& c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = std::make_shared<builder_grid>(c);

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

		row_grow_factor.push_back(row["grow_factor"]);

		for(const auto& c : row.child_range("column")) {
			flags.push_back(implementation::read_flags(c));
			border_size.push_back(c["border_size"]);
			if(rows == 0) {
				col_grow_factor.push_back(c["grow_factor"]);
			}

			widgets.push_back(create_widget_builder(c));

			++col;
		}

		if(col == 0) {
			const t_string msg = VGETTEXT("Grid '$grid' row $row must have at least one column.", {
				{"grid", id}, {"row", std::to_string(rows)}
			});

			FAIL(msg);
		}

		++rows;

		if(rows == 1) {
			cols = col;
		} else if(col != cols) {
			const t_string msg = VGETTEXT("Grid '$grid' row $row has a differing number of columns ($found found, $expected expected)", {
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
