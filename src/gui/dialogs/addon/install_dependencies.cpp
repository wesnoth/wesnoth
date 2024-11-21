/*
	Copyright (C) 2016 - 2024
	by Jyrki Vesterinen <sandgtx@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "install_dependencies.hpp"

#include "gettext.hpp"
#include "gui/widgets/addon_list.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/window.hpp"
#include "tstring.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(install_dependencies)

void install_dependencies::pre_show()
{
	find_widget<label>("label").set_label(t_string(
		_n(
			"The selected add-on has the following dependency, which is outdated or not currently installed. Do you wish to install it before continuing?",
			"The selected add-on has the following dependencies, which are outdated or not currently installed. Do you wish to install them before continuing?",
			addons_.size())
	));

	find_widget<addon_list>("dependencies").set_addons(addons_);
}

}
