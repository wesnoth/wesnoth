/*
   Copyright (C) 2014 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_GAME_CACHE_OPTIONS_HPP_INCLUDED
#define GUI_DIALOGS_GAME_CACHE_OPTIONS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{
class tlabel;

class tgame_cache_options : public tdialog
{
public:
	/** Constructor. */
	tgame_cache_options();

	/**
     * The display function.
	 *
	 * See @ref tdialog for more information.
     */
	static void display(CVideo& video)
	{
		tgame_cache_options().show(video);
	}

private:
	std::string cache_path_;
	tlabel* size_label_;

	void clean_cache_callback(CVideo& video);
	bool clean_cache();

	void purge_cache_callback(CVideo& video);
	bool purge_cache();

	void copy_to_clipboard_callback();

	void browse_cache_callback();

	void update_cache_size_display();

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

}

#endif
