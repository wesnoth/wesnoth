/*
   Copyright (C) 2004 - 2016 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LABEL_HPP_INCLUDED
#define LABEL_HPP_INCLUDED

#include "font/constants.hpp"
#include "font/standard_colors.hpp"
#include "widget.hpp"
#include <string>

namespace gui {

class label : public widget
{
public:
	label(CVideo& video, const std::string& text, int size=font::SIZE_NORMAL,
			const SDL_Color& color=font::NORMAL_COLOR, const bool auto_join=true);
	const std::string& set_text(const std::string& text);
	const std::string& get_text() const;

	const SDL_Color& set_color(const SDL_Color& color);
	const SDL_Color& get_color() const;

	virtual void draw_contents();
private:
	void update_label_size();
	std::string text_;
	int size_;
	SDL_Color color_;
};

}

#endif

