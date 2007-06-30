/* $Id$ */
/*
   Copyright (C) 2006 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth widget Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "construct_dialog.hpp"
#include "config.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "help.hpp"
#include "events.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "image.hpp"
#include "key.hpp"
#include "sound.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "thread.hpp"
#include "language.hpp"
#include "sdl_utils.hpp"
#include "tooltips.hpp"
#include "util.hpp"
#include "video.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"
#include "widgets/progressbar.hpp"
#include "widgets/textbox.hpp"
#include "wassert.hpp"

#include "sdl_ttf/SDL_ttf.h"

#include <iostream>
#include <numeric>

#define ERR_DP LOG_STREAM(err, display)
#define LOG_DP LOG_STREAM(info, display)
#define ERR_G  LOG_STREAM(err, general)

namespace {

struct help_handler : public hotkey::command_executor
{
	help_handler(display& disp, const std::string& topic) : disp_(disp), topic_(topic)
	{}

private:
	void show_help()
	{
		if(topic_.empty() == false) {
			help::show_help(disp_,topic_);
		}
	}

	bool can_execute_command(hotkey::HOTKEY_COMMAND cmd, int/*index*/ =-1) const
	{
		return (topic_.empty() == false && cmd == hotkey::HOTKEY_HELP) || cmd == hotkey::HOTKEY_SCREENSHOT;
	}

	display& disp_;
	std::string topic_;
};

} //end anonymous namespace

namespace gui {

dialog::dialog(display &disp, const std::string& title, const std::string& message,
				const DIALOG_TYPE type, const struct style* dialog_style,
				const std::string& help_topic) :
		basic_dialog(disp, title, message, type, dialog_style),
		help_button_(disp, help_topic)
{
	help_button_.set_parent(this);
}

void dialog::update_widget_positions()
{
	basic_dialog::update_widget_positions();
	help_button_.join();
}

int dialog::process(dialog_process_info &info)
{
	int res = basic_dialog::process(info);

	if(res == CONTINUE_DIALOG && help_button_.pressed()) {
		return help_button_.action(info);
	}

	return res;
}

int dialog::show()
{
	help_handler helper(disp_,help_button_.topic());
	hotkey::basic_handler help_dispatcher(&disp_,&helper);

	return basic_dialog::show();
}

int dialog::show(int xloc, int yloc)
{
	layout(xloc, yloc);
	return show();
}

dialog_frame& dialog::get_frame()
{
	return basic_dialog::get_frame(help_button_.topic().empty() ? NULL : &help_button_);
}

dialog::help_button::help_button(display& disp, const std::string &help_topic)
	: dialog_button(disp.video(), _("Help")), disp_(disp), topic_(help_topic)
{}

int dialog::help_button::action(dialog_process_info &info) {
	if(!topic_.empty()) {
		help::show_help(disp_,topic_);
		info.clear_buttons();
	}
	return CONTINUE_DIALOG;
}

}//end namespace gui
