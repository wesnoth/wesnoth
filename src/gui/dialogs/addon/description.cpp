/* $Id$ */
/*
   Copyright (C) 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/addon/description.hpp"

#include "foreach.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"

namespace {
	t_string langcode_to_tstring(const std::string& lcode)
	{
		foreach(const language_def& ld, get_languages()) {
			if(ld.localename == lcode) {
				return ld.language;
			}
		}

		return "";
	}
}

namespace gui2 {

REGISTER_WINDOW(addon_description)

void taddon_description::pre_show(CVideo& /*video*/, twindow& window)
{
	const std::string fixed_icon = ainfo_.icon + "~SCALE(72,72)";
	find_widget<tcontrol>(&window, "image", false).set_label(fixed_icon);

	find_widget<tcontrol>(&window, "title", false).set_label(ainfo_.name);
	find_widget<tcontrol>(&window, "description", false).set_label(ainfo_.description);
	find_widget<tcontrol>(&window, "version", false).set_label(ainfo_.version);

	std::string languages;

	foreach(const std::string& lc, ainfo_.translations) {
		if(languages.empty() == false) {
			languages += ", ";
		}
		languages += langcode_to_tstring(lc);
	}

	find_widget<tcontrol>(&window, "translations", false).set_label(languages);
}

}

