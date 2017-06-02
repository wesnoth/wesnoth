/*
   Copyright (C) 2007 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/grid.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/core/event/message.hpp"
#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "sdl/rect.hpp"
#include "video.hpp"

namespace gui2
{

/***** ***** ***** Constructor and destructor. ***** ***** *****/

widget::widget()
	: id_("")
	, parent_(nullptr)
	, x_(-1)
	, y_(-1)
	, width_(0)
	, height_(0)
	, layout_size_()
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, last_best_size_()
#endif
	, linked_group_()
	, visible_(visibility::visible)
	, redraw_action_(redraw_action::full)
	, clipping_rectangle_()
	, debug_border_mode_(0)
	, debug_border_color_(0,0,0,0)
{
	DBG_GUI_LF << "widget create: " << static_cast<void*>(this) << "\n";
}

widget::widget(const builder_widget& builder)
	: id_(builder.id)
	, parent_(nullptr)
	, x_(-1)
	, y_(-1)
	, width_(0)
	, height_(0)
	, layout_size_()
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, last_best_size_()
#endif
	, linked_group_(builder.linked_group)
	, visible_(visibility::visible)
	, redraw_action_(redraw_action::full)
	, clipping_rectangle_()
	, debug_border_mode_(builder.debug_border_mode)
	, debug_border_color_(builder.debug_border_color)
{
	DBG_GUI_LF << "widget create: " << static_cast<void*>(this) << "\n";
}

widget::~widget()
{
	DBG_GUI_LF
	<< "widget destroy: " << static_cast<void*>(this) << " (id: " << id_
	<< ")\n";

	widget* p = parent();
	while(p) {
		fire(event::NOTIFY_REMOVAL, *p, nullptr);
		p = p->parent();
	}

	if(!linked_group_.empty()) {
		if(window* window = get_window()) {
			window->remove_linked_widget(linked_group_, this);
		}
	}
}

/***** ***** ***** ***** ID functions. ***** ***** ***** *****/

void widget::set_id(const std::string& id)
{
	styled_widget* this_ctrl = dynamic_cast<styled_widget*>(this);

	DBG_GUI_LF
	<< "set id of " << static_cast<void*>(this) << " to '" << id << "' "
	<< "(was '" << id_ << "'). Widget type: "
	<< (this_ctrl ? this_ctrl->get_control_type() : typeid(widget).name()) << "\n";

	id_ = id;
}

const std::string& widget::id() const
{
	return id_;
}

/***** ***** ***** ***** Parent functions ***** ***** ***** *****/

window* widget::get_window()
{
	// Go up into the parent tree until we find the top level
	// parent, we can also be the toplevel so start with
	// ourselves instead of our parent.
	widget* result = this;
	while(result->parent_) {
		result = result->parent_;
	}

	// on error dynamic_cast returns nullptr which is what we want.
	return dynamic_cast<window*>(result);
}

const window* widget::get_window() const
{
	// Go up into the parent tree until we find the top level
	// parent, we can also be the toplevel so start with
	// ourselves instead of our parent.
	const widget* result = this;
	while(result->parent_) {
		result = result->parent_;
	}

	// on error dynamic_cast returns nullptr which is what we want.
	return dynamic_cast<const window*>(result);
}

grid* widget::get_parent_grid()
{
	widget* result = parent_;
	while(result && dynamic_cast<grid*>(result) == nullptr) {
		result = result->parent_;
	}

	return result ? dynamic_cast<grid*>(result) : nullptr;
}

dialogs::modal_dialog* widget::dialog()
{
	window* window = get_window();
	return window ? window->dialog() : nullptr;
}

void widget::set_parent(widget* parent)
{
	parent_ = parent;
}

widget* widget::parent()
{
	return parent_;
}

/***** ***** ***** ***** Size and layout functions. ***** ***** ***** *****/

void widget::layout_initialize(const bool /*full_initialization*/)
{
	assert(visible_ != visibility::invisible);
	assert(get_window());

	layout_size_ = point();
	if(!linked_group_.empty()) {
		get_window()->add_linked_widget(linked_group_, this);
	}
}

void widget::demand_reduce_width(const unsigned /*maximum_width*/)
{
	/* DO NOTHING */
}

void widget::request_reduce_height(const unsigned /*maximum_height*/)
{
	/* DO NOTHING */
}

void widget::demand_reduce_height(const unsigned /*maximum_height*/)
{
	/* DO NOTHING */
}

point widget::get_best_size() const
{
	assert(visible_ != visibility::invisible);

	point result = layout_size_;
	if(result == point()) {
		result = calculate_best_size();
		//Adjust to linked widget size if linked widget size was already calculated.
		if(!get_window()->get_need_layout() && !linked_group_.empty())
		{
			point linked_size = get_window()->get_linked_size(linked_group_);
			result.x = std::max(result.x, linked_size.x);
			result.y = std::max(result.y, linked_size.y);
		}
	}

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	last_best_size_ = result;
#endif

	return result;
}

