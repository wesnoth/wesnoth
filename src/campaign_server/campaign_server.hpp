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

#include "campaign_server/blacklist.hpp"
#include "server/server_base.hpp"
#include "server/simple_wml.hpp"

#include "utils/functional.hpp"
#include <boost/unordered_map.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>

namespace campaignd {

/**
 * Legacy add-ons server.
 */
class server : public server_base
{
public:
	explicit server(const std::string& cfg_file);
	server(const config& server) = delete;
	~server();

	server& operator=(const config& server) = delete;

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

		const socket_ptr sock;
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
				socket_ptr reqsock)
			: cmd(reqcmd)
			, cfg(reqcfg)
			, sock(reqsock)
			, addr(client_address(sock))
		{}
	};

	typedef std::function<void (server*, const request& req)> request_handler;
	typedef std::map<std::string, request_handler> request_handlers_table;

	config cfg_;
	const std::string cfg_file_;

	bool read_only_;
	int compress_level_; /**< Used for add-on archives. */

	/** Default upload size limit in bytes. */
	static const size_t default_document_size_limit = 100 * 1024 * 1024;

	std::map<std::string, std::string> hooks_;
	request_handlers_table handlers_;

	std::string feedback_url_format_;

	blacklist blacklist_;
	std::string blacklist_file_;

	boost::asio::basic_waitable_timer<std::chrono::steady_clock> flush_timer_;

	void handle_new_client(socket_ptr socket);
	void handle_request(socket_ptr socket, std::shared_ptr<simple_wml::document> doc);

#ifndef _WIN32
	void handle_read_from_fifo(const boost::system::error_code& error, std::size_t bytes_transferred);

	void handle_sighup(const boost::system::error_code& error, int signal_number);
#endif

	/**
	 * Starts timer to write config to disk every ten minutes.
	 */
	void flush_cfg();
	void handle_flush(const boost::system::error_code& error);

	/**
	 * Reads the server configuration from WML.
	 */
	void load_config();

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

	void handle_request_campaign_list(const request&);
	void handle_request_campaign(const request&);
	void handle_request_terms(const request&);
	void handle_upload(const request&);
	void handle_delete(const request&);
	void handle_change_passphrase(const request&);

	/**
	 * Send a client an informational message.
	 *
	 * The WML sent consists of a document containing a single @p [message]
	 * child with a @a message attribute holding the value of @a msg.
	 */
	void send_message(const std::string& msg, socket_ptr sock);

	/**
	 * Send a client an error message.
	 *
	 * The WML sent consists of a document containing a single @p [error] child
	 * with a @a message attribute holding the value of @a msg. In addition to
	 * sending the error to the client, a line with the client IP and message
	 * is recorded to the server log.
	 */
	void send_error(const std::string& msg, socket_ptr sock);
};

} // end namespace campaignd
