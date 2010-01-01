/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_SAVE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_SAVE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "tstring.hpp"

namespace gui2 {

class tgame_save : public tdialog
{
public:
	tgame_save(const std::string& title, const std::string& filename="");

	const std::string& filename() const { return filename_; }

protected:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

private:
	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	tfield_text* txtFilename_;
	std::string title_;
	std::string filename_;
};

class tgame_save_message : public tgame_save
{
public:
	tgame_save_message(const std::string& title, const std::string& filename="", const std::string& message="");

private:
	/** Inherited from tgame_save. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tgame_save. */
	twindow* build_window(CVideo& video);

	std::string message_;
};

class tgame_save_oos : public tgame_save_message
{
public:
	tgame_save_oos(const std::string& title, const std::string& filename="", const std::string& message="");

	bool ignore_all() const { return ignore_all_; }

private:
	/** Inherited from tgame_save. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	tfield_bool* btnIgnoreAll_;
	bool ignore_all_;
};

}

#endif

