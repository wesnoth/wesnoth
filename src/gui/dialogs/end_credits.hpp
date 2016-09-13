/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_END_CREDITS_HPP_INCLUDED
#define GUI_DIALOGS_END_CREDITS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include <vector>

#include "sdl/utils.hpp"

class config;
class display;

namespace gui2
{

class tscroll_label;

class tend_credits : public tdialog
{
public:
	explicit tend_credits(const std::string& campaign);

	~tend_credits();

	static void display(CVideo& video, const std::string& campaign = "")
	{
		tend_credits(campaign).show(video);
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	void timer_callback();
	void key_press_callback(bool&, bool&, const SDL_Keycode key);

	const std::string& focus_on_;

	std::vector<std::string> backgrounds_;

	size_t timer_id_;

	tscroll_label* text_widget_;

	// The speed of auto-scrolling, specified as px/s
	int scroll_speed_;

	uint32_t last_scroll_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_END_CREDITS_HPP_INCLUDED */
