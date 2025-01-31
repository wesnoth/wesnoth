/*
	Copyright (C) 2008 - 2024
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
/**
 * This shows the dialog to select a previous version of Wesnoth to migrate preferences from and redownload add-ons.
 */
class migrate_version_selection : public modal_dialog
{
public:
	migrate_version_selection();
	static void execute();

private:
	virtual void pre_show() override;
	virtual void post_show() override;
	virtual const std::string& window_id() const override;

	std::string old_config_dir();
	void migrate_credentials(const std::string& credentials_dir);

	std::vector<std::string> versions_;
};
} // namespace gui2::dialogs
