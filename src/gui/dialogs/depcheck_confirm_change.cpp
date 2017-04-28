/*
   Copyright (C) 2012 - 2017 by Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_depcheck_confirm_change
 *
 * == SP/MP Dependency Check: Confirm Change ==
 *
 * Asks the user to confirm a change required to proceed. Currently used
 * for enabling/disabling modifications
 *
 * @begin{table}{dialog_widgets}
 *
 * message & & label & m &
 * 		displays the details of the required changes $
 *
 * itemlist & & scroll_label & m &
 * 		displays the list of affected items $
 *
 * cancel & & button & m &
 * 		refuse to apply changes $
 *
 * ok & & button & m &
 * 		agree to apply changes $
 *
 * @end{table}
 */

REGISTER_DIALOG(depcheck_confirm_change)

depcheck_confirm_change::depcheck_confirm_change(
		bool action,
		const std::vector<std::string>& mods,
		const std::string& requester)
{
	utils::string_map symbols;
	symbols["requester"] = requester;
	std::string message;
	if(action) {
		message = vgettext("$requester requires the following modifications to "
						   "be enabled:",
						   symbols);
	} else {
		message = vgettext("$requester requires the following modifications to "
						   "be disabled:",
						   symbols);
	}

	std::string list = "\t";
	list += utils::join(mods, "\n\t");

	register_label("message", false, message);

	register_label("itemlist", false, list);
}
} // namespace dialogs
} // namespace gui2
