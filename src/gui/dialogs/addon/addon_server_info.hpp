/*
	Copyright (C) 2024 - 2024
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

#include "addon/client.hpp"

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2::dialogs
{

class addon_server_info : public modal_dialog
{
public:
	addon_server_info(addons_client& client, const std::string& addon, bool& needs_refresh);

	DEFINE_SIMPLE_EXECUTE_WRAPPER(addon_server_info)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	void downloads_by_version();
	void addon_count_by_forum_auth();
	void admin_delete_addon();
	void admin_hide_addon();
	void admin_unhide_addon();
	void admin_list_hidden();

	addons_client& client_;
	const std::string& addon_;
	bool& needs_refresh_;
};

} // namespace gui2::dialogs
