/* $Id$ */
/*
   Copyright (C) 2011 - 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

namespace gui2 {

class taddon_uninstall_list : public tdialog
{
public:

	/**
	 * Constructor.
	 *
	 * @param addon_ids               The information about the addon to show.
	 */
	explicit taddon_uninstall_list(const std::vector<std::string>& addon_ids)
		: ids_(addon_ids), names_(), selections_() {}

	std::vector<std::string> selected_addons() const;

private:
	std::vector<std::string> ids_;
	std::vector<std::string> names_;
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
