/*
	Copyright (C) 2012 - 2024
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

#include "gui/widgets/matrix.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/iterator/walker.hpp"
#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "wml_exception.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(matrix)


state_default::state_default() : state_(ENABLED)
{
}
void state_default::set_active(const bool active)
{
	if(get_active() != active) {
		state_ = active ? ENABLED : DISABLED;
	}
}

bool state_default::get_active() const
{
	return state_ != DISABLED;
}

unsigned state_default::get_state() const
{
	return state_;
}

matrix::matrix(const implementation::builder_matrix& builder)
	: tbase(builder, "matrix"), content_(), pane_(nullptr)
{
	const auto cfg = cast_config_to<matrix_definition>();

	builder_widget::replacements_map replacements;
	replacements.emplace("_main", builder.builder_main);

	if(builder.builder_top) {
		replacements.emplace("_top", builder.builder_top);
	}

	if(builder.builder_left) {
		replacements.emplace("_left", builder.builder_left);
	}

	if(builder.builder_right) {
		replacements.emplace("_right", builder.builder_right);
	}

	if(builder.builder_bottom) {
		replacements.emplace("_bottom", builder.builder_bottom);
	}

	cfg->content->build(content_, replacements);
	content_.set_parent(this);

	pane_ = content_.find_widget<pane>("pane", false, true);
}

unsigned
matrix::create_item(const widget_data& item_data,
					 const std::map<std::string, std::string>& tags)
{
	return pane_->create_item(item_data, tags);
}

void matrix::place(const point& origin, const point& size)
{
	widget::place(origin, size);

	content_.place(origin, size);
}

void matrix::layout_initialize(const bool full_initialization)
{
	content_.layout_initialize(full_initialization);
}

void matrix::impl_draw_children()
{
	content_.draw_children();
}

void matrix::layout_children()
{
	content_.layout_children();
}

void matrix::request_reduce_width(const unsigned /*maximum_width*/)
{
}

widget* matrix::find_at(const point& coordinate, const bool must_be_active)
{
	return content_.find_at(coordinate, must_be_active);
}

const widget* matrix::find_at(const point& coordinate,
								const bool must_be_active) const
{
	return content_.find_at(coordinate, must_be_active);
}

widget* matrix::find(const std::string_view id, const bool must_be_active)
{
	if(widget* result = widget::find(id, must_be_active)) {
		return result;
	} else {
		return content_.find(id, must_be_active);
	}
}

const widget* matrix::find(const std::string_view id, const bool must_be_active)
		const
{
	if(const widget* result = widget::find(id, must_be_active)) {
		return result;
	} else {
		return content_.find(id, must_be_active);
	}
}

point matrix::calculate_best_size() const
{
	point size = content_.get_best_size();

	return size;
}

bool matrix::disable_click_dismiss() const
{
	return false;
}

/**
 * @todo Implement properly.
 */
iteration::walker_ptr matrix::create_walker()
{
	return nullptr;
}

// }---------- DEFINITION ---------{

matrix_definition::matrix_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing matrix " << id;

	load_resolutions<resolution>(cfg);
}

matrix_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, content(new builder_grid(VALIDATE_WML_CHILD(cfg, "content", missing_mandatory_wml_tag("matrix", "content"))))
{
	// Note the order should be the same as the enum state_t in matrix.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("matrix_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("matrix_definition][resolution", "state_disabled")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_matrix::builder_matrix(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, builder_top(nullptr)
	, builder_bottom(nullptr)
	, builder_left(nullptr)
	, builder_right(nullptr)
	, builder_main(create_widget_builder(VALIDATE_WML_CHILD(cfg, "main", missing_mandatory_wml_tag("matrix", "main"))))
{
	if(auto top = cfg.optional_child("top")) {
		builder_top = std::make_shared<builder_grid>(*top);
	}

	if(auto bottom = cfg.optional_child("bottom")) {
		builder_bottom = std::make_shared<builder_grid>(*bottom);
	}

	if(auto left = cfg.optional_child("left")) {
		builder_left = std::make_shared<builder_grid>(*left);
	}

	if(auto right = cfg.optional_child("right")) {
		builder_right = std::make_shared<builder_grid>(*right);
	}
}

std::unique_ptr<widget> builder_matrix::build() const
{
	return std::make_unique<matrix>(*this);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
