/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/core/timer.hpp"
#include "gui/widgets/window.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "game_config.hpp"
#include <memory>

namespace gui2
{
namespace dialogs
{

class plugin_executor
{
	std::size_t timer_id;

	void play_slice() {
		if(plugins_context_) {
			plugins_context_->play_slice();
		}
	}
protected:
	std::unique_ptr<plugins_context> plugins_context_;

protected:
	plugin_executor()
		: timer_id(0u)
	{
		if(plugins_manager::get()) {
			timer_id = add_timer(game_config::lobby_network_timer, std::bind(&plugin_executor::play_slice, this), true);
		}
	}

	~plugin_executor()
	{
		if(plugins_manager::get()) {
			remove_timer(timer_id);
		}
	}
};

} // namespace dialogs
} // namespace gui2
