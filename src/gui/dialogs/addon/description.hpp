/*
   Copyright (C) 2010 - 2016 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_DESCRIPTION_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_DESCRIPTION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "addon/info.hpp"
#include "addon/state.hpp"

namespace gui2
{

class taddon_description : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param addon_id            The requested add-on's id.
	 * @param addons_list         Complete list of add-ons including the
	 *                            requested add-on and its dependencies.
	 * @param addon_states        Local installation status of the add-ons in
	 *                            @a addons_list.
	 */
	taddon_description(const std::string& addon_id,
					   const addons_list& addons_list,
					   const addons_tracking_list& addon_states);

	/**
	 * The display function.
	 *
	 * See @ref tdialog for more information.
	 */
	static void display(const std::string& addon_id,
						const addons_list& addons_list,
						const addons_tracking_list& addon_states,
						CVideo& video)
	{
		taddon_description(addon_id, addons_list, addon_states).show(video);
	}

private:
	std::string feedback_url_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	void browse_url_callback();
	void copy_url_callback();
};
}

#endif
