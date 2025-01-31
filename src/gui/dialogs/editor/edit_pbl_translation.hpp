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
class editor_edit_pbl_translation : public modal_dialog
{
public:
	editor_edit_pbl_translation(std::string& language, std::string& title, std::string& description);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_pbl_translation)

private:
	virtual void pre_show() override;
	virtual void post_show() override;

	virtual const std::string& window_id() const override;

	std::string& language_;
	std::string& title_;
	std::string& description_;
};

} // namespace gui2::dialogs
