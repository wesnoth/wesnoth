/*
	Copyright (C) 2015 - 2024
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

#include <cassert>
#include <vector>
#include <string>

#include <functional>

/**
 * Implements a quit confirmation dialog.
 *
 * Any object of this type will prevent the game from quitting immediately.
 * Instead, a confirmation dialog will pop up when attempting to close.
 */
class quit_confirmation
{
public:
	explicit quit_confirmation(const std::function<bool()>& prompt = &quit_confirmation::default_prompt)
		: prompt_(prompt) { blockers_.push_back(this); }

	~quit_confirmation() { blockers_.pop_back(); }

	/**
	 * Shows the quit confirmation if needed.
	 *
	 * @throws video::quit If the user chooses to quit or no prompt was
	 *                      displayed.
	 */
	static bool quit();
	static void quit_to_title();
	static void quit_to_desktop();

	static bool show_prompt(const std::string& message);
	static bool default_prompt();

private:
	// noncopyable
	quit_confirmation(const quit_confirmation&) = delete;
	const quit_confirmation& operator=(const quit_confirmation&) = delete;

	static inline std::vector<quit_confirmation*> blockers_ {};
	static inline bool open_ = false;

	std::function<bool()> prompt_;
};
