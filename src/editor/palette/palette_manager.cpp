/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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

#include "palette_manager.hpp"
#include "widgets/widget.hpp"

#include "tooltips.hpp"
#include "editor/action/mouse/mouse_action.hpp"

namespace editor {

palette_manager::palette_manager(editor_display& gui, const config& cfg
		, mouse_action** active_mouse_action)
		: gui::widget(gui.video()),
		  gui_(gui),
		  palette_start_(0),
		  mouse_action_(active_mouse_action),
		  terrain_palette_(new terrain_palette(gui, cfg, active_mouse_action)),
		  unit_palette_(new unit_palette(gui, cfg, active_mouse_action)),
		  empty_palette_(new empty_palette(gui))
{
	unit_palette_->setup(cfg);
	terrain_palette_->setup(cfg);
}

void palette_manager::set_group(size_t index)
{
	active_palette().set_group(index);
	scroll_top();
}

void palette_manager::adjust_size()
{
	scroll_top();
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

		const SDL_Rect& rect = gui_.palette_area();
		bg_restore(rect);
		set_dirty();
	}
}

void palette_manager::scroll_up()
{
	bool scrolled_up = active_palette().scroll_up();
	if(scrolled_up) {
		const SDL_Rect rect = gui_.palette_area();
		bg_restore(rect);
		set_dirty();
	}
}

void palette_manager::scroll_top()
{
	const SDL_Rect rect = gui_.palette_area();
	active_palette().set_start_item(0);
	bg_restore(rect);
	set_dirty();
}

common_palette& palette_manager::active_palette()
{
	return (*mouse_action_)->get_palette();
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

void palette_manager::draw(bool force)
{
	if (!dirty() && !force) {
		return;
	}

	const SDL_Rect &loc = location();

	tooltips::clear_tooltips(loc);

	gui::button* upscroll_button = gui_.find_button("upscroll-button-editor");
	upscroll_button->hide(false);
	gui::button* downscroll_button = gui_.find_button("downscroll-button-editor");
	downscroll_button->hide(false);
	gui::button* palette_menu_button = gui_.find_button("menu-editor-terrain");
	palette_menu_button->hide(false);

	active_palette().draw(force);

	update_rect(loc);
	set_dirty(false);
}

void palette_manager::handle_event(const SDL_Event& event) {

	if (event.type == SDL_MOUSEMOTION) {
		// If the mouse is inside the palette, give it focus.
		if (point_in_rect(event.button.x, event.button.y, location())) {
			if (!focus(&event)) {
				set_focus(true);
			}
		}
		// If the mouse is outside, remove focus.
		else {
			if (focus(&event)) {
				set_focus(false);
			}
		}
	}
	if (!focus(&event)) {
		return;
	}
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const SDL_MouseButtonEvent mouse_button_event = event.button;
	if (mouse_button_event.type == SDL_MOUSEBUTTONDOWN) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
			left_mouse_click(mousex, mousey);
		}
		/* TODO
		if (mouse_button_event.button == SDL_BUTTON_MIDDLE) {
			middle_mouse_click(mousex, mousey);
		}
		*/
		if (mouse_button_event.button == SDL_BUTTON_RIGHT) {
			right_mouse_click(mousex, mousey);
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELUP) {
			scroll_up();
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELDOWN) {
			scroll_down();
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELLEFT) {
			active_palette().prev_group();
			scroll_top();
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELRIGHT) {
			active_palette().next_group();
			scroll_top();
		}
	}
	if (mouse_button_event.type == SDL_MOUSEBUTTONUP) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
			//TODO What is missing here?
		}
	}
}

void palette_manager::left_mouse_click(const int mousex, const int mousey)
{
	if ( active_palette().left_mouse_click(mousex, mousey) ) {
		set_dirty();
		gui_.invalidate_game_status();
	}
}

void palette_manager::right_mouse_click(const int mousex, const int mousey)
{
	if ( active_palette().right_mouse_click(mousex, mousey) ) {
		set_dirty();
		gui_.invalidate_game_status();
	}
}


} //Namespace editor

