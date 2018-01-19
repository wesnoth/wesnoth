/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <list>
#include <memory>
#include <tuple>

namespace game_events
{
class event_handler;

using handler_ptr = std::shared_ptr<event_handler>;
using weak_handler_ptr = std::weak_ptr<event_handler>;
using handler_list = std::list<weak_handler_ptr>;

using pump_result_t = std::tuple<bool /* undo_disabled*/, bool /* action_aborted */>;
}
