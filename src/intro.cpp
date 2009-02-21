/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file intro.cpp
 * Introduction sequence at start of a scenario, End-screen after end of
 * campaign.
 */

#include "global.hpp"
#include "foreach.hpp"
#include "intro.hpp"
#include "variable.hpp"
#include "display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "sound.hpp"
#include "game_events.hpp"
#include "language.hpp"

#define ERR_NG LOG_STREAM(err , engine)
#define LOG_NG LOG_STREAM(info, engine)

namespace {
	void scan_deprecation_messages(const config& cfg)
	{
		foreach(config const * const child, cfg.get_children("deprecated_message")) {
			if(child == NULL) {
				ERR_NG << "NULL config while searching deprecated_messages\n";
				continue;
			}
			const std::string msg = (*child)["message"];
			if(!msg.empty()) {
				lg::wml_error << msg << '\n';
			}
		}
	}
} // end unnamed namespace

static bool show_intro_part(display &disp, const vconfig& part,
		const std::string& scenario);

void show_intro(display &disp, const vconfig& data, const config& level)
{
	LOG_NG << "showing intro sequence...\n";
	scan_deprecation_messages(data.get_parsed_config());

	// Stop the screen being resized while we're in this function
	const resize_lock stop_resizing;
	const events::event_context context;

	bool showing = true;

	const std::string& scenario = level["name"];

	for(vconfig::all_children_iterator i = data.ordered_begin();
			i != data.ordered_end() && showing; i++) {
		std::pair<const std::string, const vconfig> item = *i;

		if(item.first == "part") {
			showing = show_intro_part(disp, item.second, scenario);
		} else if(item.first == "if") {
			const std::string type = game_events::conditional_passed(
				NULL, item.second) ? "then":"else";
			const vconfig selection = item.second.child(type);
			if(selection.empty()) {
				LOG_NG << "no intro story this way...\n";
				return;
			}
			show_intro(disp, selection, level);
		}
	}

	LOG_NG << "intro sequence finished...\n";
}

/**
 * show_intro_part() is split into two parts, the second part can cause
 * an utils::invalid_utf8_exception exception and it's to much code
 * to indent. The solution is not very clean but the entire routine could
 * use a cleanup.
 */
static bool show_intro_part_helper(display &disp, const vconfig& part,
		int textx, int texty,
		bool has_background,
		bool redraw_all,
		gui::button& next_button, gui::button& skip_button,
		CKey& key);

