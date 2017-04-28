/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_PLAYER_NETWORK_HPP_INCLUDED
#define SERVER_PLAYER_NETWORK_HPP_INCLUDED

#include "player.hpp"
#include "simple_wml.hpp"

#include "utils/functional.hpp"
#include "log.hpp"

extern lg::log_domain log_config_pn;
namespace wesnothd {

namespace chat_message {
	/**
	 * Function to ensure a text message is within the allowed length
	 */
	void truncate_message(const simple_wml::string_span& str,
		simple_wml::node& message);
} // end chat_message namespace

} //end namespace wesnothd

#endif
