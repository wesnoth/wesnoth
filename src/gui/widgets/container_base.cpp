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

#include "gui/widgets/container_base.hpp"

#include "gui/auxiliary/iterator/walker_container.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/window.hpp"

#include <algorithm>

#define LOG_SCOPE_HEADER                                                       \
	"tcontainer(" + get_control_type() + ") [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

container_base::container_base(const implementation::builder_styled_widget& builder, const std::string& control_type)
	: styled_widget(builder, control_type)
	, grid_()
{
	grid_.set_parent(this);
	connect_signal<event::REQUEST_PLACEMENT>(
		std::bind(&container_base::clear_layout_size, this),
		event::dispatcher::back_pre_child);
}

SDL_Rect container_base::get_client_rect() const
{
	return get_rectangle();
}

void container_base::layout_initialize(const bool full_initialization)
{
	// Inherited.
	styled_widget::layout_initialize(full_initialization);

	inject_linked_groups();

	grid_.layout_initialize(full_initialization);
}

void container_base::reduce_width(const unsigned maximum_width)
{
	point size = get_best_size();
	point grid_size = grid_.get_best_size();
	if(static_cast<int>(maximum_width) - border_space().x < grid_size.x) {
		grid_.reduce_width(maximum_width - border_space().x);
		grid_size = grid_.get_best_size();
		size.x = grid_size.x + border_space().x;
		size.y = std::max(size.y, grid_size.y + border_space().y);
	} else {
		size.x = maximum_width;
	}

	set_layout_size(size);
}

void container_base::request_reduce_width(const unsigned maximum_width)
{
	point size = get_best_size();
	point grid_size = grid_.get_best_size();
	if(static_cast<int>(maximum_width) - border_space().x < grid_size.x) {
		grid_.request_reduce_width(maximum_width - border_space().x);
		grid_size = grid_.get_best_size();
		size.x = grid_size.x + border_space().x;
		size.y = std::max(size.y, grid_size.y + border_space().y);
	} else {
		size.x = maximum_width;
	}

	set_layout_size(size);
}

void container_base::demand_reduce_width(const unsigned maximum_width)
{
	grid_.demand_reduce_width(maximum_width - border_space().x);
}

void container_base::reduce_height(const unsigned maximum_height)
{
	point size = get_best_size();
	point grid_size = grid_.get_best_size();
	if(static_cast<int>(maximum_height) - border_space().y < grid_size.y) {
		grid_.reduce_height(maximum_height - border_space().y);
		grid_size = grid_.get_best_size();
		size.y = grid_size.y + border_space().y;
	} else {
		size.y = maximum_height;
	}

	set_layout_size(size);
}

void container_base::request_reduce_height(const unsigned maximum_height)
{
	point size = get_best_size();
	point grid_size = grid_.get_best_size();
	if(static_cast<int>(maximum_height) - border_space().y < grid_size.y) {
		grid_.request_reduce_height(maximum_height - border_space().y);
		grid_size = grid_.get_best_size();
		size.y = grid_size.y + border_space().y;
	} else {
		size.y = maximum_height;
	}

	set_layout_size(size);
}

void container_base::demand_reduce_height(const unsigned maximum_height)
{
	grid_.demand_reduce_height(maximum_height - border_space().y);
}

bool container_base::can_wrap() const
{
	return grid_.can_wrap() || widget::can_wrap();
}

void container_base::place(const point& origin, const point& size)
{
	styled_widget::place(origin, size);

	const SDL_Rect rect = get_client_rect();
	const point client_size(rect.w, rect.h);
	const point client_position(rect.x, rect.y);
	grid_.place(client_position, client_size);
}

bool container_base::has_widget(const widget& widget) const
{
	return widget::has_widget(widget) || grid_.has_widget(widget);
}

point container_base::calculate_best_size() const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	point result(grid_.get_best_size());
	const point border_size = border_space();
	const point default_size = get_config_default_size();

	// If the best size has a value of 0 it's means no limit so don't
	// add the border_size might set a very small best size.
	if(result.x) {
		result.x += border_size.x;
	}
	if(default_size.x != 0 && result.x < default_size.x) {
		result.x = default_size.x;
	}

	if(result.y) {
		result.y += border_size.y;
	}
	if(default_size.y != 0 && result.y < default_size.y) {
		result.y = default_size.y;
	}


	DBG_GUI_L << LOG_HEADER << " border size " << border_size << " returning "
			  << result << ".";

	return result;
}

void container_base::set_origin(const point& origin)
{
	// Inherited.
	widget::set_origin(origin);

	const SDL_Rect rect = get_client_rect();
	const point client_position(rect.x, rect.y);
	grid_.set_origin(client_position);
}

void container_base::set_visible_rectangle(const SDL_Rect& rectangle)
{
	// Inherited.
	widget::set_visible_rectangle(rectangle);

	grid_.set_visible_rectangle(rectangle);
}

void container_base::impl_draw_children()
{
	assert(get_visible() == widget::visibility::visible
	       && grid_.get_visible() == widget::visibility::visible);

	grid_.draw_children();
}

void container_base::layout_children()
{
	grid_.layout_children();
}

widget* container_base::find_at(const point& coordinate,
							  const bool must_be_active)
{
	return grid_.find_at(coordinate, must_be_active);
}

const widget* container_base::find_at(const point& coordinate,
									const bool must_be_active) const
{
	return grid_.find_at(coordinate, must_be_active);
}

widget* container_base::find(const std::string_view id, const bool must_be_active)
{
	widget* result = styled_widget::find(id, must_be_active);
	return result ? result : grid_.find(id, must_be_active);
}

const widget* container_base::find(const std::string_view id,
								 const bool must_be_active) const
{
	const widget* result = styled_widget::find(id, must_be_active);
	return result ? result : grid_.find(id, must_be_active);
}

void container_base::set_active(const bool active)
{
	// Not all our children might have the proper state so let them run
	// unconditionally.
	grid_.set_active(active);

	if(active == get_active()) {
		return;
	}

	queue_redraw();

	set_self_active(active);
}

bool container_base::disable_click_dismiss() const
{
	return styled_widget::disable_click_dismiss() && grid_.disable_click_dismiss();
}

iteration::walker_ptr container_base::create_walker()
{
	return std::make_unique<gui2::iteration::container>(*this);
}

void container_base::init_grid(const builder_grid& grid_builder)
{
	log_scope2(log_gui_general, LOG_SCOPE_HEADER);

	assert(grid_.get_rows() == 0 && grid_.get_cols() == 0);

	grid_builder.build(grid_);
}

point container_base::border_space() const
{
	return point();
}

void container_base::inject_linked_groups()
{
	for(const auto& [id, fixed_width, fixed_height] : get_config()->linked_groups) {
		// No-op if group with this ID already exists
		get_window()->init_linked_size_group(id, fixed_width, fixed_height);
	}
}

} // namespace gui2
