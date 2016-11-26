/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2
{
namespace dialogs
{

class editor_new_map : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] width     The parameter's usage is:
	 *                            - Input: The initial width of the map.
	 *                            - Output: The selected width of the map if
	 *                              the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 * @param [in, out] height    The parameter's usage is:
	 *                            - Input: The initial height of the map.
	 *                            - Output: The selected height of the map if
	 *                              the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 */
	editor_new_map(int& width, int& height);

	/** The execute function see @ref modal_dialog for more information. */
	static bool execute(int& width, int& height, CVideo& video)
	{
		return editor_new_map(width, height).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

} // namespace dialogs
} // namespace gui2

#endif
