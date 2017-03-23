/*
   Copyright (C) 2017 by Charles Dang <exodia339@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_HELP_BROWSER_HPP_INCLUDED
#define GUI_DIALOGS_HELP_BROWSER_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

class config;
class CVideo;

namespace gui2
{
namespace dialogs
{

/** Help browser dialog. */
class help_browser : public modal_dialog
{
public:
	help_browser();

	static void display(CVideo& video)
	{
		help_browser().show(video);
	}

private:
	std::string initial_topic_;

	const config& help_cfg_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	void on_topic_select(window& window);
};

} // namespace dialogs
} // namespace gui2

#endif
