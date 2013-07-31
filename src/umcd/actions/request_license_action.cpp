/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/* Design rational:
The license is not shipped with the Wesnoth client because this server can be re-use with different licenses on other server than Wesnoth ones.
*/

#include "umcd/actions/request_license_action.hpp"
#include "umcd/protocol/wml/umcd_protocol.hpp"
#include "filesystem.hpp"

request_license_action::request_license_action(const config& server_config)
: server_config_(server_config)
{}

void request_license_action::execute(boost::shared_ptr<umcd_protocol> protocol)
{
	// NOTE: We don't use the COPYING file because the " are not double quoted, instead we use a preformatted license file with " replaced by "".
	config reply("request_license");
	reply.child("request_license")["text"] = "\"" + read_file(server_config_["wesnoth_dir"].str() + "data/umcd/license.txt") + "\"";
	protocol->get_reply() = wml_reply(reply, umcd_protocol::REQUEST_HEADER_SIZE_FIELD_LENGTH);
	protocol->async_send_reply();
}

boost::shared_ptr<request_license_action::base> request_license_action::clone() const
{
	return boost::shared_ptr<base>(new request_license_action(*this));
}