bool widget::can_wrap() const
{
	return false;
}

void widget::set_origin(const point& origin)
{
	x_ = origin.x;
	y_ = origin.y;
}

void widget::set_size(const point& size)
{
	assert(size.x >= 0);
	assert(size.y >= 0);

	width_ = size.x;
	height_ = size.y;
}

void widget::place(const point& origin, const point& size)
{
	assert(size.x >= 0);
	assert(size.y >= 0);

	x_ = origin.x;
	y_ = origin.y;
	width_ = size.x;
	height_ = size.y;

#if 0
	std::cerr
			<< "Id " << id()
			<< " rect " << get_rectangle()
			<< " parent "
			<< (parent ? parent->get_x() : 0)
			<< ','
			<< (parent ? parent->get_y() : 0)
			<< " screen origin " << x_ << ',' << y_
			<< ".\n";
#endif
}

void widget::move(const int x_offset, const int y_offset)
{
	x_ += x_offset;
	y_ += y_offset;
}

void widget::set_horizontal_alignment(const std::string& alignment)
{
	grid* parent_grid = get_parent_grid();
	if(!parent_grid) {
		return;
	}

	parent_grid->set_child_alignment(this, implementation::get_h_align(alignment), grid::HORIZONTAL_MASK);

	// TODO: evaluate necessity
	//get_window()->invalidate_layout();
}

void widget::set_vertical_alignment(const std::string& alignment)
{
	grid* parent_grid = get_parent_grid();
	if(!parent_grid) {
		return;
	}

	parent_grid->set_child_alignment(this, implementation::get_v_align(alignment), grid::VERTICAL_MASK);

	// TODO: evaluate necessity
	//get_window()->invalidate_layout();
}

void widget::layout_children()
{
	/* DO NOTHING */
}

point widget::get_origin() const
{
	return point(x_, y_);
}

point widget::get_size() const
{
	return point(width_, height_);
}

SDL_Rect widget::get_rectangle() const
{
	return create_rect(get_origin(), get_size());
}

int widget::get_x() const
{
	return x_;
}

int widget::get_y() const
{
	return y_;
}

unsigned widget::get_width() const
{
	return width_;
}

unsigned widget::get_height() const
{
	return height_;
}

void widget::set_layout_size(const point& size)
{
	layout_size_ = size;
}

const point& widget::layout_size() const
{
	return layout_size_;
}

void widget::set_linked_group(const std::string& linked_group)
{
	linked_group_ = linked_group;
}

/***** ***** ***** ***** Drawing functions. ***** ***** ***** *****/

SDL_Rect widget::calculate_blitting_rectangle(const int x_offset, const int y_offset) const
{
	SDL_Rect result = get_rectangle();
	result.x += x_offset;
	result.y += y_offset;
	return result;
}

SDL_Rect widget::calculate_clipping_rectangle(const int x_offset, const int y_offset) const
{
	SDL_Rect result = clipping_rectangle_;
	result.x += x_offset;
	result.y += y_offset;
	return result;
}

namespace
{
/**
 * Small RAII helper class to set the renderer viewport and clip rect for the drawing routines.
 */
class viewport_and_clip_rect_setter
{
public:
	viewport_and_clip_rect_setter(const widget& widget, int x_offset, int y_offset)
		: renderer_(*CVideo::get_singleton().get_window())
	{
		// Set viewport.
		const SDL_Rect dst_rect = widget.calculate_blitting_rectangle(x_offset, y_offset);
		SDL_RenderSetViewport(renderer_, &dst_rect);

		// Set clip rect, if appropriate.
		if(widget.get_drawing_action() != widget::redraw_action::partly) {
			return;
		}

		SDL_Rect clip_rect = widget.calculate_clipping_rectangle(x_offset, y_offset);

		// Adjust clip rect origin to match the viewport origin. Currently, the both rects are mapped to
		// absolute screen coordinates. However, setting the viewport essentially moves the screen origin,
		// meaning if both the viewport rect and clip rect have x = 100, then clipping will actually
		// happen at x = 200.
		clip_rect.x -= dst_rect.x;
		clip_rect.y -= dst_rect.y;

		SDL_RenderSetClipRect(renderer_, &clip_rect);
	}

	~viewport_and_clip_rect_setter()
	{
		SDL_RenderSetClipRect(renderer_, nullptr);
		SDL_RenderSetViewport(renderer_, nullptr);
	}

private:
	SDL_Renderer* renderer_;
};

} // anon namespace

/**
 * @todo remove the offset arguments from these functions.
 * Currently they're only needed by the minimap.
 */

