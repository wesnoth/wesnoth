/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/vertical_scrollbar_container.hpp"

#include "foreach.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

void callback_scrollbar_button(twidget* caller)
{
	get_parent<tvertical_scrollbar_container_>(caller)->scrollbar_click(caller);
}

void callback_scrollbar(twidget* caller)
{
	get_parent<tvertical_scrollbar_container_>(caller)->scrollbar_moved(caller);
}

namespace {

	static const std::string button_up_names[] = {
		"_begin", "_line_up", "_half_page_up", "_page_up" };

	static const std::string button_down_names[] = {
		"_end", "_line_down", "_half_page_down", "_page_down" };

/**
 * Returns a map with the names of all buttons and the scrollbar jump they're
 * supposed to execute.
 */
const std::map<std::string, tscrollbar_::tscroll>& scroll_lookup()
{
	static std::map<std::string, tscrollbar_::tscroll> lookup;
	if(lookup.empty()) {
		lookup["_begin"] = tscrollbar_::BEGIN;
		lookup["_line_up"] = tscrollbar_::ITEM_BACKWARDS;
		lookup["_half_page_up"] = tscrollbar_::HALF_JUMP_BACKWARDS;
		lookup["_page_up"] = tscrollbar_::JUMP_BACKWARDS;

		lookup["_end"] = tscrollbar_::END;
		lookup["_line_down"] = tscrollbar_::ITEM_FORWARD;
		lookup["_half_page_down"] = tscrollbar_::HALF_JUMP_FORWARD;
		lookup["_page_down"] = tscrollbar_::JUMP_FORWARD;
	}

	return lookup;
}

} // namespace

tvertical_scrollbar_container_::
		tvertical_scrollbar_container_(const unsigned canvas_count)
	: tcontainer_(canvas_count)
	, scrollbar_mode_(SHOW_WHEN_NEEDED)
	, scrollbar_grid_(NULL)
	, callback_value_change_(NULL)
	, content_layout_size_(tpoint(0, 0))
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, content_last_best_size_(tpoint(0, 0))
#endif
{
}

tvertical_scrollbar_container_::~tvertical_scrollbar_container_()
{
	delete scrollbar_grid_;
}

void tvertical_scrollbar_container_::
		set_scrollbar_mode(const tscrollbar_mode scrollbar_mode)
{
	if(scrollbar_mode_ != scrollbar_mode) {
		scrollbar_mode_ = scrollbar_mode;
		show_scrollbar(scrollbar_mode_ != HIDE);
	}
}

void tvertical_scrollbar_container_::layout_init()
{
	// Inherited.
	tcontainer_::layout_init();

	content_layout_size_ = tpoint(0, 0);
}

void tvertical_scrollbar_container_::layout_wrap(const unsigned maximum_width)
{
	// Inherited.
	twidget::layout_wrap(maximum_width);

	// FIXME it might we we have omitted some borders
	const tpoint scrollbar = find_scrollbar_grid()->get_best_size();

	content_layout_wrap(maximum_width - scrollbar.x);

	set_layout_size(calculate_best_size());
}

void tvertical_scrollbar_container_::
		layout_use_vertical_scrollbar(const unsigned maximum_height)
{
	twidget::layout_use_vertical_scrollbar(maximum_height);

	log_scope2(gui_layout,
		std::string("tvertical_scrollbar_container_ ") + __func__);

	DBG_G_L << "tvertical_scrollbar_container_ maximum_height "
		<< maximum_height << ".\n";

	content_use_vertical_scrollbar(maximum_height);

	set_layout_size(calculate_best_size());
}

tpoint tvertical_scrollbar_container_::calculate_best_size() const
{
	log_scope2(gui_layout,
		std::string("tvertical_scrollbar_container_ ") + __func__);

	const tpoint content = content_get_best_size();
	if(scrollbar_mode_ == HIDE) {
		DBG_G_L << "tvertical_scrollbar_container_ result " << content << ".\n";
		return content;
	}

	const tpoint scrollbar = find_scrollbar_grid()->get_best_size();
	if(scrollbar_mode_ == SHOW) {
		// We need to show the scrollbar so the biggest height of scrollbar and
		// content is needed. The width is the sum of them.
		const tpoint result(
			content.x + scrollbar.x,
			std::max(content.y, scrollbar.y));

		DBG_G_L << "tvertical_scrollbar_container_ result " << result << ".\n";
		return result;
	}

	// When auto show the height of the content is leading. (Width again the sum.)
	const tpoint result(content.x + scrollbar.x, content.y);

	DBG_G_L << "tvertical_scrollbar_container_ result " << result << ".\n";
	return result;
}

