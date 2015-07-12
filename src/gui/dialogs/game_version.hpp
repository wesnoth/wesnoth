/*
   Copyright (C) 2013 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_GAME_VERSION_HPP_INCLUDED
#define GUI_DIALOGS_GAME_VERSION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include <map>

namespace gui2
{

class tgame_version : public tdialog
{
public:
	/**
	 * Constructor.
	 */
	tgame_version();

	/**
	 * The display function.
	 *
	 * See @ref tdialog for more information.
	 */
	static void display(CVideo& video)
	{
		tgame_version().show(video);
	}

private:
	const std::string path_wid_stem_;
	const std::string copy_wid_stem_;
	const std::string browse_wid_stem_;

	std::map<std::string, std::string> path_map_;

	/**
	 * Callback function for copy-to-clipboard action buttons.
	 *
	 * @param path Filesystem path associated with the widget.
	 */
	void copy_to_clipboard_callback(const std::string& path);

	/**
	 * Callback function for browse-directory action buttons.
	 *
	 * @param path Filesystem path associated with the widget.
	 */
	void browse_directory_callback(const std::string& path);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
};
}

#endif
