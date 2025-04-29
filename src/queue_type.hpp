/*
	Copyright (C) 2003 - 2025
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

#include "enum_base.hpp"

struct queue_type_defines
{
	static constexpr const char* const normal = "normal";
	static constexpr const char* const client_preset = "client_preset";
	static constexpr const char* const server_preset = "server_preset";

	ENUM_AND_ARRAY(normal, client_preset, server_preset)
};
using queue_type = string_enums::enum_base<queue_type_defines>;
