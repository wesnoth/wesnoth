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

#include "help/help_text_area.hpp"

#include "config.hpp"                   // for config, etc
#include "game_config.hpp"              // for debug
#include "help/help_impl.hpp"           // for parse_error, box_width, etc
#include "image.hpp"                    // for get_image
#include "log.hpp"                      // for LOG_STREAM, log_domain, etc
#include "preferences/general.hpp"              // for font_scaled
#include "sdl/rect.hpp"                 // for draw_rectangle, etc
#include "serialization/parser.hpp"     // for read, write
#include "video.hpp"                    // for CVideo

#include <algorithm>                    // for max, min, find_if
#include <ostream>                      // for operator<<, stringstream, etc
#include <vector>                       // for vector, etc

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

namespace help {

help_text_area::help_text_area(CVideo &video, const section &toplevel) :
	gui::scrollarea(video),
	items_(),
	last_row_(),
	toplevel_(toplevel),
	shown_topic_(nullptr),
	title_spacing_(16),
	curr_loc_(0, 0),
	min_row_height_(font::get_max_height(normal_font_size)),
	curr_row_height_(min_row_height_),
	contents_height_(0)
{
	set_scroll_rate(40);
}

void help_text_area::set_inner_location(SDL_Rect const &rect)
{
	bg_register(rect);
	if (shown_topic_)
		set_items();
}

void help_text_area::show_topic(const topic &t)
{
	shown_topic_ = &t;
	set_items();
	set_dirty(true);
}


help_text_area::item::item(surface surface, int x, int y, const std::string& _text,
						   const std::string& reference_to, bool _floating,
						   bool _box, ALIGNMENT alignment) :
	rect(),
	surf(surface),
	text(_text),
	ref_to(reference_to),
	floating(_floating), box(_box),
	align(alignment)
{
	rect.x = x;
	rect.y = y;
	rect.w = box ? surface->w + box_width * 2 : surface->w;
	rect.h = box ? surface->h + box_width * 2 : surface->h;
}

help_text_area::item::item(surface surface, int x, int y, bool _floating,
						   bool _box, ALIGNMENT alignment) :
	rect(),
	surf(surface),
	text(""),
	ref_to(""),
	floating(_floating),
	box(_box), align(alignment)
{
	rect.x = x;
	rect.y = y;
	rect.w = box ? surface->w + box_width * 2 : surface->w;
	rect.h = box ? surface->h + box_width * 2 : surface->h;
}

void help_text_area::set_items()
{
	last_row_.clear();
	items_.clear();
	curr_loc_.first = 0;
	curr_loc_.second = 0;
	curr_row_height_ = min_row_height_;
	// Add the title item.
	const std::string show_title =
		font::make_text_ellipsis(shown_topic_->title, title_size, inner_location().w);
	surface surf(font::get_rendered_text(show_title, title_size,
					     font::NORMAL_COLOR, TTF_STYLE_BOLD));
	if (surf != nullptr) {
		add_item(item(surf, 0, 0, show_title));
		curr_loc_.second = title_spacing_;
		contents_height_ = title_spacing_;
		down_one_line();
	}
	// Parse and add the text.
	std::vector<std::string> const &parsed_items = shown_topic_->text.parsed_text();
	std::vector<std::string>::const_iterator it;
	for (it = parsed_items.begin(); it != parsed_items.end(); ++it) {
		if (!(*it).empty() && (*it)[0] == '[') {
			// Should be parsed as WML.
			try {
				config cfg;
				std::istringstream stream(*it);
				read(cfg, stream);

#define TRY(name) do { \
				if (config &child = cfg.child(#name)) \
					handle_##name##_cfg(child); \
				} while (0)

				TRY(ref);
				TRY(img);
				TRY(bold);
				TRY(italic);
				TRY(header);
				TRY(jump);
				TRY(format);

#undef TRY

			}
			catch (config::error& e) {
				std::stringstream msg;
				msg << "Error when parsing help markup as WML: '" << e.message << "'";
				throw parse_error(msg.str());
			}
		}
		else {
			add_text_item(*it);
		}
	}
	down_one_line(); // End the last line.
	int h = height();
	set_position(0);
	set_full_size(contents_height_);
	set_shown_size(h);
}

void help_text_area::handle_ref_cfg(const config &cfg)
{
	const std::string dst = cfg["dst"];
	const std::string text = cfg["text"];
	bool force = cfg["force"].to_bool();

	if (dst.empty()) {
		std::stringstream msg;
		msg << "Ref markup must have dst attribute. Please submit a bug"
		       " report if you have not modified the game files yourself. Erroneous config: ";
		write(msg, cfg);
		throw parse_error(msg.str());
	}

	if (find_topic(toplevel_, dst) == nullptr && !force) {
		// detect the broken link but quietly silence the hyperlink for normal user
		add_text_item(text, game_config::debug ? dst : "", true);

		// FIXME: workaround: if different campaigns define different
		// terrains, some terrains available in one campaign will
		// appear in the list of seen terrains, and be displayed in the
		// help, even if the current campaign does not handle such
		// terrains. This will lead to the unit page generator creating
		// invalid references.
		//
		// Disabling this is a kludgey workaround until the
		// encountered_terrains system is fixed
		//
		// -- Ayin apr 8 2005
#if 0
		if (game_config::debug) {
			std::stringstream msg;
			msg << "Reference to non-existent topic '" << dst
			    << "'. Please submit a bug report if you have not"
			       "modified the game files yourself. Erroneous config: ";
			write(msg, cfg);
			throw parse_error(msg.str());
		}
#endif
	} else {
		add_text_item(text, dst);
	}
}

void help_text_area::handle_img_cfg(const config &cfg)
{
	const std::string src = cfg["src"];
	const std::string align = cfg["align"];
	bool floating = cfg["float"].to_bool();
	bool box = cfg["box"].to_bool(true);
	if (src.empty()) {
		throw parse_error("Img markup must have src attribute.");
	}
	add_img_item(src, align, floating, box);
}

void help_text_area::handle_bold_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text.empty()) {
		throw parse_error("Bold markup must have text attribute.");
	}
	add_text_item(text, "", false, -1, true);
}

