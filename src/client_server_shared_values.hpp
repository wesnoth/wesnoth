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

/**
 * client server shared values - things that need to be referenced by the client and wesnothd/campaignd
 */
namespace cssv
{
enum QUEUE_TYPE {
	NORMAL,
	CLIENT_PRESET,
	SERVER_PRESET,
};
}
