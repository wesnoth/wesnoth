/* $Id$ */
/*
   Copyright (C) 2008 - 2013 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_NEW_MAP_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_NEW_MAP_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class teditor_new_map : public tdialog
{
public:

	/**
	 * Constructor.
	 *
	 * @param width [in]          The initial width of the map.
	 * @param width [out]         The selected width of the map if the dialog
	 *                            returns @ref twindow::OK undefined otherwise.
	 * @param height [in]         The initial height of the map.
	 * @param height [out]        The selected height of the map if the dialog
	 *                            returns @ref twindow::OK undefined otherwise.
	 */
	teditor_new_map(int& width, int& height);

	/** The excute function see @ref tdialog for more information. */
	static bool execute(int& width, int& height, CVideo& video)
	{
		return teditor_new_map(width, height).show(video);
	}

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

} // namespace gui2

#endif