void help_text_area::handle_italic_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text.empty()) {
		throw parse_error("Italic markup must have text attribute.");
	}
	add_text_item(text, "", false, -1, false, true);
}

void help_text_area::handle_header_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text.empty()) {
		throw parse_error("Header markup must have text attribute.");
	}
	add_text_item(text, "", false, title2_size, true);
}

void help_text_area::handle_jump_cfg(const config &cfg)
{
	const std::string amount_str = cfg["amount"];
	const std::string to_str = cfg["to"];
	if (amount_str.empty() && to_str.empty()) {
		throw parse_error("Jump markup must have either a to or an amount attribute.");
	}
	unsigned jump_to = curr_loc_.first;
	if (!amount_str.empty()) {
		unsigned amount;
		try {
			amount = lexical_cast<unsigned, std::string>(amount_str);
		}
		catch (bad_lexical_cast) {
			throw parse_error("Invalid amount the amount attribute in jump markup.");
		}
		jump_to += amount;
	}
	if (!to_str.empty()) {
		unsigned to;
		try {
			to = lexical_cast<unsigned, std::string>(to_str);
		}
		catch (bad_lexical_cast) {
			throw parse_error("Invalid amount in the to attribute in jump markup.");
		}
		if (to < jump_to) {
			down_one_line();
		}
		jump_to = to;
	}
	if (jump_to != 0 && static_cast<int>(jump_to) <
            get_max_x(curr_loc_.first, curr_row_height_)) {

		curr_loc_.first = jump_to;
	}
}

void help_text_area::handle_format_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text.empty()) {
		throw parse_error("Format markup must have text attribute.");
	}
	bool bold = cfg["bold"].to_bool();
	bool italic = cfg["italic"].to_bool();
	int font_size = cfg["font_size"].to_int(normal_font_size);
	color_t color = help::string_to_color(cfg["color"]);
	add_text_item(text, "", false, font_size, bold, italic, color);
}

