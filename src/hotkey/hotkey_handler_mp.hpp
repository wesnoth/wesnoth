/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * An extension of playsingle_controller::hotkey_handler, which has support for
 * MP wesnoth features like network traffic
 */

#pragma once

#include "playmp_controller.hpp"
#include "hotkey/hotkey_handler_sp.hpp"

class playmp_controller::hotkey_handler : public playsingle_controller::hotkey_handler {

protected:
	playmp_controller & playmp_controller_;

public:
	hotkey_handler(playmp_controller &, saved_game &);
	~hotkey_handler();

	virtual void speak() override;
	virtual void whisper() override;
	virtual void shout() override;
	virtual void start_network() override;
	virtual void stop_network() override;
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const override;

};
