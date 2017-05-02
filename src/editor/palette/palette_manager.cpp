/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/palette/palette_manager.hpp"
#include "widgets/widget.hpp"

#include "tooltips.hpp"
#include "editor/toolkit/editor_toolkit.hpp"

namespace editor {

palette_manager::palette_manager(editor_display& gui, const config& cfg
                               , editor_toolkit& toolkit)
		: gui::widget(gui.video()),
		  gui_(gui),
		  palette_start_(0),
		  toolkit_(toolkit),
		  terrain_palette_(new terrain_palette(gui, cfg, toolkit)),
		  unit_palette_(new unit_palette(gui, cfg, toolkit)),
		  empty_palette_(new empty_palette(gui)),
		  item_palette_(new item_palette(gui, cfg, toolkit))
		, location_palette_(new location_palette(gui, cfg, toolkit))
{
	unit_palette_->setup(cfg);
	terrain_palette_->setup(cfg);
	item_palette_->setup(cfg);
}

void palette_manager::set_group(size_t index)
{
	active_palette().set_group(index);
	scroll_top();
}

void palette_manager::adjust_size()
{
	restore_palette_bg(false);
	const SDL_Rect& rect = gui_.palette_area();
	set_location(rect);
	palette_start_ = rect.y;
	bg_register(rect);
	active_palette().adjust_size(rect);
	set_dirty();
}

void palette_manager::scroll_down()
{
	bool scrolled = active_palette().scroll_down();

	if (scrolled) {

//		const SDL_Rect& rect = gui_.palette_area();
//		bg_restore(rect);
		set_dirty();
		draw();
	}
}

bool palette_manager::can_scroll_up()
{
	return active_palette().can_scroll_up();
}

bool palette_manager::can_scroll_down()
{
	return active_palette().can_scroll_down();
}

void palette_manager::scroll_up()
{
	bool scrolled_up = active_palette().scroll_up();
	if(scrolled_up) {
//		const SDL_Rect rect = gui_.palette_area();
//		bg_restore(rect);
		set_dirty();
		draw();
	}
}

void palette_manager::scroll_top()
{
	restore_palette_bg(true);
}

void palette_manager::restore_palette_bg(bool scroll_top)
{
	const SDL_Rect rect = gui_.palette_area();
	if (scroll_top) {
		active_palette().set_start_item(0);
	}
	bg_restore(rect);
	set_dirty();
}

common_palette& palette_manager::active_palette()
{
	return toolkit_.get_palette();
}

void palette_manager::scroll_bottom()
{
	unsigned int old_start   = active_palette().num_items();
	unsigned int items_start = active_palette().start_num();
	while (old_start != items_start) {
		old_start = items_start;
		scroll_down();
	}
}

void palette_manager::draw_contents()
{
	//if (!dirty() && !force) {
	//	return;
	//}

	const SDL_Rect &loc = location();

	tooltips::clear_tooltips(loc);

	std::shared_ptr<gui::button> upscroll_button = gui_.find_action_button("upscroll-button-editor");
	if (upscroll_button)
		upscroll_button->hide(false);
	std::shared_ptr<gui::button> downscroll_button = gui_.find_action_button("downscroll-button-editor");
	if (downscroll_button)
		downscroll_button->hide(false);
	std::shared_ptr<gui::button> palette_menu_button = gui_.find_menu_button("menu-editor-terrain");
	if (palette_menu_button)
		palette_menu_button->hide(false);

//	bg_restore(loc);
	active_palette().set_dirty(true);
	active_palette().hide(false);
	active_palette().draw();

//	set_dirty(false);
}

sdl_handler_vector palette_manager::handler_members()
{
	//handler_vector h;
//	for (gui::widget& b : active_palette().get_widgets()) {
//		h.push_back(&b);
//	}
	//return h;
	return active_palette().handler_members();
}

void palette_manager::handle_event(const SDL_Event& event) {

	gui::widget::handle_event(event);

	if (event.type == SDL_MOUSEMOTION) {
		// If the mouse is inside the palette, give it focus.
		if (sdl::point_in_rect(event.button.x, event.button.y, location())) {
			if (!focus(&event)) set_focus(true);
		}
		// If the mouse is outside, remove focus.
		else if (focus(&event)) set_focus(false);
	}
	if (!focus(&event)) {
		return;
	}

	const SDL_MouseButtonEvent &mouse_button_event = event.button;


	if (event.type == SDL_MOUSEWHEEL) {
		if (event.wheel.y > 0) {
			scroll_up();
		} else if (event.wheel.y < 0) {
			scroll_down();
		}

		if (event.wheel.x < 0) {
			active_palette().prev_group();
			scroll_top();
		} else if (event.wheel.x > 0) {
			active_palette().next_group();
			scroll_top();
		}
	}

	if (mouse_button_event.type == SDL_MOUSEBUTTONUP) {
		//set_dirty(true);
//		draw(true);
//		set_dirty(active_palette().mouse_click());
//		gui_.invalidate_game_status();
	}
}


} //Namespace editor