void tvertical_scrollbar_container_::set_size(const tpoint& origin, const tpoint& size)
{
	// Inherited. -- note we don't use client size, might change
	tcontrol::set_size(origin, size);

	// Test whether we need a scrollbar.
	/**
	 * @todo the test might/will fail if the text not wrapped does fit
	 * without a scrollbar and wrapped does need it.
	 */
	bool scrollbar_needed = scrollbar_mode_ == SHOW
		|| (scrollbar_mode_ == SHOW_WHEN_NEEDED
				&& content_calculate_best_size().y > size.y);

	if(scrollbar_needed) {

		show_scrollbar(true);

		const tpoint scrollbar = find_scrollbar_grid()->get_best_size();

		tpoint tmp_origin(origin);
		tpoint tmp_size(size);
		tmp_origin.x += tmp_size.x - scrollbar.x;
		tmp_size.x = scrollbar.x;
		find_scrollbar_grid()->set_size(tmp_origin, tmp_size);


		tmp_origin = origin;
		tmp_size = size;

		tmp_size.x -= scrollbar.x;
		content_find_grid()->set_size(tmp_origin, tmp_size);
		content_set_size(create_rect(tmp_origin, tmp_size));
	} else {

		show_scrollbar(false);

		content_find_grid()->set_size(origin, size);
	 	content_set_size(create_rect(origin, size));
	}

	set_block_easy_close(get_visible() && get_active() && does_block_easy_close());
	set_scrollbar_button_status();
}

void tvertical_scrollbar_container_::key_press(tevent_handler& /*event*/,
		bool& handled, SDLKey key, SDLMod /*modifier*/, Uint16 /*unicode*/)
{
	DBG_G_E << "Listbox: key press.\n";

	tscrollbar_* sb = find_scrollbar();
	int row = get_selected_row();
	switch(key) {

		case SDLK_PAGEUP :
			row -= sb->get_visible_items() - 1;
			if(row <= 0) {
				row = 1;
			}
			// FALL DOWN

		case SDLK_UP :

			--row;
			while(row >= 0 && !get_item_active(row)) {
				--row;
			}
			if(row >= 0) {
				select_row(row);
				handled = true;

				if(static_cast<size_t>(row) < sb->get_item_position()) {
					sb->set_item_position(row);
					set_scrollbar_button_status();
				}
				value_changed();
			}
			break;

		case SDLK_PAGEDOWN :
			row += sb->get_visible_items() - 1;
			if(static_cast<size_t>(row + 1) >= sb->get_item_count()) {
				row = sb->get_item_count() - 2;
			}
			// FALL DOWN

		case SDLK_DOWN :

			++row;
			while(static_cast<size_t>(row) < sb->get_item_count()
					&& !get_item_active(row)) {

				++row;
			}
			if(static_cast<size_t>(row) < sb->get_item_count()) {
				select_row(row);
				handled = true;
				if(static_cast<size_t>(row) >= sb->get_item_position()
						+ sb->get_visible_items()) {

					sb->set_item_position(row + 1 - sb->get_visible_items());
					set_scrollbar_button_status();
				}
				value_changed();
			}
			break;

		default :
			/* DO NOTHING */
			break;
	}
}
#ifndef NEW_DRAW
void tvertical_scrollbar_container_::draw(
		surface& surface, const bool force, const bool invalidate_background)
{
	// Inherited.
	const bool do_force = force || needs_full_redraw();
	tcontainer_::draw(surface, do_force, invalidate_background);

	if(scrollbar_mode_ != HIDE) {
		draw_content(surface, do_force, invalidate_background);
	}
	draw_content(surface, do_force, invalidate_background);
}
#endif
twidget* tvertical_scrollbar_container_::find_widget(
		const tpoint& coordinate, const bool must_be_active)
{
	SDL_Rect content = content_find_grid()->get_rect();

	if(point_in_rect(coordinate.x, coordinate.y, content)) {

		return content_find_widget(tpoint(
			coordinate.x - get_x(), coordinate.y - get_y())
			, must_be_active);
	}

	// Inherited
	return tcontainer_::find_widget(coordinate, must_be_active);
}

const twidget* tvertical_scrollbar_container_::find_widget(
		const tpoint& coordinate, const bool must_be_active) const
{
	SDL_Rect content = content_find_grid()->get_rect();

	if(point_in_rect(coordinate.x, coordinate.y, content)) {

		return content_find_widget(tpoint(
			coordinate.x - get_x(), coordinate.y - get_y())
			, must_be_active);
	}

	// Inherited
	return tcontainer_::find_widget(coordinate, must_be_active);
}

