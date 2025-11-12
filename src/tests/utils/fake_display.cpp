/*
	Copyright (C) 2008 - 2025
	by Pauli Nieminen <paniemin@cc.hut.fi>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "tests/utils/fake_display.hpp"

#include "events.hpp"
#include "video.hpp"

namespace test_utils
{
namespace
{
class context_manager
{
public:
	context_manager()
	{
		video::init(video::fake::no_draw);
	}

private:
	const events::event_context main_event_context_;
};

} // namespace

void set_test_resolution(const int width, const int height)
{
	// Single object for the lifetime of the unit tests.
	static const context_manager main_event_context;

	if(width >= 0 && height >= 0) {
		video::set_resolution({width, height});
	}
}

} // namespace test_utils
