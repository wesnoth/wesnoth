/*
   Copyright (C) 2008 - 2016 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_SAVE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_SAVE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{

class tgame_save : public tdialog
{
public:
	tgame_save(std::string& filename, const std::string& title);

	static bool
	execute(std::string& filename, const std::string& title, CVideo& video)
	{
		return tgame_save(filename, title).show(video);
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

class tgame_save_message : public tdialog
{
public:
	tgame_save_message(std::string& filename,
					   const std::string& title,
					   const std::string& message);

	static bool execute(std::string& filename,
						const std::string& title,
						const std::string& message,
						CVideo& video)
	{
		return tgame_save_message(filename, title, message).show(video);
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

class tgame_save_oos : public tdialog
{
public:
	tgame_save_oos(bool& ignore_all,
				   std::string& filename,
				   const std::string& title,
				   const std::string& message);

	static bool execute(bool& ignore_all,
						std::string& filename,
						const std::string& title,
						const std::string& message,
						CVideo& video)
	{
		return tgame_save_oos(ignore_all, filename, title, message).show(video);
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};
}

#endif
