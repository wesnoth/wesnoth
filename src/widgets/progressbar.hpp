/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PROGRESS_BAR_HPP_INCLUDED
#define PROGRESS_BAR_HPP_INCLUDED

#include "widget.hpp"

namespace gui {

class progress_bar : public widget
{
public:
	progress_bar(CVideo& video);

	void set_progress_percent(int progress);
	void set_text(const std::string& text);

	void draw_contents();

private:
	int progress_;
	std::string text_;
};

}

#endif
