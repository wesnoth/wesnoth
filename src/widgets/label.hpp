/* $Id$ */
/*
   Copyright (C) 2004 - 2008 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LABEL_HPP_INCLUDED
#define LABEL_HPP_INCLUDED

#include "../font.hpp"
#include "widget.hpp"
#include <string>

namespace gui {

class label : public widget
{
public:
	label(CVideo& video, const std::string& text, int size=font::SIZE_NORMAL,
			const SDL_Color& colour=font::NORMAL_COLOUR, const bool auto_join=true);
	const std::string& set_text(const std::string& text);
	const std::string& get_text() const;

	int set_size(int size);
	int get_size() const;

	const SDL_Color& set_colour(const SDL_Color& colour);
	const SDL_Color& get_colour() const;

	virtual void draw_contents();
private:
	void update_label_size();

	std::string text_;
	int size_;
	SDL_Color colour_;
};

}

#endif

