/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth"

#include "gui/dialogs/lobby_main.hpp"
#include "gui/dialogs/field.hpp"

#include "gui/widgets/listbox.hpp"
#include "foreach.hpp"

namespace gui2 {

tlobby_main::tlobby_main()
{
}

twindow* tlobby_main::build_window(CVideo& video)
{
	return build(video, get_id(LOBBY_MAIN));
}

void tlobby_main::update_gamelist(const config& cfg)
{
	foreach (const config &game, cfg.child("gamelist").child_range("game"))
	{
		std::map<std::string, string_map> data;
		string_map item;
		std::string tmp;

		tmp = game["name"];
		utils::truncate_as_wstring(tmp, 20);
		item["label"] = tmp;
		data.insert(std::make_pair("name", item));

		tmp = game["mp_era"];
		utils::truncate_as_wstring(tmp, 20);
		item["label"] = tmp;
		data.insert(std::make_pair("name", item));

		gamelist_->add_row(data);
	}
}

void tlobby_main::pre_show(CVideo& /*video*/, twindow& window)
{
	gamelist_ =
		dynamic_cast<tlistbox*>(window.find_widget("game_list", false));
	VALIDATE(gamelist_, missing_widget("game_list"));
}

} // namespace gui2
