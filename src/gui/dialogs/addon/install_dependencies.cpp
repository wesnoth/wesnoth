/*
   Copyright (C) 2016 - 2017 by Jyrki Vesterinen <sandgtx@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "install_dependencies.hpp"

#define GETTEXT_DOMAIN "wesnoth"

#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/addon_list.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "tstring.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(install_dependencies)

void install_dependencies::pre_show(window& window)
{
	find_widget<label>(&window, "label", false).set_label(t_string(
		_n(
			"The selected add-on has the following dependency, which is not currently installed. Do you wish to install it before continuing?",
			"The selected add-on has the following dependencies, which are not currently installed. Do you wish to install them before continuing?",
			addons_.size())
	));

	find_widget<addon_list>(&window, "dependencies", false).set_addons(addons_);
}

}
}
