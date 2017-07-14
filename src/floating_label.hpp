/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
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

#include "color.hpp"
#include "sdl/texture.hpp"

#include <string>

namespace font
{
/**
 * Struct which will hide all current floating labels, and cause floating labels
 * instantiated after it is created to be displayed.
 */
struct floating_label_context
{
	floating_label_context();
	~floating_label_context();
};

enum ALIGN { LEFT_ALIGN, CENTER_ALIGN, RIGHT_ALIGN };

enum LABEL_SCROLL_MODE { ANCHOR_LABEL_SCREEN, ANCHOR_LABEL_MAP };

class floating_label
{
public:
	explicit floating_label(const std::string& text);

	void set_font_size(int font_size)
	{
		font_size_ = font_size;
	}

	/** Set the location on the screen to display the text. */
	void set_position(double xpos, double ypos)
	{
		xpos_ = xpos;
		ypos_ = ypos;
	}

	/** Set the amount to move the text each frame. */
	void set_move(double xmove, double ymove)
	{
		xmove_ = xmove;
		ymove_ = ymove;
	}

	/** Set the number of frames to display the text for, or -1 to display until removed. */
	void set_lifetime(int lifetime)
	{
		lifetime_ = lifetime;
		alpha_change_ = -255 / lifetime_;
	}

	void set_color(const color_t& color)
	{
		color_ = color;
	}

	void set_bg_color(const color_t& bg_color)
	{
		bgcolor_ = bg_color;
		fill_background_ = bgcolor_.a != 255;
	}

	void set_border_size(int border)
	{
		border_ = border;
	}

	/** Set width for word wrapping (use -1 to disable it). */
	void set_width(int w)
	{
		width_ = w;
	}

	void set_height(int h)
	{
		height_ = h;
	}

	void set_clip_rect(const SDL_Rect& r)
	{
		clip_rect_ = r;
	}

	void set_alignment(ALIGN align)
	{
		align_ = align;
	}

	void set_scroll_mode(LABEL_SCROLL_MODE scroll)
	{
		scroll_ = scroll;
	}

	void use_markup(bool b)
	{
		use_markup_ = b;
	}

	bool expired() const
	{
		return lifetime_ == 0;
	}

	void show(const bool value)
	{
		visible_ = value;
	}

	LABEL_SCROLL_MODE scroll() const
	{
		return scroll_;
	}

	void move(double xmove, double ymove);

	/** Draws this label. */
	void draw();

	/** Creates a texture of the label's text. */
	texture create_texture();

private:
	int xpos(size_t width) const;

	texture texture_;

	std::string text_;

	color_t color_, bgcolor_;

	double xpos_, ypos_, xmove_, ymove_;

	int width_, height_;
	int font_size_;
	int lifetime_;
	int border_;
	int alpha_change_;

	unsigned int current_alpha_;

	SDL_Rect clip_rect_;

	font::ALIGN align_;

	LABEL_SCROLL_MODE scroll_;

	bool visible_;
	bool use_markup_;
	bool fill_background_;
};

/**
 * Add a label floating on the screen above everything else.
 * @returns a handle to the label which can be used with other label functions
 */
int add_floating_label(const floating_label& flabel);

/** Moves the floating label given by 'handle' by (xmove, ymove). */
void move_floating_label(int handle, double xmove, double ymove);

/** Moves all floating labels that have 'scroll_mode' set to ANCHOR_LABEL_MAP. */
void scroll_floating_labels(double xmove, double ymove);

/** Removes the floating label given by 'handle' from the screen. */
void remove_floating_label(int handle);

/** Hides or shows a floating label. */
void show_floating_label(int handle, bool show);

/** Renders all floating labels. */
void draw_floating_labels();

SDL_Rect get_floating_label_rect(int handle);

} // end namespace font
