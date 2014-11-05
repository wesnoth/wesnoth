/*
   Copyright (C) 2011 - 2014 by Ján Dugáček
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
#include "addon/info.hpp"

#include <map>

namespace gui2
{

class taddon_reviews_list : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param addon
	 *                        Infomation about the add-on
	 */
	explicit taddon_reviews_list(addon_info& _addon, addon_info::this_users_rating& current_users_rating);

private:
	std::string& cut_into_more_lines(std::string& inserted, int max_length = 95);

	addon_info& addon;
	addon_info::this_users_rating& current_users_rating_;

	std::string initial_comment;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void write_review_button_callback(twindow& window);
};

} // namespace gui2

#endif
