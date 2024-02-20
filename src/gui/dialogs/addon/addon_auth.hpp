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
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the dialog to provide a password when uploading an add-on.
 *
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * password          | text_box      |yes      |The password used to verify the uploader.
 */
class addon_auth : public modal_dialog
{
public:
	addon_auth(config& cfg_);

	DEFINE_SIMPLE_EXECUTE_WRAPPER(addon_auth)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	config& cfg_;
};

} // namespace gui2::dialogs