void widget::draw_background(int x_offset, int y_offset)
{
	assert(visible_ == visibility::visible);

	viewport_and_clip_rect_setter setter(*this, x_offset, y_offset);

	draw_debug_border(x_offset, y_offset);
	impl_draw_background(x_offset, y_offset);
}

void widget::draw_children(int x_offset, int y_offset)
{
	assert(visible_ == visibility::visible);

	viewport_and_clip_rect_setter setter(*this, x_offset, y_offset);

	impl_draw_children(x_offset, y_offset);
}

void widget::draw_foreground(int x_offset, int y_offset)
{
	assert(visible_ == visibility::visible);

	viewport_and_clip_rect_setter setter(*this, x_offset, y_offset);

	impl_draw_foreground(x_offset, y_offset);
}

SDL_Rect widget::get_dirty_rectangle() const
{
	return redraw_action_ == redraw_action::full ? get_rectangle()
												  : clipping_rectangle_;
}

void widget::set_visible_rectangle(const SDL_Rect& rectangle)
{
	clipping_rectangle_ = sdl::intersect_rects(rectangle, get_rectangle());

	if(clipping_rectangle_ == get_rectangle()) {
		redraw_action_ = redraw_action::full;
	} else if(clipping_rectangle_ == sdl::empty_rect) {
		redraw_action_ = redraw_action::none;
	} else {
		redraw_action_ = redraw_action::partly;
	}
}

void widget::set_visible(const visibility visible)
{
	if(visible == visible_) {
		return;
	}

	// Switching to or from invisible should invalidate the layout
	// if the widget has already been laid out.
	const bool need_resize = visible_ == visibility::invisible
		|| (visible == visibility::invisible && get_size() != point());
	visible_ = visible;

	if(need_resize) {
		if(visible == visibility::visible && new_widgets) {
			event::message message;
			fire(event::REQUEST_PLACEMENT, *this, message);
		} else {
			window* window = get_window();
			if(window) {
				window->invalidate_layout();
			}
		}
	}
}

widget::visibility widget::get_visible() const
{
	return visible_;
}

widget::redraw_action widget::get_drawing_action() const
{
	return (width_ == 0 || height_ == 0) ? redraw_action::none
										 : redraw_action_;
}

void widget::set_debug_border_mode(const unsigned debug_border_mode)
{
	debug_border_mode_ = debug_border_mode;
}

void widget::set_debug_border_color(const color_t debug_border_color)
{
	debug_border_color_ = debug_border_color;
}

void widget::draw_debug_border()
{
	SDL_Rect r = redraw_action_ == redraw_action::partly ? clipping_rectangle_
														  : get_rectangle();

	switch(debug_border_mode_) {
		case 0:
			/* DO NOTHING */
			break;
		case 1:
			sdl::draw_rectangle(r, debug_border_color_);
			break;

		case 2:
			sdl::fill_rectangle(r, debug_border_color_);
			break;

		default:
			assert(false);
	}
}

void
widget::draw_debug_border(int x_offset, int y_offset)
{
	SDL_Rect r = redraw_action_ == redraw_action::partly
						 ? calculate_clipping_rectangle(x_offset, y_offset)
						 : calculate_blitting_rectangle(x_offset, y_offset);

	switch(debug_border_mode_) {
		case 0:
			/* DO NOTHING */
			break;

		case 1:
			sdl::draw_rectangle(r, debug_border_color_);
			break;

		case 2:
			sdl::fill_rectangle(r, debug_border_color_);
			break;

		default:
			assert(false);
	}
}

/***** ***** ***** ***** Query functions ***** ***** ***** *****/

widget* widget::find_at(const point& coordinate, const bool must_be_active)
{
	return is_at(coordinate, must_be_active) ? this : nullptr;
}

const widget* widget::find_at(const point& coordinate,
								const bool must_be_active) const
{
	return is_at(coordinate, must_be_active) ? this : nullptr;
}

widget* widget::find(const std::string& id, const bool /*must_be_active*/)
{
	return id_ == id ? this : nullptr;
}

const widget* widget::find(const std::string& id,
							 const bool /*must_be_active*/) const
{
	return id_ == id ? this : nullptr;
}

bool widget::has_widget(const widget& widget) const
{
	return &widget == this;
}

bool widget::is_at(const point& coordinate) const
{
	return is_at(coordinate, true);
}

bool widget::recursive_is_visible(const widget* widget, const bool must_be_active) const
{
	while(widget) {
		if(widget->visible_ == visibility::invisible
		   || (widget->visible_ == visibility::hidden && must_be_active)) {
			return false;
		}

		widget = widget->parent_;
	}

	return true;
}

bool widget::is_at(const point& coordinate, const bool must_be_active) const
{
	if(!recursive_is_visible(this, must_be_active)) {
		return false;
	}

	return sdl::point_in_rect(coordinate, get_rectangle());
}

} // namespace gui2
