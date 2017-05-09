/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "hotkey/command_executor.hpp"
#include "construct_dialog.hpp"

class config;
class CVideo;

namespace help {

class help_button : public gui::dialog_button, public hotkey::command_executor {
public:
	help_button(CVideo& video, const std::string &help_topic);
	~help_button();
	int action(gui::dialog_process_info &info) override;
	std::string topic() const { return topic_; }
	void join() override;
	void leave() override;
	CVideo& get_video() override { return video_; }
private:
	void show_help() override;
	bool can_execute_command(const hotkey::hotkey_command& command, int/*index*/ =-1) const override;

	CVideo &video_;
	const std::string topic_;
	hotkey::basic_handler *help_hand_;
};

} // end namespace help
