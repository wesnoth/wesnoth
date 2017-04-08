/*
   Copyright (C) 2012 - 2016 by Fabian Mueller <fabianmueller5@gmx.de>
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

/**
 * @file
 * Manages the hotkey bindings.
 */

#include "hotkey/hotkey_item.hpp"

#include "construct_dialog.hpp"
#include "gettext.hpp"
#include "font/marked-up_text.hpp"
#include "font/standard_colors.hpp"
#include "sdl/rect.hpp"

namespace hotkey {

hotkey::hotkey_ptr show_binding_dialog(CVideo& video, const std::string& id)
{
	// Lets bind a hotkey...

	const std::string text = _("Press desired hotkey (Esc cancels)");

	SDL_Rect clip_rect = sdl::create_rect(0, 0, video.getx(), video.gety());
	SDL_Rect text_size = font::draw_text(nullptr, clip_rect, font::SIZE_LARGE,
			font::NORMAL_COLOR, text, 0, 0);

	const int centerx = video.getx() / 2;
	const int centery = video.gety() / 2;

	SDL_Rect dlgr = sdl::create_rect(centerx - text_size.w / 2 - 30,
			centery - text_size.h / 2 - 16, text_size.w + 60,
			text_size.h + 32);

	surface_restorer restorer(&video, dlgr);
	gui::dialog_frame mini_frame(video);
	mini_frame.layout(centerx - text_size.w / 2 - 20,
			centery - text_size.h / 2 - 6, text_size.w + 40,
			text_size.h + 12);
	mini_frame.draw_background();
	mini_frame.draw_border();
	font::draw_text(&video, clip_rect, font::SIZE_LARGE,
			font::NORMAL_COLOR, text,
			centerx - text_size.w / 2, centery - text_size.h / 2);
	video.flip();
	SDL_Event event = {0};
	event.type = 0;
	int keycode = -1;

	do {
		switch (event.type) {

		case SDL_KEYDOWN:
			keycode = event.key.keysym.sym;
			break;
		}

		SDL_PollEvent(&event);
		events::peek_for_resize();

	} while (!hotkey::is_hotkeyable_event(event));

	restorer.restore();
	return keycode == SDLK_ESCAPE ? hotkey::hotkey_ptr() : hotkey::create_hotkey(id, event);
}

} //end namespace hotkey
