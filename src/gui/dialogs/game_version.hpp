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

#include "build_info.hpp"

#include <map>

#include <boost/array.hpp>

namespace gui2
{

class tselectable_;
class tstacked_widget;

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

#ifdef _WIN32
	const std::string log_path_;
#endif

	typedef boost::array<std::string, 4> deplist_entry;
	std::vector<deplist_entry> deps_;

	std::vector<game_config::optional_feature> opts_;

	std::vector<tselectable_*> tabs_;

	std::string report_;

	void generate_plain_text_report();

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	//
	// Widget event callbacks.
	//

	/**
	 * Callback function called when the tab switch widgets' state changes.
	 *
	 * @param me            The widget whose state just changed.
	 * @param tab_container The tab pages container widget.
	 */
	void tab_switch_callback(tselectable_& me, tstacked_widget& tab_container);

	/**
	 * Callback function for the dialog-wide copy-to-clipboard button.
	 */
	void report_copy_callback();

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
};
}

#endif
