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

#ifndef GUI_DIALOGS_ADDON_INSTALL_DEPENDENCIES_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_INSTALL_DEPENDENCIES_HPP_INCLUDED

#include "addon/info.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include <string>

namespace gui2
{
namespace dialogs
{

class install_dependencies : public modal_dialog
{
public:
	explicit install_dependencies(const addons_list& addons)
		: addons_(addons)
	{}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	addons_list addons_;
};

}
}

#endif
