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

#include <chrono>

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
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(outro)

	/** TLD override to update animations, called once per frame */
	virtual void update() override;

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	/** Returns a normalized [0.0 .. 1.0] value representing elapsed fade time. */
	double get_fade_progress(const std::chrono::steady_clock::time_point& now) const;

	/** The text to draw. Each entry is shown for the specified duration. */
	std::vector<std::string> text_;

	/** The index of the text currently being shown. */
	std::size_t text_index_;

	/** How long to display each text entry. */
	std::chrono::milliseconds display_duration_;

	/** Tracks whether we're fading in, displaying text, or fading out. */
	enum class stage { fading_in, waiting, fading_out } stage_;

	/** The time point at which the current stage began. */
	std::chrono::steady_clock::time_point stage_start_;
};

} // namespace dialogs
