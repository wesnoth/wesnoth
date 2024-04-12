/*
	Copyright (C) 2003 - 2024
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
 * This shows a dialog displaying community links.
 *
 * Key    |Type    |Mandatory|Description
 * -------|--------|---------|-----------
 * forums |button  |yes      |Links to the Wesnoth forums
 * discord|button  |yes      |Links to the Wesnoth Discord server
 * irc    |button  |yes      |Links to a web client for the Wesnoth IRC channels
 * steam  |button  |yes      |Links to the Wesnoth Steam forums
 * reddit |button  |yes      |Links to the Wesnoth sub-reddit
 * donate |button  |yes      |Links to the SPI donation page for Wesnoth
 */

class community_dialog : public modal_dialog
{
public:
	DEFINE_SIMPLE_EXECUTE_WRAPPER(community_dialog)

	community_dialog();

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace gui2::dialogs
