/*
   Copyright (C) 2013 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_SCREENSHOT_NOTIFICATION_HPP_INCLUDED
#define GUI_DIALOGS_SCREENSHOT_NOTIFICATION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{

class tscreenshot_notification : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param path     Path to the screenshot file created,
	 * @param filesize Size of the screenshot file.
	 */
	tscreenshot_notification(const std::string& path, int filesize);

	/**
	 * The display function.
	 *
	 * See @ref tdialog for more information.
	 */
	static void display(const std::string& path, int filesize, CVideo& video)
	{
		tscreenshot_notification(path, filesize).show(video);
	}

private:
	const std::string path_;
	const std::string screenshots_dir_path_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
};
}

#endif /* ! GUI_DIALOGS_SCREENSHOT_NOTIFICATION_HPP_INCLUDED */
