/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

#include <SDL_keycode.h>
#include <vector>

class config;
class display;

namespace gui2
{

class scroll_label;

namespace dialogs
{

class end_credits : public modal_dialog
{
public:
	explicit end_credits(const std::string& campaign);

	static void display(CVideo& video, const std::string& campaign = "")
	{
		end_credits(campaign).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	void timer_callback();
	void key_press_callback(const SDL_Keycode key);

	const std::string& focus_on_;

	std::vector<std::string> backgrounds_;

	scroll_label* text_widget_;

	// The speed of auto-scrolling, specified as px/s
	int scroll_speed_;

	uint32_t last_scroll_;
};

} // namespace dialogs
} // namespace gui2
