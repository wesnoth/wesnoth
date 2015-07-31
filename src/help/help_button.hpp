/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef HELP_BUTTON_HPP
#define HELP_BUTTON_HPP

#include "hotkey/command_executor.hpp"
#include "construct_dialog.hpp"

class config;
class display;

namespace help {

class help_button : public gui::dialog_button, public hotkey::command_executor {
public:
	help_button(display& disp, const std::string &help_topic);
	~help_button();
	int action(gui::dialog_process_info &info);
	std::string topic() const { return topic_; }
	void join();
	void leave();
private:
	void show_help();
	bool can_execute_command(const hotkey::hotkey_command& command, int/*index*/ =-1) const;

	display &disp_;
	const std::string topic_;
	hotkey::basic_handler *help_hand_;
};

} // end namespace help

#endif
