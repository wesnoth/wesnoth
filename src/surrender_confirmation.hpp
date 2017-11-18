/*
   Copyright (C) 2015 - 2017 by the Battle for Wesnoth Project
   <http://www.wesnoth.org/>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

class CVideo;
class display;

#include <cassert>
#include <vector>
#include "utils/functional.hpp"

/**
 * Implements a surrender confirmation dialog.
 *
 * Any object of this type will prevent the player from surrendering immediately.
 * Instead, a confirmation dialog will pop up when attempting to surrender.
 */
class surrender_confirmation
{
public:
	explicit surrender_confirmation(const std::function<bool()>& prompt = &surrender_confirmation::default_prompt)
		: prompt_(prompt) { blockers_.push_back(this); }

	~surrender_confirmation() { blockers_.pop_back(); }

	/**
	 * Shows the surrender confirmation if needed.
	 */
	static bool surrender();

	static bool show_prompt(const std::string& message);
	static bool default_prompt();

private:
	// noncopyable
	surrender_confirmation(const surrender_confirmation&) = delete;
	const surrender_confirmation& operator=(const surrender_confirmation&) = delete;
	static std::vector<surrender_confirmation*> blockers_;
	static bool open_;

	std::function<bool()> prompt_;
};
