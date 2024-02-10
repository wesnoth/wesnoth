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

namespace gui2
{

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * Dialog for editing an add-on's _server.pbl.
 * Key               |Type              |Mandatory|Description
 * ------------------|------------------|---------|-----------
 * name              | text_box         |yes      |The name of the add-on displayed on the UI.
 * description       | text_box         |yes      |The add-on's description.
 * icon              | text_box         |yes      |The add-on's icon.
 * author            | text_box         |yes      |The author of the add-on. When using forum_auth, this must be a forum username.
 * version           | text_box         |yes      |The add-on's version.
 * dependencies      | text_box         |yes      |Other add-on IDs which this add-on depends on.
 * tags              | multimenu_button |yes      |Tags for the add-on for searching in the add-ons manager.
 * type              | menu_button      |yes      |The type of the add-on.
 * forum_thread      | text_box         |yes      |The topic ID of this add-on's forum feedback thread.
 * forum_auth        | toggle_button    |yes      |Whether to use forum authentication when uploading or deleting the add-on.
 * secondary_authors | text_box         |yes      |Any other forum usernames of people who are also allowed to upload updates for this add-on. Only works with forum_auth being enabled.
 * email             | text_box         |yes      |A contact email address. Requred when not using forum_auth.
 * password          | text_box         |yes      |The password to use when uploading the add-on. Requred when not using forum_auth.
 */
class editor_edit_pbl : public modal_dialog
{
public:
	editor_edit_pbl(const std::string& pbl, const std::string& current_addon);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_pbl)

private:
	virtual void pre_show(window& window) override;
	virtual void post_show(window& window) override;

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

} // namespace dialogs
} // namespace gui2
