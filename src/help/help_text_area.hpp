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

#pragma once

#include <list>                         // for list
#include <string>                       // for string
#include <utility>                      // for pair
#include "font/standard_colors.hpp"     // for NORMAL_COLOR
#include "sdl/surface.hpp"                // for surface
#include "widgets/scrollarea.hpp"       // for scrollarea
class CVideo;
class config;
namespace help { struct section; }
namespace help { struct topic; }

namespace help {


/// The area where the content is shown in the help browser.
class help_text_area : public gui::scrollarea
{
public:
	help_text_area(CVideo &video, const section &toplevel);
	/// Display the topic.
	void show_topic(const topic &t);

	/// Return the ID that is cross-referenced at the (screen)
	/// coordinates x, y. If no cross-reference is there, return the
	/// empty string.
	std::string ref_at(const int x, const int y);

protected:
	virtual void scroll(unsigned int pos);
	virtual void set_inner_location(const SDL_Rect& rect);

private:
	enum ALIGNMENT {LEFT, MIDDLE, RIGHT, HERE};
	/// Convert a string to an alignment. Throw parse_error if
	/// unsuccessful.
	ALIGNMENT str_to_align(const std::string &s);

	/// An item that is displayed in the text area. Contains the surface
	/// that should be blitted along with some other information.
	struct item {

		item(surface surface, int x, int y, const std::string& text="",
			 const std::string& reference_to="", bool floating=false,
			 bool box=false, ALIGNMENT alignment=HERE);

		item(surface surface, int x, int y,
			 bool floating, bool box=false, ALIGNMENT=HERE);

		/// Relative coordinates of this item.
		SDL_Rect rect;

		surface surf;

		// If this item contains text, this will contain that text.
		std::string text;

		// If this item contains a cross-reference, this is the id
		// of the referenced topic.
		std::string ref_to;

		// If this item is floating, that is, if things should be filled
		// around it.
		bool floating;
		bool box;
		ALIGNMENT align;
	};

	/// Function object to find an item at the specified coordinates.
	class item_at {
	public:
		item_at(const int x, const int y) : x_(x), y_(y) {}
		bool operator()(const item&) const;
	private:
		const int x_, y_;
	};

	/// Update the vector with the items of the shown topic, creating
	/// surfaces for everything and putting things where they belong.
	void set_items();

	// Create appropriate items from configs. Items will be added to the
	// internal vector. These methods check that the necessary
	// attributes are specified.
	void handle_ref_cfg(const config &cfg);
	void handle_img_cfg(const config &cfg);
	void handle_bold_cfg(const config &cfg);
	void handle_italic_cfg(const config &cfg);
	void handle_header_cfg(const config &cfg);
	void handle_jump_cfg(const config &cfg);
	void handle_format_cfg(const config &cfg);
	void handle_text_cfg(const config &cfg);

	void draw_contents();

	/// Add an item with text. If ref_dst is something else than the
	/// empty string, the text item will be underlined to show that it
	/// is a cross-reference. The item will also remember what the
	/// reference points to. If font_size is below zero, the default
	/// will be used.
	void add_text_item(const std::string& text, const std::string& ref_dst="",
					   bool broken_link = false,
					   int font_size=-1, bool bold=false, bool italic=false,
					   color_t color=font::NORMAL_COLOR);

	/// Add an image item with the specified attributes.
	void add_img_item(const std::string& path, const std::string& alignment, const bool floating,
					  const bool box);

	/// Move the current input point to the next line.
	void down_one_line();

	/// Adjust the heights of the items in the last row to make it look
	/// good .
	void adjust_last_row();

	/// Return the width that remain on the line the current input point is at.
	int get_remaining_width();

	/// Return the least x coordinate at which something of the
	/// specified height can be drawn at the specified y coordinate
	/// without interfering with floating images.
	int get_min_x(const int y, const int height=0);

	/// Analogous with get_min_x but return the maximum X.
	int get_max_x(const int y, const int height=0);

	/// Find the lowest y coordinate where a floating img of the
	/// specified width and at the specified x coordinate can be
	/// placed. Start looking at desired_y and continue downwards. Only
	/// check against other floating things, since text and inline
	/// images only can be above this place if called correctly.
	int get_y_for_floating_img(const int width, const int x, const int desired_y);

	/// Add an item to the internal list, update the locations and row
	/// height.
	void add_item(const item& itm);

	std::list<item> items_;
	std::list<item *> last_row_;
	const section &toplevel_;
	topic const *shown_topic_;
	const int title_spacing_;
	// The current input location when creating items.
	std::pair<int, int> curr_loc_;
	const unsigned min_row_height_;
	unsigned curr_row_height_;
	/// The height of all items in total.
	int contents_height_;
};

} // end namespace help
