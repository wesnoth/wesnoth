/*
   Copyright (C) 2011 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_UNINSTALL_LIST_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_UNINSTALL_LIST_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include <map>

namespace gui2
{

class taddon_uninstall_list : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param addon_titles_map
	 *                        Internal id <-> user-visible title mappings for
	 *                        the add-ons to display.
	 */
	explicit taddon_uninstall_list(
			const std::map<std::string, std::string>& addon_titles_map)
		: titles_map_(addon_titles_map), ids_(), selections_()
	{
	}

	std::vector<std::string> selected_addons() const;

private:
	std::map<std::string, std::string> titles_map_;
	std::vector<std::string> ids_;
	std::map<std::string, bool> selections_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif
