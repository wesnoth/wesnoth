/*
   Copyright (C) 2012 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/matrix.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"

#include "utils/functional.hpp"

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

	pane_ = find_widget<pane>(&content_, "pane", false, true);
}

matrix* matrix::build(const implementation::builder_matrix& builder)
{
	return new matrix(builder);
}

unsigned
matrix::create_item(const std::map<std::string, string_map>& item_data,
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

void
matrix::impl_draw_children(surface& frame_buffer, int x_offset, int y_offset)
{
	content_.draw_children(frame_buffer, x_offset, y_offset);
}

void matrix::layout_children()
{
	content_.layout_children();
}

void matrix::child_populate_dirty_list(window& caller,
										const std::vector<widget*>& call_stack)
{
	std::vector<widget*> child_call_stack = call_stack;
	content_.populate_dirty_list(caller, child_call_stack);
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

widget* matrix::find(const std::string& id, const bool must_be_active)
{
	if(widget* result = widget::find(id, must_be_active)) {
		return result;
	} else {
		return content_.find(id, must_be_active);
	}
}

const widget* matrix::find(const std::string& id, const bool must_be_active)
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

iteration::walker_base* matrix::create_walker()
{
	/**
	 * @todo Implement properly.
	 */
	return nullptr;
}

// }---------- DEFINITION ---------{

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

matrix_definition::matrix_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing matrix " << id << '\n';

	load_resolutions<resolution>(cfg);
}

matrix_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, content(new builder_grid(cfg.child("content", "[matrix_definition]")))
{
	// Note the order should be the same as the enum state_t in matrix.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_matrix
 *
 * == Listbox ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="matrix"}{min=0}{max=-1}{super="generic/widget_instance"}
 *
 *
 * List with the matrix specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 * @end{table}
 *
 *
 * @begin{tag}{name="top"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="top"}
 * @begin{tag}{name="bottom"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="bottom"}
 *
 * @begin{tag}{name="left"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="left"}
 * @begin{tag}{name="right"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="right"}
 *
 * @begin{tag}{name="main"}{min="1"}{max="1"}{super="gui/window/resolution/grid/row/column"}
 * @end{tag}{name="main"}
 * @end{tag}{name="matrix"}
 *
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

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
	, builder_main(create_widget_builder(cfg.child("main", "[matrix]")))
{
	if(const config& top = cfg.child("top")) {
		builder_top = std::make_shared<builder_grid>(top);
	}

	if(const config& bottom = cfg.child("bottom")) {
		builder_bottom = std::make_shared<builder_grid>(bottom);
	}

	if(const config& left = cfg.child("left")) {
		builder_left = std::make_shared<builder_grid>(left);
	}

	if(const config& right = cfg.child("right")) {
		builder_right = std::make_shared<builder_grid>(right);
	}
}

widget* builder_matrix::build() const
{
	return matrix::build(*this);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
