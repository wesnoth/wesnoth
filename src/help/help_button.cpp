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

#include "help_button.hpp"

#include "help.hpp"
#include "gettext.hpp"
#include "display.hpp"
#include "config.hpp"

#include "hotkey/command_executor.hpp"
#include "construct_dialog.hpp"

#include <string>

namespace help {

help_button::help_button(display& disp, const std::string &help_topic)
	: dialog_button(disp.video(), _("Help")), disp_(disp), topic_(help_topic), help_hand_(NULL)
{}

help_button::~help_button() {
	delete help_hand_;
}

int help_button::action(gui::dialog_process_info &info) {
	if(!topic_.empty()) {
		show_help();
		info.clear_buttons();
	}
	return gui::CONTINUE_DIALOG;
}

void help_button::show_help()
{
	help::show_help(disp_, topic_);
}

bool help_button::can_execute_command(const hotkey::hotkey_command& cmd, int/*index*/) const
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	return (topic_.empty() == false && command == hotkey::HOTKEY_HELP) || command == hotkey::HOTKEY_SCREENSHOT;
}

void help_button::join() {
	dialog_button::join();

	//wait until we join the event context to start a hotkey handler
	delete help_hand_;
	help_hand_ = new hotkey::basic_handler(&disp_, this);
}

void help_button::leave() {
	dialog_button::leave();

	//now kill the hotkey handler
	delete help_hand_;
	help_hand_ = NULL;
}

} // end namespace help
