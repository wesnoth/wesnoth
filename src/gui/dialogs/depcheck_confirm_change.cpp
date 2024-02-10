/*
	Copyright (C) 2012 - 2024
	by Boldizsár Lipka <lipkab@zoho.com>
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

#include "gui/dialogs/depcheck_confirm_change.hpp"

#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(depcheck_confirm_change)

depcheck_confirm_change::depcheck_confirm_change(
		bool action,
		const std::vector<std::string>& mods,
		const std::string& requester)
	: modal_dialog(window_id())
{
	utils::string_map symbols;
	symbols["requester"] = requester;
	std::string message;
	if(action) {
		message = VGETTEXT("$requester requires the following modifications to "
						   "be enabled:",
						   symbols);
	} else {
		message = VGETTEXT("$requester requires the following modifications to "
						   "be disabled:",
						   symbols);
	}

	std::string list = "\t";
	list += utils::join(mods, "\n\t");

	register_label("message", false, message);

	register_label("itemlist", false, list);
}
} // namespace dialogs
