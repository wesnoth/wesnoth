/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_CORE_SELECTION_HPP_INCLUDED
#define GUI_DIALOGS_CORE_SELECTION_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

class config;

namespace gui2
{
namespace dialogs
{

class core_selection : public modal_dialog
{
public:
	explicit core_selection(const std::vector<config>& cores, int choice)
		: cores_(cores), choice_(choice)

	{
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	int get_choice() const
	{
		return choice_;
	}

private:
	/** Called when another core is selected. */
	void core_selected(window& window);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);

	/** Contains the config objects for all cores. */
	const std::vector<config>& cores_;

	/** The chosen core. */
	int choice_;
};

} // namespace dialogs
} // namespace gui2

#endif
