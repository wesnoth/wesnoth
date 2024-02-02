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

#pragma once

#include "addon/info.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include <string>

namespace gui2::dialogs
{

class install_dependencies : public modal_dialog
{
public:
	explicit install_dependencies(const addons_list& addons)
		: modal_dialog(window_id()), addons_(addons)
	{}

	DEFINE_SIMPLE_EXECUTE_WRAPPER(install_dependencies)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	addons_list addons_;
};

}
