/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Copyright (C) 2015 - 2020 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "addon/validation.hpp"
#include "server/campaignd/blacklist.hpp"
#include "server/common/server_base.hpp"
#include "server/common/simple_wml.hpp"
#include "server/common/user_handler.hpp"

#include <boost/asio/basic_waitable_timer.hpp>

#include <chrono>
#include <functional>
#include <iosfwd>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace campaignd {

/**
 * Legacy add-ons server.
 */
class server : public server_base
{
public:
	explicit server(const std::string& cfg_file,
					unsigned short port = 0);
	server(const config& server) = delete;
	~server();

	server& operator=(const config& server) = delete;

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

		const any_socket_ptr sock;
		const std::string addr;

		/**
		 * context of the coroutine the request is executed in
		 * async operations on @a sock can use it instead of a handler.
		 */
		boost::asio::yield_context yield;

		/**
		 * Constructor.
		 *
		 * @param reqcmd  Request command.
		 * @param reqcfg  Request WML body.
		 * @param reqsock Client socket that initiated the request.
		 * @param yield The function will suspend on write operation using this yield context
		 *
		 * @note Neither @a reqcmd nor @a reqcfg are copied into instances, so
		 *       they are required to exist for as long as every @a request
		 *       instance that uses them. Furthermore, @a reqcfg MUST NOT REFER
		 *       TO A CONST OBJECT, since some code may modify it directly for
		 *       performance reasons.
		 */
		template<class Socket>
		request(const std::string& reqcmd,
				config& reqcfg,
				Socket reqsock,
				boost::asio::yield_context yield)
			: cmd(reqcmd)
			, cfg(reqcfg)
			, sock(reqsock)
			, addr(client_address(reqsock))
			, yield(yield)
		{}
	};

	friend std::ostream& operator<<(std::ostream& o, const request& r);

private:

	std::unique_ptr<user_handler> user_handler_;
	typedef std::function<void (server*, const request& req)> request_handler;
	typedef std::map<std::string, request_handler> request_handlers_table;

	std::set<std::string> capabilities_;

	/**The hash map of addons metadata*/
	std::unordered_map<std::string, config> addons_;
	/**The set of unique addon names with pending metadata updates*/
	std::unordered_set<std::string> dirty_addons_;

	/**Server config*/
	config cfg_;
	const std::string cfg_file_;

	bool read_only_;
	int compress_level_; /**< Used for add-on archives. */
	time_t update_pack_lifespan_;

	bool strict_versions_;

	/** Default upload size limit in bytes. */
	static const std::size_t default_document_size_limit = 100 * 1024 * 1024;

	std::map<std::string, std::string> hooks_;
	request_handlers_table handlers_;

	std::string server_id_;

	std::string feedback_url_format_;

	std::string web_url_;
	std::string license_notice_;

	blacklist blacklist_;
	std::string blacklist_file_;

	std::vector<std::string> stats_exempt_ips_;

	boost::asio::basic_waitable_timer<std::chrono::steady_clock> flush_timer_;

	void handle_new_client(socket_ptr socket);
	void handle_new_client(tls_socket_ptr socket);

	template<class Socket>
	void serve_requests(Socket socket, boost::asio::yield_context yield);

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

	/** Retrieves an addon by id if found, or a null config otherwise. */
	config& get_addon(const std::string& id);

	void delete_addon(const std::string& id);

	void mark_dirty(const std::string& addon)
	{
		dirty_addons_.emplace(addon);
	}

	/**
	 * Performs validation on an incoming add-on.
	 *
	 * @param req              Server request info, containing either a full
	 *                         WML pack, or a delta update pack.
	 * @param existing_addon   Used to store a pointer to the existing add-on
	 *                         WML on the server if the add-on already exists.
	 *                         nullptr is stored otherwise.
	 * @param error_data       Used to store extra error data for status codes
	 *                         other than ADDON_CHECK_STATUS::SUCCESS. Cleared
	 *                         otherwise.
	 *
	 * @return ADDON_CHECK_STATUS::SUCCESS if the validation checks pass, a
	 *         more relevant value otherwise.
	 */
	ADDON_CHECK_STATUS validate_addon(const server::request& req,
									  config*& existing_addon,
									  std::string& error_data);

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

	void handle_server_id(const request&);
	void handle_request_campaign_list(const request&);//#TODO: rename with 'addon' later?
	void handle_request_campaign(const request&);
	void handle_request_campaign_hash(const request&);
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
	void send_message(const std::string& msg, const any_socket_ptr& sock);

	/**
	 * Send a client an error message.
	 *
	 * The WML sent consists of a document containing a single @p [error] child
	 * with a @a message attribute holding the value of @a msg. In addition to
	 * sending the error to the client, a line with the client IP and message
	 * is recorded to the server log.
	 */
	void send_error(const std::string& msg, const any_socket_ptr& sock);

	/**
	 * Send a client an error message.
	 *
	 * The WML sent consists of a document containing a single @p [error] child
	 * with a @a message attribute holding the value of @a msg, an
	 * @a extra_data attribute holding the value of @a extra_data, and a
	 * @a status_code attribute holding the value of @a status_code. In
	 * addition to sending the error to the client, a line with the client IP
	 * and message is recorded to the server log.
	 */
	void send_error(const std::string& msg, const std::string& extra_data, unsigned int status_code, const any_socket_ptr& sock);
};

} // end namespace campaignd
