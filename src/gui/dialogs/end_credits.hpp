/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <SDL2/SDL_keycode.h>
#include <vector>


namespace gui2
{

class scroll_label;

namespace dialogs
{

class end_credits : public modal_dialog
{
public:
	explicit end_credits(const std::string& campaign = "");

	DEFINE_SIMPLE_DISPLAY_WRAPPER(end_credits)

	/** TLD override to update animations, called once per frame */
	virtual void update() override;

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	void key_press_callback(const SDL_Keycode key);

	const std::string& focus_on_;

	std::vector<std::string> backgrounds_;

	scroll_label* text_widget_;

	// The speed of auto-scrolling, specified as px/s
	int scroll_speed_;

	uint32_t last_scroll_;

	/**
	 * sliding_size_ alters how many of the sliding contents are to be run at once
	 * n-1 = 2 => 3 strings at once concatinated
	 */
	static constexpr std::size_t sliding_size_ = 2;
	/**
	 * number of lines to put in each chunk of text to display
	 * the final chunk will of course probably have fewer lines
	 */
	static constexpr std::size_t lines_per_chunk_ = 50;
	std::size_t first_idx_;
	std::size_t last_idx_;
	std::string content_;
	std::string sliding_content_;
	std::vector<std::vector<std::string>> chunks_;
};

} // namespace dialogs
} // namespace gui2