bool show_intro_part(display &disp, const vconfig& part,
		const std::string& scenario)
{
	LOG_NG << "showing intro part\n";
	scan_deprecation_messages(part.get_parsed_config());

	CVideo &video = disp.video();
	const std::string music_file = part["music"];

	// Play music if available
	if(music_file != "") {
		sound::play_music_repeatedly(music_file);
	}

	CKey key;

	gui::button next_button(video,_("Next") + std::string(">>>"));
	gui::button skip_button(video,_("Skip"));

	draw_solid_tinted_rectangle(0,0,video.getx(),video.gety(),
			0,0,0,1.0,video.getSurface());


	const std::string background_name = part["background"];
	const bool show_title = utils::string_bool(part["show_title"]);
	const bool scale_background = utils::string_bool(part["scale_background"], true);

	surface background(NULL);
	if(background_name.empty() == false) {
		background.assign(image::get_image(background_name));
	}
	const bool has_background = !background.null();

	int textx = 200;
	int texty = 400;

	SDL_Rect dstrect;

	if(background.null() || background->w*background->h == 0) {
		background.assign(create_neutral_surface(video.getx(),video.gety()));
	}

	const double xscale = 1.0 * video.getx() / background->w;
	const double yscale = 1.0 * video.gety() / background->h;
	const double scale = scale_background ? std::min<double>(xscale,yscale) : 1.0;

	background = scale_surface(background, static_cast<int>(background->w*scale), static_cast<int>(background->h*scale));
	assert(background.null() == false);

	dstrect.x = (video.getx() - background->w) / 2;
	dstrect.y = (video.gety() - background->h) / 2;
	dstrect.w = background->w;
	dstrect.h = background->h;

	SDL_BlitSurface(background,NULL,video.getSurface(),&dstrect);

#ifdef USE_TINY_GUI
	textx = 10;
	int xbuttons = video.getx() - 50;
	int ybuttons = dstrect.y + dstrect.h - 20;

	// Use the whole screen for text
	texty = 0;

	next_button.set_location(xbuttons,ybuttons-20);
	skip_button.set_location(xbuttons,ybuttons);
#else
	int xbuttons, ybuttons;
	textx = 200;
	xbuttons = video.getx() - 200 - 40;
	texty = video.gety() - 200;
	ybuttons = video.gety() - 40;

	next_button.set_location(xbuttons,ybuttons-30);
	skip_button.set_location(xbuttons,ybuttons);
#endif

	// Draw title if needed
	if(show_title) {
		const SDL_Rect area = {0,0,video.getx(),video.gety()};
		const SDL_Rect txt_shadow_rect = font::line_size(scenario, font::SIZE_XLARGE);
		draw_solid_tinted_rectangle(dstrect.x + 15,dstrect.y + 15,txt_shadow_rect.w + 10,txt_shadow_rect.h + 10,0,0,0,0.5,video.getSurface());
		update_rect(font::draw_text(&video,area,font::SIZE_XLARGE,font::BIGMAP_COLOUR,scenario,
					    dstrect.x + 20,dstrect.y + 20));
	}

	const vconfig::child_list images = part.get_children("image");

	if(!images.empty()) {
		// Redraw all
		events::raise_draw_event();
		update_whole_screen();

		// Draw images
		bool pass = false;

		for(vconfig::child_list::const_iterator i = images.begin(); i != images.end(); ++i){
			const std::string image_name = (*i)["file"];
			if(image_name == "") continue;
			surface img(image::get_image(image_name));
			if(img.null()) continue;

			const int delay = lexical_cast_default<int>((*i)["delay"], 0);
			const int x = static_cast<int>(atoi((*i)["x"].c_str())*scale);
			const int y = static_cast<int>(atoi((*i)["y"].c_str())*scale);

			if (utils::string_bool((*i)["scaled"])){
				img = scale_surface(img, static_cast<int>(img->w*scale), static_cast<int>(img->h*scale));
			}

			SDL_Rect image_rect;
			image_rect.x = x + dstrect.x;
			image_rect.y = y + dstrect.y;
			image_rect.w = img->w;
			image_rect.h = img->h;

			if (utils::string_bool((*i)["centered"])){
				image_rect.x -= image_rect.w/2;
				image_rect.y -= image_rect.h/2;
			}

			SDL_BlitSurface(img,NULL,video.getSurface(),&image_rect);

			update_rect(image_rect);

			if(pass == false) {
				for(int i = 0; i != 50; ++i) {
					if(key[SDLK_ESCAPE] || next_button.pressed() || skip_button.pressed()) {
						return false;
					}

					disp.delay(delay/50);

					events::pump();
					events::raise_process_event();
					events::raise_draw_event();

					int a, b;
					const int mouse_flags = SDL_GetMouseState(&a,&b);
					if(key[SDLK_RETURN] || key[SDLK_KP_ENTER] || key[SDLK_SPACE] || mouse_flags) {
						pass = true;
						continue;
					}

					// Update display only if there's a slideshow going on.
					// This prevents the textbox from flickering in the most
					// common scenario.
					if(images.size() > 1 && delay > 0) {
						disp.flip();
					}
				}
			}

			if(key[SDLK_ESCAPE] || next_button.pressed() || skip_button.pressed()) {
				pass = true;
				continue;
			}
		}
	}
	try {
		return show_intro_part_helper(
			disp, part, textx, texty,
			has_background,
			images.empty(),
			next_button, skip_button, key
		);

	} catch (utils::invalid_utf8_exception&) {
		LOG_STREAM(err, engine) << "Invalid utf-8 found, story message is ignored.\n";
		// stop showing on an error, there might be more badly formed utf-8 messages
		return false;
	}
}

#ifdef LOW_MEM
static void blur_helper(CVideo&,int,int)
{}
#else
static void blur_helper(CVideo& video, int y, int h)
{
	SDL_Rect blur_rect = { 0, y, screen_area().w, h };
	surface blur = get_surface_portion(video.getSurface(), blur_rect);
	blur = blur_surface(blur, 1, false);
	video.blit_surface(0, y, blur);
}
#endif

