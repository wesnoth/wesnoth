/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_LOBBY_SOUNDS_OPTIONS_HPP_INCLUDED
#define GUI_DIALOGS_LOBBY_SOUNDS_OPTIONS_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2
{
class label;
namespace dialogs
{

class mp_alerts_options : public modal_dialog
{
public:
	/** Constructor. */
	mp_alerts_options();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	static void display(CVideo& video)
	{
		mp_alerts_options().show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);
};

} // namespace dialogs
} // namespace gui2

#endif
