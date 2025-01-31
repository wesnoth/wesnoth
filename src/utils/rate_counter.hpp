/*
	Copyright (C) 2003 - 2024
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

namespace utils
{
class rate_counter
{
public:
	explicit rate_counter(unsigned rate) : rate_(rate) {}

	/** Increments the counter by one and checks whether it is now a multiple of the chosen rate. */
	bool poll() { return (++counter_ % rate_) == 0; }

private:
	unsigned counter_ = 0;
	unsigned rate_ = 1;
};

} // end namespace utils