static bool show_intro_part_helper(display &disp, const vconfig& part,
		int textx, int texty,
		bool has_background,
		bool redraw_all,
		gui::button& next_button, gui::button& skip_button,
		CKey& key)
{
	bool lang_rtl = current_language_rtl();
	CVideo &video = disp.video();

	const int max_width = next_button.location().x - 10 - textx;
	const std::string story =
		font::word_wrap_text(part["story"], font::SIZE_PLUS, max_width);

	utils::utf8_iterator itor(story);

	bool skip = false, last_key = true;
	int update_y = 0, update_h = 0;

	// Draw the text box
	if(story.empty() != true)
	{
		// this should kill the tiniest flickering caused
		// by the buttons being hidden and unhidden in this scope.
		update_locker locker(disp.video());

		const SDL_Rect total_size = font::draw_text(NULL, screen_area(), font::SIZE_PLUS,
				font::NORMAL_COLOUR, story, 0, 0);

		next_button.hide();
		skip_button.hide();

		if (texty + 20 + total_size.h > screen_area().h) {
			texty = screen_area().h > total_size.h + 1 ? screen_area().h - total_size.h - 21 : 0;
		}

		update_y = texty;
		update_h = screen_area().h-texty;
		blur_helper(disp.video(), update_y, update_h);

		draw_solid_tinted_rectangle(
			0, texty, screen_area().w, screen_area().h - texty,
			0, 0, 0, 0.5, video.getSurface()
		);

		// Draw a nice border
		if(has_background) {
			// FIXME: perhaps hard-coding the image path isn't a really
			// good idea - it must not be forgotten if someone decides to switch
			// the image directories around.
			surface top_border = image::get_image("dialogs/translucent54-border-top.png");
			top_border = scale_surface_blended(top_border, screen_area().w, top_border->h);
			update_y = texty - top_border->h;
			update_h += top_border->h;
			blur_helper(disp.video(), update_y, top_border->h);
			disp.video().blit_surface(0, texty - top_border->h, top_border);
		}

		// Make buttons aware of the changes in the background
		next_button.set_location(next_button.location());
		next_button.hide(false);
		skip_button.set_location(skip_button.location());
		skip_button.hide(false);
	}

	if(redraw_all) {
		update_whole_screen();
	} else if(update_h > 0) {
		update_rect(0,update_y,screen_area().w,update_h);
	}

	if(lang_rtl)
		textx += max_width;

#ifdef USE_TINY_GUI
	int xpos = textx, ypos = texty + 10;
#else
	int xpos = textx, ypos = texty + 20;
#endif

	// The maximum position that text can reach before wrapping
	size_t height = 0;

	for(;;) {
		if(itor != utils::utf8_iterator::end(story)) {
			if(*itor == '\n') {
				xpos = textx;
				ypos += height;
				++itor;
			}

			// Output the character
			/** @todo  FIXME: this is broken: it does not take kerning into account. */
			std::string tmp;
			tmp.append(itor.substr().first, itor.substr().second);
			if(lang_rtl)
				xpos -= font::line_width(tmp, font::SIZE_PLUS);
			const SDL_Rect rect = font::draw_text(&video,
					screen_area(),font::SIZE_PLUS,
					font::NORMAL_COLOUR,tmp,xpos,ypos,
					false);

			if(rect.h > height)
				height = rect.h;
			if(!lang_rtl)
				xpos += rect.w;
			update_rect(rect);

			++itor;
			if(itor == utils::utf8_iterator::end(story))
				skip = true;

		}

		const bool keydown = key[SDLK_SPACE] || key[SDLK_RETURN] || key[SDLK_KP_ENTER];

		if((keydown && !last_key) || next_button.pressed()) {
			if(skip == true || itor == utils::utf8_iterator::end(story)) {
				break;
			} else {
				skip = true;
			}
		}

		last_key = keydown;

		if(key[SDLK_ESCAPE] || skip_button.pressed())
			return false;

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		disp.flip();

		if(!skip || itor == utils::utf8_iterator::end(story))
			disp.delay(20);
	}

	draw_solid_tinted_rectangle(0,0,video.getx(),video.gety(),0,0,0,1.0,
                                     video.getSurface());

	return true;
}

void the_end(display &disp, std::string text, unsigned int duration)
{
	//
	// Some sane defaults.
	//
	if(text.empty())
		text = _("The End");
	if(!duration)
		duration = 3500;

	SDL_Rect area = screen_area();
	CVideo &video = disp.video();
	SDL_FillRect(video.getSurface(),&area,0);

	update_whole_screen();
	disp.flip();

	const size_t font_size = font::SIZE_XLARGE;

	area = font::text_area(text,font_size);
	area.x = screen_area().w/2 - area.w/2;
	area.y = screen_area().h/2 - area.h/2;

	for(size_t n = 0; n < 255; n += 5) {
		if(n)
			SDL_FillRect(video.getSurface(),&area,0);

		const SDL_Color col = {n,n,n,n};
		font::draw_text(&video,area,font_size,col,text,area.x,area.y);
		update_rect(area);

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		disp.flip();
		disp.delay(10);
	}

	//
	// Delay after the end of fading.
	// Rounded to multiples of 10.
	//
	unsigned int count = duration/10;
	while(count) {
		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		disp.flip();
		disp.delay(10);
		--count;
	}
}
