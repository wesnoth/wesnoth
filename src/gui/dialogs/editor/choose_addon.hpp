/*
	Copyright (C) 2023 - 2024
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

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2::dialogs
{

class editor_choose_addon : public modal_dialog
{
public:
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_choose_addon)

	editor_choose_addon(std::string& addon_id);

private:
	std::string& addon_id_;

	void toggle_installed();
	void populate_list(bool show_all);

	virtual const std::string& window_id() const override;

	virtual void post_show() override;
};

} // namespace gui2::dialogs
