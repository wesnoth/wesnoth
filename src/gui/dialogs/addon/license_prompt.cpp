/*
   Copyright (C) 2020 by Iris Morelle <shadowm@wesnoth.org>
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
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {
namespace dialogs {

REGISTER_DIALOG(addon_license_prompt)

addon_license_prompt::addon_license_prompt(const std::string& license_terms)
	: license_terms_(license_terms)
{
}

void addon_license_prompt::pre_show(window& window)
{
	styled_widget& terms = find_widget<styled_widget>(&window, "terms", false);
	terms.set_use_markup(true);
	terms.set_label(license_terms_);
}

}} // end namespace gui2::dialogs
