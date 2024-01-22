/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
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
 * A Preferences subdialog permitting to configure the sounds and notifications generated in response to various mp lobby / game events.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * _label            | @ref label    |yes      |Item name.
 * _sound            | toggle_button |yes      |Toggles whether to play the item sound.
 * _notif            | toggle_button |yes      |Toggles whether to give a notification.
 * _lobby            | toggle_button |yes      |Toggles whether to take actions for this item when in the lobby.
 */
class mp_alerts_options : public modal_dialog
{
public:
	/** Constructor. */
	mp_alerts_options();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(mp_alerts_options)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