void help_text_area::add_text_item(const std::string& text, const std::string& ref_dst,
								   bool broken_link, int _font_size, bool bold, bool italic,
								   color_t text_color
)
{
	const int font_size = _font_size < 0 ? normal_font_size : _font_size;
	// font::line_width(), font::get_rendered_text() are not use scaled font inside
	const int scaled_font_size = preferences::font_scaled(font_size);
	if (text.empty())
		return;
	const int remaining_width = get_remaining_width();
	size_t first_word_start = text.find_first_not_of(" ");
	if (first_word_start == std::string::npos) {
		first_word_start = 0;
	}
	if (text[first_word_start] == '\n') {
		down_one_line();
		std::string rest_text = text;
		rest_text.erase(0, first_word_start + 1);
		add_text_item(rest_text, ref_dst, broken_link, _font_size, bold, italic, text_color);
		return;
	}
	const std::string first_word = get_first_word(text);
	int state = 0;
	state |= bold ? TTF_STYLE_BOLD : 0;
	state |= italic ? TTF_STYLE_ITALIC : 0;
	if (curr_loc_.first != get_min_x(curr_loc_.second, curr_row_height_)
		&& remaining_width < font::line_width(first_word, scaled_font_size, state)) {
		// The first word does not fit, and we are not at the start of
		// the line. Move down.
		down_one_line();
		std::string s = remove_first_space(text);
		add_text_item(s, ref_dst, broken_link, _font_size, bold, italic, text_color);
	}
	else {
		std::vector<std::string> parts = split_in_width(text, font_size, remaining_width);
		std::string first_part = parts.front();
		// Always override the color if we have a cross reference.
		color_t color;
		if(ref_dst.empty())
			color = text_color;
		else if(broken_link)
			color = font::BAD_COLOR;
		else
			color = font::YELLOW_COLOR;

		// In split_in_width(), no_break_after() and no_break_before() are used(see marked-up_text.cpp).
		// Thus, even if there is enough remaining_width for the next word,
		// sometimes empty string is returned from split_in_width().
		if (first_part.empty()) {
			down_one_line();
		}
		else {
			surface surf(font::get_rendered_text(first_part, scaled_font_size, color, state));
			if (!surf.null())
				add_item(item(surf, curr_loc_.first, curr_loc_.second, first_part, ref_dst));
		}
		if (parts.size() > 1) {

			std::string& s = parts.back();

			const std::string first_word_before = get_first_word(s);
			const std::string first_word_after = get_first_word(remove_first_space(s));
			if (get_remaining_width() >= font::line_width(first_word_after, scaled_font_size, state)
				&& get_remaining_width()
				< font::line_width(first_word_before, scaled_font_size, state)) {
				// If the removal of the space made this word fit, we
				// must move down a line, otherwise it will be drawn
				// without a space at the end of the line.
				s = remove_first_space(s);
				down_one_line();
			}
			else if (!(font::line_width(first_word_before, scaled_font_size, state)
					   < get_remaining_width())) {
				s = remove_first_space(s);
			}
			add_text_item(s, ref_dst, broken_link, _font_size, bold, italic, text_color);

		}
	}
}

void help_text_area::add_img_item(const std::string& path, const std::string& alignment,
								  const bool floating, const bool box)
{
	surface surf(image::get_image(path));
	if (surf.null())
		return;
	ALIGNMENT align = str_to_align(alignment);
	if (align == HERE && floating) {
		WRN_DP << "Floating image with align HERE, aligning left." << std::endl;
		align = LEFT;
	}
	const int width = surf->w + (box ? box_width * 2 : 0);
	int xpos;
	int ypos = curr_loc_.second;
	int text_width = inner_location().w;
	switch (align) {
	case HERE:
		xpos = curr_loc_.first;
		break;
	case LEFT:
	default:
		xpos = 0;
		break;
	case MIDDLE:
		xpos = text_width / 2 - width / 2 - (box ? box_width : 0);
		break;
	case RIGHT:
		xpos = text_width - width - (box ? box_width * 2 : 0);
		break;
	}
	if (curr_loc_.first != get_min_x(curr_loc_.second, curr_row_height_)
		&& (xpos < curr_loc_.first || xpos + width > text_width)) {
		down_one_line();
		add_img_item(path, alignment, floating, box);
	}
	else {
		if (!floating) {
			curr_loc_.first = xpos;
		}
		else {
			ypos = get_y_for_floating_img(width, xpos, ypos);
		}
		add_item(item(surf, xpos, ypos, floating, box, align));
	}
}

int help_text_area::get_y_for_floating_img(const int width, const int x, const int desired_y)
{
	int min_y = desired_y;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); ++it) {
		const item& itm = *it;
		if (itm.floating) {
			if ((itm.rect.x + itm.rect.w > x && itm.rect.x < x + width)
				|| (itm.rect.x > x && itm.rect.x < x + width)) {
				min_y = std::max<int>(min_y, itm.rect.y + itm.rect.h);
			}
		}
	}
	return min_y;
}

int help_text_area::get_min_x(const int y, const int height)
{
	int min_x = 0;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); ++it) {
		const item& itm = *it;
		if (itm.floating) {
			if (itm.rect.y < y + height && itm.rect.y + itm.rect.h > y && itm.align == LEFT) {
				min_x = std::max<int>(min_x, itm.rect.w + 5);
			}
		}
	}
	return min_x;
}

