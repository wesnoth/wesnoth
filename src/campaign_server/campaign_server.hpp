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

#ifndef CAMPAIGN_SERVER_HPP_INCLUDED
#define CAMPAIGN_SERVER_HPP_INCLUDED

#include "campaign_server/blacklist.hpp"
#include "network.hpp"
#include "server/input_stream.hpp"

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>

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
	/**
	 * Client request information object.
	 *
	 * Contains data and metadata associated with a single request from a
	 * remote add-ons client, in a light-weight format for passing to request
	 * handlers.
	 */
	struct request
	{
		const std::string& cmd;
		const config& cfg;

		const network::connection sock;
		const std::string addr;

		/**
		 * Constructor.
		 *
		 * @param reqcmd  Request command.
		 * @param reqcfg  Request WML body.
		 * @param reqsock Client socket that initiated the request.
		 *
		 * @note Neither @a reqcmd nor @a reqcfg are copied into instances, so
		 *       they are required to exist for as long as every @a request
		 *       instance that uses them.
		 */
		request(const std::string& reqcmd,
				const config& reqcfg,
				network::connection reqsock)
			: cmd(reqcmd)
			, cfg(reqcfg)
			, sock(reqsock)
			, addr(network::ip_address(sock))
		{}
	};

	typedef boost::function<void (server*, const request& req)> request_handler;
	typedef std::map<std::string, request_handler> request_handlers_table;

	config cfg_;
	const std::string cfg_file_;

	bool read_only_;
	int compress_level_; /**< Used for add-on archives. */

	boost::scoped_ptr<input_stream> input_; /**< Server control socket. */

	std::map<std::string, std::string> hooks_;
	request_handlers_table handlers_;

	std::string feedback_url_format_;

	blacklist blacklist_;
	std::string blacklist_file_;

	std::vector<std::string> stats_exempt_ips_;

	int port_;

	const network::manager net_manager_;
	const network::server_manager server_manager_;

	/**
	 * Reads the server configuration from WML.
	 *
	 * @return The configured listening port number.
	 */
	int load_config();

	/**
	 * Writes the server configuration WML back to disk.
	 */
	void write_config();

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

	/** Retrieves a campaign by id if found, or a null config otherwise. */
	config& get_campaign(const std::string& id) { return campaigns().find_child("campaign", "name", id); }

	/** Retrieves the contents of the [server_info] WML node. */
	const config& server_info() const { return cfg_.child("server_info"); }

	/** Retrieves the contents of the [server_info] WML node. */
	config& server_info() { return cfg_.child("server_info"); }

	/** Checks if the specified address should never bump download counts. */
	bool ignore_address_stats(const std::string& addr) const;

	//
	// Request handling.
	//

	/**
	 * Registers client request handlers.
	 *
	 * This is called by the class constructor. Individual handlers must be
	 * methods of this class that take a single parameter of type @a request
	 * and they are registered using the @a register_handler method.
	 *
	 * When adding new handlers, make sure to update the implementation of
	 * this method accordingly so they are recognized and invoked at runtime.
	 */
	void register_handlers();

	/**
	 * Registers a single request handler.
	 *
	 * @param cmd  The request command, corresponding to the name of the [tag}
	 *             with the request body (e.g. "handle_request_terms").
	 * @param func The request function. This should be a class method passed
	 *             as a @a boost::bind function object that takes a single
	 *             parameter of type @a request.
	 */
	void register_handler(const std::string& cmd, const request_handler& func);

	void handle_request_campaign_list(const request&);
	void handle_request_campaign(const request&);
	void handle_request_terms(const request&);
	void handle_upload(const request&);
	void handle_delete(const request&);
	void handle_change_passphrase(const request&);

	//
	// Generic responses.
	//

	/**
	 * Send a client an informational message.
	 *
	 * The WML sent consists of a document containing a single @p [message]
	 * child with a @a message attribute holding the value of @a msg.
	 */
	void send_message(const std::string& msg, network::connection sock);

	/**
	 * Send a client an error message.
	 *
	 * The WML sent consists of a document containing a single @p [error] child
	 * with a @a message attribute holding the value of @a msg. In addition to
	 * sending the error to the client, a line with the client IP and message
	 * is recorded to the server log.
	 */
	void send_error(const std::string& msg, network::connection sock);
};

} // end namespace campaignd

#endif
