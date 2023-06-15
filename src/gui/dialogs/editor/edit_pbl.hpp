/*
	Copyright (C) 2010 - 2023
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

namespace gui2
{

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * Dialog for editing an add-on's _server.pbl.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 */
class editor_edit_pbl : public modal_dialog
{
public:
	editor_edit_pbl(const std::string& pbl);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_pbl)

private:
	virtual void pre_show(window& window) override;
	virtual void post_show(window& window) override;

	virtual const std::string& window_id() const override;

	void toggle_auth();
	void add_translation();
	void delete_translation();

	std::string pbl_;
};

} // namespace dialogs
} // namespace gui2
