/*
   Copyright (C) 2004 - 2017 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "font/constants.hpp"
#include "font/standard_colors.hpp"
#include "widget.hpp"
#include <string>

namespace gui {

class label : public widget
{
public:
	label(CVideo& video, const std::string& text, int size=font::SIZE_NORMAL,
			const color_t& color=font::NORMAL_COLOR, const bool auto_join=true);
	const std::string& set_text(const std::string& text);
	const std::string& get_text() const;

	const color_t& set_color(const color_t& color);
	const color_t& get_color() const;

	virtual void draw_contents();
private:
	void update_label_size();
	std::string text_;
	int size_;
	color_t color_;
};

}
