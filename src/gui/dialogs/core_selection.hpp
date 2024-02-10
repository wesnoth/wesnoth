/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

class config;

namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the dialog which allows the user to choose which core to play.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * core_list         | @ref listbox |yes      |A listbox that contains all available cores.
 * icon              | @ref image   |no       |The icon for the core.
 * name              | control      |no       |The name of the core.
 * core_details      | multi_page   |yes      |A multi page widget that shows more details for the selected core.
 * image             | @ref image   |no       |The image for the core.
 * description       | control      |no       |The description of the core.
 */
class core_selection : public modal_dialog
{
public:
	explicit core_selection(const std::vector<config>& cores, int choice)
		: modal_dialog(window_id())
		, cores_(cores), choice_(choice)
	{
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	int get_choice() const
	{
		return choice_;
	}

private:
	/** Called when another core is selected. */
	void core_selected();

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	/** Contains the config objects for all cores. */
	const std::vector<config>& cores_;

	/** The chosen core. */
	int choice_;
};

} // namespace dialogs
