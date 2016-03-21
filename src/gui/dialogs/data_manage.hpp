/*
   Copyright (C) 2010 - 2016 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MANAGE_DATA_HPP_INCLUDED
#define GUI_DIALOGS_MANAGE_DATA_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "save_index.hpp"
#include "tstring.hpp"

namespace gui2
{

class tlistbox;
class ttext_;

class tdata_manage : public tdialog
{
public:
	tdata_manage();

	const std::string& filename() const
	{
		return filename_;
	}

protected:
	/** Inherited from tdialog. */
	void pre_show(twindow& window);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	bool filter_text_changed(ttext_* textbox, const std::string& text);
	void list_item_clicked(twindow& window);
	void delete_button_callback(twindow& window);

	void fill_game_list(twindow& window,
						std::vector<savegame::save_info>& games);

	tfield_text* txtFilter_;

	std::string filename_;

	std::vector<savegame::save_info> games_;

	std::vector<std::string> last_words_;
};
}

#endif
