/*
	Copyright (C) 2026 - 2026
	by Neil Hiddink
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

#include "utils/optional_fwd.hpp"

#include <string>

namespace desktop {
namespace apple {

utils::optional<std::string> get_icloud_drive_documents_dir();

} // end namespace apple
} // end namespace desktop
