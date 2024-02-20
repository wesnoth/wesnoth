/*
	Copyright (C) 2017 - 2024
	by Charles Dang <exodia339@gmail.com>
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

class game_classification;

namespace gui2::dialogs
{
/** Dialog to display 'The End' at the end of a campaign. */
class outro : public modal_dialog
{
public:
	outro(const game_classification& info);

	/**
	 * Displays a simple fading screen with any user-provided text.
	 * Used after the end of single-player campaigns.
	 *
	 * @param text     Text to display, centered on the screen.
	 *
	 * @param duration In milliseconds, for how much time the text will
	 *                 be displayed on screen.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(outro)

	/** TLD override to update animations, called once per frame */
	virtual void update() override;

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	std::vector<std::string> text_;
	std::string current_text_;
	std::size_t text_index_;

	unsigned int duration_;
	int fade_step_;

	bool fading_in_;

	std::size_t timer_id_;
};

} // namespace dialogs
