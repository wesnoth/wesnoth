/*
	Copyright (C) 2012 - 2024
	by Boldizsár Lipka <lipkab@zoho.com>
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
#include "game_initialization/depcheck.hpp"
#include <vector>


namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * Offers a list of compatible items if a currently selected one is incompatible. Currently used for switching era or map.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * message           | @ref label   |yes      |displays the details of the required changes
 * itemlist          | @ref listbox |yes      |displays the available items to choose from
 * cancel            | @ref button  |yes      |refuse to apply any changes
 * ok                | @ref button  |yes      |select the chosen item
 */
class depcheck_select_new : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param name 		the type of which we want to select a new item
	 * @param options 	the names of the components which can be chosen
	 */
	depcheck_select_new(ng::depcheck::component_type name,
							const std::vector<std::string>& options);

	/**
	 * Returns the selected item.
	 *
	 * @return 		the index of the selected item, or -1 if none was selected
	 * 				(the dialog was closed with the cancel button)
	 */
	int result() const
	{
		return result_;
	}

protected:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

private:
	/** the options available */
	std::vector<std::string> items_;

	/** the index of the selected item */
	int result_;
};
} // namespace dialogs
