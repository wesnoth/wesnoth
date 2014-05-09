/*
   Copyright (C) 2009 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/dialog.hpp"

#include "config.hpp"

namespace gui2
{

class tcore_selection : public tdialog
{
public:
	explicit tcore_selection(const std::vector<config>& cores, int choice)
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
	void core_selected(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** Contains the config objects for all cores. */
	const std::vector<config>& cores_;

	/** The chosen core. */
	int choice_;
};

} // namespace gui2

#endif
