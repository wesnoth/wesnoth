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

#pragma once

#include "gui/dialogs/modeless_dialog.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"

#include <set>

namespace gui2::dialogs {

	class floating_textbox : public modeless_dialog {
	public:
		enum MODE { NONE, SEARCH, MESSAGE, COMMAND, AI };
		floating_textbox(MODE mode, const std::string& label, const std::string& check_label = "", bool checked = false);

		MODE mode() const { return mode_; }
		bool checked() const;
		bool active() const { return active_; }
		std::string get_value() const;
		void set_value(const std::string& text);
		void set_properties(MODE mode, const std::string& label, const std::string& check_label, bool checked);
		const std::vector<std::string>& command_history() const { return command_history_; }

		// returns the text to chat log
		std::string tab(const std::set<std::string>& dictionary);
		void history_update(bool up);
		void memorize_command(const std::string& command);

		// called when enter pressed
		void on_execute(std::function<void(const std::string&)> f)
		{
			do_enter_ = std::move(f);
		}

		// called when tab pressed
		void on_completion(std::function<void(void)> f)
		{
			do_tab_ = std::move(f);
		}

	private:
		virtual const std::string& window_id() const;
		void pre_show();
		void key_down(const event::ui_event /*event*/,
						bool& handled,
						const SDL_Keycode key,
						SDL_Keymod modifier);

		gui2::toggle_button* check_;
		gui2::text_box* box_;
		MODE mode_;

		std::string label_string_, check_label_;
		bool initially_checked_, active_;
		std::vector<std::string> command_history_;

		std::function<void(const std::string&)> do_enter_;
		std::function<void(void)> do_tab_;
	};
}