int help_text_area::get_max_x(const int y, const int height)
{
	int text_width = inner_location().w;
	int max_x = text_width;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); ++it) {
		const item& itm = *it;
		if (itm.floating) {
			if (itm.rect.y < y + height && itm.rect.y + itm.rect.h > y) {
				if (itm.align == RIGHT) {
					max_x = std::min<int>(max_x, text_width - itm.rect.w - 5);
				} else if (itm.align == MIDDLE) {
					max_x = std::min<int>(max_x, text_width / 2 - itm.rect.w / 2 - 5);
				}
			}
		}
	}
	return max_x;
}

void help_text_area::add_item(const item &itm)
{
	items_.push_back(itm);
	if (!itm.floating) {
		curr_loc_.first += itm.rect.w;
		curr_row_height_ = std::max<int>(itm.rect.h, curr_row_height_);
		contents_height_ = std::max<int>(contents_height_, curr_loc_.second + curr_row_height_);
		last_row_.push_back(&items_.back());
	}
	else {
		if (itm.align == LEFT) {
			curr_loc_.first = itm.rect.w + 5;
		}
		contents_height_ = std::max<int>(contents_height_, itm.rect.y + itm.rect.h);
	}
}


help_text_area::ALIGNMENT help_text_area::str_to_align(const std::string &cmp_str)
{
	if (cmp_str == "left") {
		return LEFT;
	} else if (cmp_str == "middle") {
		return MIDDLE;
	} else if (cmp_str == "right") {
		return RIGHT;
	} else if (cmp_str == "here" || cmp_str.empty()) { // Make the empty string be "here" alignment.
		return HERE;
	}
	std::stringstream msg;
	msg << "Invalid alignment string: '" << cmp_str << "'";
	throw parse_error(msg.str());
}

void help_text_area::down_one_line()
{
	adjust_last_row();
	last_row_.clear();
	curr_loc_.second += curr_row_height_ + (curr_row_height_ == min_row_height_ ? 0 : 2);
	curr_row_height_ = min_row_height_;
	contents_height_ = std::max<int>(curr_loc_.second + curr_row_height_, contents_height_);
	curr_loc_.first = get_min_x(curr_loc_.second, curr_row_height_);
}

void help_text_area::adjust_last_row()
{
	for (std::list<item *>::iterator it = last_row_.begin(); it != last_row_.end(); ++it) {
		item &itm = *(*it);
		const int gap = curr_row_height_ - itm.rect.h;
		itm.rect.y += gap / 2;
	}
}

int help_text_area::get_remaining_width()
{
	const int total_w = get_max_x(curr_loc_.second, curr_row_height_);
	return total_w - curr_loc_.first;
}

void help_text_area::draw_contents()
{
	SDL_Rect const &loc = inner_location();
	bg_restore();
	surface& screen = video().getSurface();
	clip_rect_setter clip_rect_set(screen, &loc);
	for(std::list<item>::const_iterator it = items_.begin(), end = items_.end(); it != end; ++it) {
		SDL_Rect dst = it->rect;
		dst.y -= get_position();
		if (dst.y < static_cast<int>(loc.h) && dst.y + it->rect.h > 0) {
			dst.x += loc.x;
			dst.y += loc.y;
			if (it->box) {
				for (int i = 0; i < box_width; ++i) {
					SDL_Rect draw_rect {
						dst.x,
						dst.y,
						it->rect.w - i * 2,
						it->rect.h - i * 2
					};

					sdl::draw_rectangle(draw_rect, {0, 0, 0, 0});
					++dst.x;
					++dst.y;
				}
			}
			sdl_blit(it->surf, nullptr, screen, &dst);
		}
	}
}

void help_text_area::scroll(unsigned int)
{
	// Nothing will be done on the actual scroll event. The scroll
	// position is checked when drawing instead and things drawn
	// accordingly.
	set_dirty(true);
}

bool help_text_area::item_at::operator()(const item& item) const {
	return sdl::point_in_rect(x_, y_, item.rect);
}

std::string help_text_area::ref_at(const int x, const int y)
{
	const int local_x = x - location().x;
	const int local_y = y - location().y;
	if (local_y < height() && local_y > 0) {
		const int cmp_y = local_y + get_position();
		const std::list<item>::const_iterator it =
			std::find_if(items_.begin(), items_.end(), item_at(local_x, cmp_y));
		if (it != items_.end()) {
			if (!(*it).ref_to.empty()) {
				return ((*it).ref_to);
			}
		}
	}
	return "";
}

} // end namespace help
