/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include <deque>                        // for deque
#include <string>                       // for string
#include <SDL_events.h>                 // for SDL_Event
#include "help_menu.hpp"				// for help_menu
#include "help_text_area.hpp"           // for help_text_area
#include "widgets/button.hpp"           // for button
#include "widgets/widget.hpp"           // for widget
class CVideo;  // lines 18-18
struct SDL_Rect;

namespace help {

/// A help browser widget.
class help_browser : public gui::widget
{
public:
	help_browser(CVideo& video, const section &toplevel);

	void adjust_layout();

	/// Display the topic with the specified identifier. Open the menu
	/// on the right location and display the topic in the text area.
	void show_topic(const std::string &topic_id);

protected:
	virtual void update_location(SDL_Rect const &rect);
	virtual void process_event();
	virtual void handle_event(const SDL_Event &event);

private:
	/// Update the current cursor, set it to the reference cursor if
	/// mousex, mousey is over a cross-reference, otherwise, set it to
	/// the normal cursor.
	void update_cursor();
	void show_topic(const topic &t, bool save_in_history=true);
	/// Move in the topic history. Pop an element from from and insert
	/// it in to. Pop at the fronts if the maximum number of elements is
	/// exceeded.
	void move_in_history(std::deque<const topic *> &from, std::deque<const topic *> &to);
	help_menu menu_;
	help_text_area text_area_;
	const section &toplevel_;
	bool ref_cursor_; // If the cursor currently is the hyperlink cursor.
	std::deque<const topic *> back_topics_, forward_topics_;
	gui::button back_button_, forward_button_;
	topic const *shown_topic_;
};

} // end namespace help
