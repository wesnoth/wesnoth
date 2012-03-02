/* $Id$ */
/*
   Copyright (C) 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_DEPS_PROMPT_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_DEPS_PROMPT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "addon/info.hpp"

namespace gui2 {

class taddon_deps_prompt : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param deps List of addon_info objects for the dependencies list.
	 */
	explicit taddon_deps_prompt(const std::vector<addon_info>& deps): deps_(deps) {}

	/** The execute function. See @ref tdialog for more information. */
	static bool execute(const std::vector<addon_info>& deps, CVideo& video)
	{
		return taddon_deps_prompt(deps).show(video);
	}

private:
	std::vector<addon_info> deps_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	void description_button_callback(twindow& window, const addon_info& addon);
};

} // end namespace gui2

#endif
