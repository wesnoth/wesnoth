/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CAMPAIGN_SERVER_HPP_INCLUDED
#define CAMPAIGN_SERVER_HPP_INCLUDED

#include "campaign_server/blacklist.hpp"
#include "network.hpp"
#include "server/input_stream.hpp"

namespace campaignd {

/**
 * Legacy add-ons server.
 */
class server : private boost::noncopyable
{
public:
	explicit server(const std::string& cfg_file,
					size_t min_threads = 10,
					size_t max_threads = 0);
	~server();

	/**
	 * Runs the server request processing loop.
	 */
	void run();

private:
	config cfg_;
	const std::string file_;

	bool read_only_;
	int compress_level_; /**< Used for add-on archives. */

	input_stream* input_; /**< Server control socket. */

	std::map<std::string, std::string> hooks_;

	std::string feedback_url_format_;

	blacklist blacklist_;
	std::string blacklist_file_;

	const network::manager net_manager_;
	const network::server_manager server_manager_;

	/**
	 * Reads the server configuration from WML.
	 *
	 * @return The configured listening port number.
	 */
	int load_config();

	/**
	 * Reads the add-ons upload blacklist from WML.
	 */
	void load_blacklist();

	/**
	 * Fires a hook script.
	 */
	void fire(const std::string& hook, const std::string& addon);

	/** Retrieves the contents of the [campaigns] WML node. */
	const config& campaigns() const { return cfg_.child("campaigns"); }

	/** Retrieves the contents of the [campaigns] WML node. */
	config& campaigns() { return cfg_.child("campaigns"); }

	/** Retrieves the contents of the [server_info] WML node. */
	const config& server_info() const { return cfg_.child("server_info"); }

	/** Retrieves the contents of the [server_info] WML node. */
	config& server_info() { return cfg_.child("server_info"); }
};

} // end namespace campaignd

#endif