bool tvertical_scrollbar_container_::does_block_easy_close() const
{
	if(scrollbar_grid_) {
		// scrollbar is hidden.
		return false;
	} else {
		// scrollbar is visible.
		return find_scrollbar()->get_active();
	}
}

void tvertical_scrollbar_container_::value_changed()
{
	if(callback_value_change_) {
		callback_value_change_(this);
	}
}

tgrid* tvertical_scrollbar_container_::find_scrollbar_grid(const bool must_exist)
{
    return scrollbar_grid_ ? scrollbar_grid_
		: find_widget<tgrid>("_scrollbar_grid", false, must_exist);
}

const tgrid* tvertical_scrollbar_container_::
		find_scrollbar_grid(const bool must_exist) const
{
    return scrollbar_grid_ ? scrollbar_grid_
    	: find_widget<tgrid>("_scrollbar_grid", false, must_exist);
}

tscrollbar_* tvertical_scrollbar_container_::find_scrollbar(const bool must_exist)
{
    return static_cast<twidget*>(find_scrollbar_grid())
		->find_widget<tscrollbar_>("_scrollbar", false, must_exist);
}

const tscrollbar_* tvertical_scrollbar_container_::find_scrollbar(
		const bool must_exist) const
{
    return static_cast<const twidget*>(find_scrollbar_grid())
    	->find_widget<const tscrollbar_>("_scrollbar", false, must_exist);
}

tgrid* tvertical_scrollbar_container_::content_find_grid(const bool must_exist)
{
    return find_widget<tgrid>("_content_grid", false, must_exist);
}

const tgrid* tvertical_scrollbar_container_::
		content_find_grid(const bool must_exist) const
{
    return find_widget<tgrid>("_content_grid", false, must_exist);
}

void tvertical_scrollbar_container_::set_scrollbar_button_status()
{
	// Set scroll up button status
	foreach(const std::string& name, button_up_names) {
		tbutton* button =
			dynamic_cast<tbutton*>(tcontainer_::find_widget(name, false));

		if(button) {
			button->set_active(!find_scrollbar()->at_begin());
		}
	}

	// Set scroll down button status
	foreach(const std::string& name, button_down_names) {
		tbutton* button =
			dynamic_cast<tbutton*>(tcontainer_::find_widget(name, false));

		if(button) {
			button->set_active(!find_scrollbar()->at_end());
		}
	}

	// Set the scrollbar itself
	find_scrollbar()->set_active(
		!(find_scrollbar()->at_begin() && find_scrollbar()->at_end()));
}

void tvertical_scrollbar_container_::show_scrollbar(const bool show)
{
	if(show && scrollbar_grid_) {

		tgrid* tmp = dynamic_cast<tgrid*>
			(grid().swap_child("_scrollbar_grid", scrollbar_grid_, true));

		delete tmp;
		scrollbar_grid_ = NULL;

	} else if(!show && !scrollbar_grid_) {

		tgrid* tmp = new tgrid();
        scrollbar_grid_ = dynamic_cast<tgrid*>
			(grid().swap_child("_scrollbar_grid", tmp, true, this));
	}
}

void tvertical_scrollbar_container_::finalize_setup()
{
	find_scrollbar()->set_callback_positioner_move(callback_scrollbar);

	// typedef boost problem work around.
	typedef std::pair<std::string, tscrollbar_::tscroll> hack;
	foreach(const hack& item, scroll_lookup()) {

		tbutton* button =
			dynamic_cast<tbutton*>(tcontainer_::find_widget(item.first, false));

		if(button) {
			button->set_callback_mouse_left_click(callback_scrollbar_button);
		}
	}

	// Make sure all mandatory widgets are tested
	find_scrollbar_grid();
	content_find_grid();

	// Set the easy close status.
	/** @todo needs more testing. */
	set_block_easy_close(get_visible() && get_active() && does_block_easy_close());

	// Call the virtual function to subclasses can do their finalization part.
	finalize();
}

void tvertical_scrollbar_container_::scrollbar_click(twidget* caller)
{
	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	const std::map<std::string, tscrollbar_::tscroll>::const_iterator
		itor = scroll_lookup().find(caller->id());

	assert(itor != scroll_lookup().end());
	find_scrollbar()->scroll(itor->second);

	set_scrollbar_button_status();
}

unsigned tvertical_scrollbar_container_::get_selected_row() const
{
	return find_scrollbar()->get_item_position();
}

tpoint tvertical_scrollbar_container_::content_get_best_size() const
{
	tpoint result = content_layout_size_;
	if(result == tpoint(0, 0)) {
		result = content_calculate_best_size();
	}

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	content_last_best_size_ = result;
#endif
	return result;
}


} // namespace gui2

