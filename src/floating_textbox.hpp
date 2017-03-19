/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

// Scoped_resource can't use a pointer to an incomplete pointer with MSVC.
#include "widgets/textbox.hpp"

#include <memory>
#include <set>

class game_display;
class team;
class unit_map;

namespace gui{

	class button;

	enum TEXTBOX_MODE { TEXTBOX_NONE, TEXTBOX_SEARCH, TEXTBOX_MESSAGE,
		        TEXTBOX_COMMAND, TEXTBOX_AI };

	class floating_textbox{
	public:
		floating_textbox();

		TEXTBOX_MODE mode() const { return mode_; }
		const std::unique_ptr<gui::button>& check() const { return check_; }
		const std::unique_ptr<gui::textbox>& box() const { return box_; }

		void close(game_display& gui);
		void update_location(game_display& gui);
		void show(gui::TEXTBOX_MODE mode, const std::string& label,
			const std::string& check_label, bool checked, game_display& gui);
		void tab(const std::set<std::string>& dictionary);
		bool active() const { return box_.get() != nullptr; }

	private:
		std::unique_ptr<gui::textbox> box_;
		std::unique_ptr<gui::button> check_;

		TEXTBOX_MODE mode_;

		std::string label_string_;
		int label_;
	};
}

#endif
