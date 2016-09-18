/*
   Copyright (C) 2006 - 2016 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FLOATING_TEXTBOX_H_INCLUDED
#define FLOATING_TEXTBOX_H_INCLUDED

#include "gui/dialogs/popup.hpp"

#include <memory>
#include <set>

class game_display;
class team;
class unit_map;

namespace gui2 {
	class ttoggle_button;
	class ttext_box;
}

namespace gui2 {

	class tfloating_textbox : public tpopup {
	public:
		enum MODE { NONE, SEARCH, MESSAGE, COMMAND, AI };
		tfloating_textbox(game_display& gui, MODE mode, const std::string& label, const std::string& check_label = "", bool checked = false);
		//~tfloating_textbox();

		MODE mode() const { return mode_; }
		bool checked() const;
		bool active() const { return active_; }
		std::string get_value() const;

		//void update_location();
		void tab(const std::set<std::string>& dictionary);
		void show(CVideo& video) { tpopup::show(video, true); }

	private:
		virtual const std::string& window_id() const;
		void pre_show(twindow& window);
		ttoggle_button* check_;
		ttext_box* box_;
		//game_display& gui_;
		MODE mode_;

		std::string label_string_, check_label_;
		bool initially_checked_, active_;
	};
}

#endif
