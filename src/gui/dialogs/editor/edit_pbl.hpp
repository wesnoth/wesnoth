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
class editor_edit_pbl : public modal_dialog
{
public:
	editor_edit_pbl(const std::string& pbl, const std::string& current_addon);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_pbl)

private:
	virtual void pre_show() override;
	virtual void post_show() override;

	virtual const std::string& window_id() const override;

	void toggle_auth();
	void add_translation();
	void delete_translation();
	void validate();
	void update_icon_preview();
	void update_url_preview();
	void select_icon_file();
	config create_cfg();

	std::string pbl_;
	std::string current_addon_;
	std::vector<std::string> dirs_;
};

} // namespace gui2::dialogs
