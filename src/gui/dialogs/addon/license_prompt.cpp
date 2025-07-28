/*
	Copyright (C) 2020 - 2025
	by Iris Morelle <shadowm@wesnoth.org>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/addon/license_prompt.hpp"

#include "gettext.hpp"

namespace gui2::dialogs
{
REGISTER_DIALOG(addon_license_prompt)

addon_license_prompt::addon_license_prompt(const std::string& license_terms)
	: modal_dialog(window_id())
{
	register_label("terms", true, license_terms, true);
}

} // end namespace gui2::dialogs
